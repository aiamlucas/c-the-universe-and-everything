# Minitalk

> “Treat a UNIX signal as a **single-bit courier**. Your job is to turn two couriers (SIGUSR1/SIGUSR2) into a reliable byte stream.”

---

## Table of Contents

1. Signal Communication
2. Process IDs & `kill()`
3. Signal Delivery Semantics (pending set, coalescing, masks)
4. Signal Handlers (disposition, SA_SIGINFO, async-signal-safety)
5. Binary Encoding (bits → bytes, bit order vs endianness)
6. Bit-by-Bit Transmission (LSB/MSB choices, assembly)
7. Acknowledgment Protocol (pacing, flow control)
8. Race Conditions & Reliability (coalescing, reentrancy, “busy server”)
9. The Solution Strategy: Stop-and-Wait
10. Multiple Clients (in a row, not concurrently)
11. Unicode / UTF-8 (why “just bytes” works)
12. Robustness & Error Handling (timeouts, wrong PIDs, mid-flight failures)
13. Performance & Timing (latency vs throughput, sizing sleeps)
14. Memory Model & Globals (why exactly one global, `volatile sig_atomic_t`)
15. Handler Design Patterns (state, masking, printing policy)
16. Debugging & Troubleshooting (symptoms → causes)

---

## 1) Signal Communication

- **What is a signal?** A small, asynchronous **event** delivered by the kernel to a process.
- **What is a signal?** A small, asynchronous **notification** the kernel delivers to a process (or thread) to say “**something happened**.” It’s like a **software interrupt**: your program can be doing anything and a signal can arrive and briefly **divert** it to run a small **handler** function.

### Where do signals come from?

- **The kernel** (faults/events): e.g. `SIGSEGV` on invalid memory, `SIGALRM` on timer expiry.
- **The terminal / shell**: e.g. `Ctrl+C` → `SIGINT`, `Ctrl+Z` → `SIGTSTP`.
- **Other processes**: via `kill(pid, sig)` (what *minitalk* uses).
- **The same process**: it can signal itself.

```
// concept: send a notification to another process
kill(server_pid, SIGUSR1);   // no payload, just "an event"
```

### More about signals 
- **Asynchronous**: they can arrive at **any time**, not aligned with your code flow.
- **Tiny**: a signal carries **only its type** (e.g., USR1 vs USR2). Standard signals have **no data payload**.
- **Interrupting**: on delivery, the kernel can call your **handler** briefly, then return to what you were doing.

- **In minitalk** you’re constrained to **two** signals:
  - `SIGUSR1` (user-defined #1)
  - `SIGUSR2` (user-defined #2)
- **You must encode information using only these.** The canonical mapping is:
  - `SIGUSR1` ⇒ bit **0**
  - `SIGUSR2` ⇒ bit **1**
- **Key constraint on Linux:** standard (non–real-time) signals are **not queued per-instance**. If multiple of the same type arrive while one is pending, they appear as **one**. This is the root cause of “lost bits unless paced.”

---

## 2) Process IDs & `kill()`

- **PID**: Kernel identifier of a running process.
- **`kill(pid, sig)`**: Sends a signal to `pid`. It does *not* necessarily “kill” the process; it **delivers** `sig`.
- **Existence/probe**:
  - `kill(pid, 0)` returns `0` if you can signal the process (exists & permitted),
  - `-1` with `ESRCH` if it doesn’t exist. (No such process)
  - `-1` with `EPERM` if it exists but you lack permission. (Operation not permitted)
- **Why it matters**: Before attempting to transmit, validate the PID and permissions to avoid undefined behavior.

---

## 3) Signal Delivery Semantics (pending set, coalescing, masks)

### Pending set (coalescing)
- Linux keeps **one “pending” flag per signal type**. Ten fast `SIGUSR2` become **one** pending USR2; the rest are merged.
- **In Minitalk:** if the client fires bits in a tight loop, the server will **miss** some ‘1’s/‘0’s → corrupted characters.
  - That’s why the client must **send 1 bit → wait for ACK → send next**.
	- **ACK =** a tiny confirmation signal (e.g., `SIGUSR2`) sent by the server after it **receives and processes** a bit; it tells the client “OK, send the next one.”

Tiny timeline:

```
client:  send SIGUSR2 (bit=1)  ─────────►  server
server:  process bit; ACK      ◄─────────  SIGUSR2
client:  (only now) send next bit
```

---

### Masks (blocking to avoid reentrancy)
- A **signal mask** blocks delivery while your handler runs; blocked signals wait as “pending” and are delivered after.
- **In Minitalk (server):** block **both** `SIGUSR1` and `SIGUSR2` inside the handler to protect your shared state
  (`client_pid`, `current_char`, `bit_count`) from being updated **mid-operation**.

Setup idea:

```
sa.sa_sigaction = handler;
sigemptyset(&sa.sa_mask);
sigaddset(&sa.sa_mask, SIGUSR1);   // block while inside handler
sigaddset(&sa.sa_mask, SIGUSR2);
sa.sa_flags = SA_SIGINFO;
sigaction(SIGUSR1, &sa, NULL);
sigaction(SIGUSR2, &sa, NULL);
```

Effect: while assembling a bit, no second bit can interrupt; state updates stay consistent.

---

### Default actions
- Every signal has a **default** (often terminate). With `sigaction` you install **your** handler to override it.
- **In Minitalk:** you **must** set handlers for **both** `SIGUSR1` and `SIGUSR2`. Otherwise the first incoming signal could **kill** your server instead of delivering a bit.

---

### Takeaway (project-level)
- Standard signals **don’t queue** per instance → you **cannot blast bits**.
- **Protocol required:** **Stop-and-Wait**  

  ```
  client: send 1 bit → wait for ACK
  server: process 1 bit → send ACK
  repeat 8 times → 1 byte; send '\0' to end
  ```
- This pacing guarantees at most **one** bit “in flight,” making your signal-based link **reliable**.
---

## 4) Signal Handlers (disposition, SA_SIGINFO, async-signal-safety)

- **Installing a handler**:
  - Use `sigaction` (not legacy `signal()`), because:
    - `SA_SIGINFO` provides **sender info** (`siginfo_t`), notably `si_pid` = sender PID.
    - You can specify a **mask** of signals to block during handler execution.
- **Async-signal-safety**:
  - Only call **async-signal-safe** functions in a handler (e.g., `write`, `kill`, `_exit`).  
  - Functions like `printf`, `malloc`, `free`, `strtok`, etc. are **not** safe. Using them can deadlock or corrupt state.
- **Minimal work**:
  - Handlers should **only**:
    1) update tiny global state,
    2) send an ACK signal,
    3) optionally `write` a completed byte to stdout.

```
// Conceptual handler checklist:
on_signal(sig, info):
  block(SIGUSR1, SIGUSR2) while running
  if first bit of this message: record info->si_pid as current client
  update current_char and bit_count according to sig (0 or 1)
  send ACK to sender PID
  if bit_count == 8:
      if current_char == '\0': print newline; clear client_pid; reset byte state
      else: write byte; reset byte state
  return
```

---

## 5) Binary Encoding (bits → bytes, bit order vs endianness)

- **Eight bits make a byte**. You will send **8 signals per char**.
- **Bit order**:
  - You must choose to send either **MSB→LSB** (bit 7 down to bit 0) or **LSB→MSB**; pick one and keep it **consistent** between client and server.
  - Many folks choose **MSB→LSB** because the server can build a byte with **left-shifts**: append a 0 by shifting left; append a 1 by shifting then OR 1.
- **Endianness** (CPU byte order) vs **bit order**:
  - CPU endianness affects **byte order in multi-byte integers**; it has **no impact** on how you push bits over signals. Bit order here is **your protocol choice**.
- **Message terminator**:
  - Use the C-string terminator `'\0'` (0x00) to mark **end-of-message**. This byte will never appear inside a valid UTF-8 code point, so it’s a safe delimiter.

---

## 6) Bit-by-Bit Transmission (assembly logic)

- **Mapping**:
  - `SIGUSR1` = **0**; `SIGUSR2` = **1**.
- **Server assembly**:
  - Maintain `(current_char, bit_count)`.
  - On each signal:
    - For MSB-first: `current_char <<= 1; if (sig == SIGUSR2) current_char |= 1;`
    - Increment `bit_count`.
  - When `bit_count == 8`:
    - If `current_char == 0`: **end-of-message**; print newline; reset sender lock.
    - Else: `write` the byte; reset `(current_char, bit_count)`.

---

## 7) Acknowledgment Protocol (pacing, flow control)

- **Why ACK?** Because standard signals are **coalesced**. Without pacing, you **will** lose bits.
- **Per-bit ACK** (Stop-and-Wait):
  - After processing **one bit**, the server **sends an ACK signal** (e.g., `SIGUSR2`) to the client’s PID.
  - The client **waits** until it sees the ACK, then sends the **next bit**.
- **Timeouts**:
  - The client should not wait forever. Add a timeout while waiting for an ACK (the server might have died).
- **Message ACK (optional)**:
  - Some designs send a **final ACK** (distinct signal) after `'\0'`. This can create a **race** with per-bit ACK for the last bit. If you keep it, design the client’s wait logic to accept the final ACK as satisfying the last per-bit wait.  
  - Simplest approach: **per-bit ACK only** (no final-ACK), avoiding that race.

---

## 8) Race Conditions & Reliability

**Common hazards and mitigations:**

1) **Signal coalescing** (lost bits)  
   - *Cause:* sending multiple identical signals faster than the receiver processes them.  
   - *Fix:* **Stop-and-Wait** per-bit ACK. One bit in flight → no loss.

2) **Reentrancy** (handler runs during handler)  
   - *Cause:* you receive the other user signal while your handler is using shared state.  
   - *Fix:* in `sigaction`, set `sa_mask` to block **both** SIGUSR1 and SIGUSR2 during the handler. Update state atomically (simple ints/bytes).

3) **Multiple clients at once**  
   - *Cause:* two different PIDs interleave bits.  
   - *Fix:* record the **first** sender PID (`si_pid`) and **ignore** bits from others until you receive `'\0'`. This honors “several clients in a row” without interleaving.

4) **End-of-message ACK race** (if you implement it)  
   - *Cause:* server sends per-bit ACK and immediately sends a final message ACK; client may be waiting for per-bit ACK but sees final first.  
   - *Fix:* either accept final ACK as satisfying the last per-bit wait, or **omit** message-level ACK. The latter is simpler.

5) **Printing from the handler**  
   - *Cause:* using `printf` or non-safe functions.  
   - *Fix:* use **`write`** only. It’s async-signal-safe.

---

## 9) The Solution Strategy: Stop-and-Wait

**Protocol (deterministic & reliable):**

1) **Client sends one bit** using `SIGUSR1` (0) or `SIGUSR2` (1).
2) **Server receives**, updates `(current_char, bit_count)`, then **ACKs** with a signal (e.g., `SIGUSR2`).
3) **Client waits** until it sees the ACK; only then sends the **next bit**.
4) Repeat for 8 bits → 1 byte.  
   If byte is `'\0'`: server prints newline, **releases** current client PID, resets state; ready for next client.
5) **Why it works:** at most **one** in-flight bit; delivery is lossless.

```
// Timeline example for one character (MSB-first):
Client:  (bit7=1) → SIGUSR2 ────────────────► Server
Server:                                  ACK ◄────────── SIGUSR2
Client: (bit6=0) → SIGUSR1 ────────────────► Server
Server:                                  ACK ◄────────── SIGUSR2
...
(repeat 8 times)
```

---

## 10) Multiple Clients (in a row, not concurrently)

- **Requirement**: “The server should be able to receive strings from several clients **in a row** without restart.”
- **Policy**:
  - Track `current_sender_pid`.
  - If `si_pid` differs from `current_sender_pid` **and** a message is in progress, **ignore** the bit. (Do not send an ACK; the intrusive client will time out and retry later.)
  - When you receive `'\0'`, reset `current_sender_pid = 0`. The next client that contacts you becomes the active sender.

This preserves protocol correctness and keeps the implementation simple and defendable.

---

## 11) Unicode / UTF-8 (why “just bytes” works)

- **UTF-8** is a variable-length encoding: ASCII characters are 1 byte, many symbols are 2–4 bytes.
- You do **not** need to parse or understand code points. You are transporting **bytes**.  
- The terminal will interpret those bytes as UTF-8 and render the character.
- The **`'\0'`** terminator is a single byte `0x00`, and UTF-8 never embeds `0x00` inside a character, so it’s a safe message boundary.

---

## 12) Robustness & Error Handling

**Client side:**
- **PID validation**:
  - Parse strictly (positive integer).
  - Probe with `kill(pid, 0)`; on failure print a clear error and exit.
- **Signal send failure**:
  - If any `kill(pid, sig)` returns `-1`, abort cleanly—likely server died between bits.
- **ACK timeout**:
  - If ACK doesn’t arrive within a bounded number of small sleeps, print a timeout error and exit. Prevents infinite hangs.

**Server side:**
- **Handler safety**:
  - Only async-signal-safe calls (`write`, `kill`) inside the handler; keep it short.
- **State reset**:
  - After `'\0'`: print newline, reset sender PID and byte assembly state.
- **Ignore interlopers**:
  - If a second client speaks mid-message, ignore its signals (no ACK) to maintain message integrity.

---

## 13) Performance & Timing

- **Cost per character**:
  - 8 client→server signals + 8 server→client ACKs = **16 signals** per char.
- **Throughput** depends on:
  - Handler latency (very small if it just flips a few bits + write),
  - Client ACK polling sleep (e.g., 1 ms),
  - Context switch overhead.
- **Rule of thumb**:
  - Keep the per-ACK polling sleep short (hundreds of microseconds to a couple milliseconds).
  - Avoid extra sleeps between bytes—ACK already paces you.
- **Judge’s hint**:
  - “If 100 characters take ~1 s, you’re too slow.”  
  - With efficient handlers and ~1 ms polling, you’ll comfortably beat this.

---

## 14) Memory Model & Globals

- **Why a global at all?** POSIX signal handlers don’t accept user data; state must persist across interrupts and be accessible from the handler. Heap allocations in a handler are unsafe.  
- **Server global**:
  - A **single** struct:
    - `client_pid` (who we’re serving),
    - `current_char` (byte under construction),
    - `bit_count` (how many bits accumulated).
  - This is **one** variable (not “three”), complying with “one global per program”.
- **Client global**:
  - `volatile sig_atomic_t ack_flag` (or similar).
  - `sig_atomic_t` ensures atomic read/write relative to signals; `volatile` prevents compiler optimizations that would cache the value.

**Note:** `volatile sig_atomic_t` is not a full memory barrier, but it’s sufficient here: a single boolean handoff between handler and main loop.

---

## 15) Handler Design Patterns (state, masking, printing policy)

**Masking to avoid reentrancy**:
- In `sigaction`, set `sa_mask` to include **both** `SIGUSR1` and `SIGUSR2`. While the handler runs, the other signal is blocked. This prevents corruption of `(current_char, bit_count)`.

**Printing policy**:
- **Byte-complete printing**:
  - After assembling 8 bits, immediately `write` the completed byte (unless it’s `'\0'`).
  - This streams output (no buffering), avoids heap, and is safe in handlers.

**End-of-message**:
- On `'\0'`: `write("\n")`, set `client_pid = 0` to release the slot, reset byte state.

**ACK timing**:
- Send per-bit ACK **after** you update the byte state (so the sender never gets ahead of you).
- This creates a “lockstep” pipeline: send → process → ACK → send next.

---

## 16) Debugging & Troubleshooting

**Symptom: missing characters**  
- *Cause:* No per-bit ACK; too-small sleeps; signal coalescing.  
- *Fix:* Implement per-bit ACK; ensure the client really **waits** before sending next bit.

**Symptom: client hangs forever waiting for ACK**  
- *Cause:* Server died mid-flight; wrong PID; server policy ignoring a second client.  
- *Fix:* Add a client-side **timeout**; verify PID; ensure you’re not colliding with an ongoing transmission.

**Symptom: gibberish characters**  
- *Cause:* Assembling bits in the wrong order (MSB/LSB mismatch), or reentrant clobbering due to missing handler mask.  
- *Fix:* Standardize on MSB→LSB; set `sa_mask` to block both signals during handler.

**Symptom: crashes or weird I/O in handler**  
- *Cause:* Using non–signal-safe functions (e.g., `printf`, `malloc`).  
- *Fix:* Use only `write`, `kill`, and trivial state updates.

**Symptom: can’t send from two clients back-to-back**  
- *Cause:* Server didn’t reset `client_pid` on `'\0'`.  
- *Fix:* On end-of-message, print newline, reset sender PID and byte state.

---
