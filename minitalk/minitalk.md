# Minitalk

> “Treat a UNIX signal as a **single-bit courier**. Your job is to turn two couriers (SIGUSR1/SIGUSR2) into a byte stream.”

---

## Table of Contents

1. Signal Communication
2. Process IDs & `kill()`
3. Signal Delivery Semantics (pending set, coalescing, masks)
4. Signal Handlers (disposition, SA_SIGINFO, async-signal-safety)
5. Binary Encoding (bits → bytes, bit order vs endianness)
6. Bit-by-Bit Transmission (assembly logic)
7. Acknowledgment Protocol (pacing, flow control)
8. Race Conditions (the essentials)
9. Multiple Clients (policy)
10. Robustness (quick)
11. One Global Variable (why)
12. Unicode / UTF-8

---

## 1) Signal Communication

**What is a signal?** It's a small, asynchronous **notification** the kernel delivers to a process (or thread) to say “**something happened**.” 
It’s like a **software interrupt**: your program can be doing anything and a signal can arrive and briefly **divert** it to run a small **handler** function.

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

---

### Common UNIX Signals

Before focusing on `SIGUSR1` and `SIGUSR2`, here’s a quick reference of the most common POSIX signals.  
Use this to understand what types of signals exist, their default behavior, and what usually triggers them.

| **Signal** | **Value (POSIX)** | **Default Action** | **Description / Typical Cause**                 |
|------------|-------------------|--------------------|-------------------------------------------------|
| `SIGHUP`   | 1                 | Terminate          | Terminal hangup or controlling terminal closed  |
| `SIGINT`   | 2                 | Terminate          | Interrupt from keyboard (``` Ctrl+C ```)        |
| `SIGQUIT`  | 3                 | Core dump          | Quit from keyboard (``` Ctrl+\ ```)             |
| `SIGILL`   | 4                 | Core dump          | Illegal instruction                             |
| `SIGABRT`  | 6                 | Core dump          | Abort from ``` abort() ```                      |
| `SIGBUS`   | 7                 | Core dump          | Bus error (bad memory access)                   |
| `SIGFPE`   | 8                 | Core dump          | Floating-point exception (divide by zero, etc.) |
| `SIGKILL`  | 9                 | Terminate          | Immediate kill (cannot be handled or ignored)   |
| `SIGUSR1`  | 10                | Terminate          | User-defined signal #1 (free for app use)       |
| `SIGSEGV`  | 11                | Core dump          | Invalid memory reference (segmentation fault)   |
| `SIGUSR2`  | 12                | Terminate          | User-defined signal #2 (free for app use)       |
| `SIGPIPE`  | 13                | Terminate          | Write to a pipe with no reader                  |
| `SIGALRM`  | 14                | Terminate          | Timer signal from ``` alarm() ```               |
| `SIGTERM`  | 15                | Terminate          | Graceful termination request                    |
| `SIGCHLD`  | 17                | Ignore             | Child process stopped or terminated             |
| `SIGCONT`  | 18                | Continue           | Continue if stopped                             |
| `SIGSTOP`  | 19                | Stop               | Stop process (cannot be handled or ignored)     |
| `SIGTSTP`  | 20                | Stop               | Stop from keyboard (``` Ctrl+Z ```)             |
| `SIGTTIN`  | 21                | Stop               | Background process read from tty                |
| `SIGTTOU`  | 22                | Stop               | Background process write to tty                 |

>  **Note:**  
> Signal numbers may vary slightly depending on the OS (Linux, macOS, BSD).  
> You can check your system’s exact list with:  
> ``` man 7 signal ```

---

**In Minitalk**, only `SIGUSR1` and `SIGUSR2` are used — they are *user-defined* signals, meaning you can assign them custom meaning.  
Here, they represent binary **0** and **1**, forming the basis of the signal-based communication protocol.


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

- **Why ACK?** Because standard signals are **coalesced** (*multiple identical signals are merged into a single “pending” event—extras aren’t queued*). Without pacing, you **will** lose bits.
- **Per-bit ACK** (Stop-and-Wait):
  - After processing **one bit**, the server **sends an ACK signal** (e.g., `SIGUSR2`) to the client’s PID.
  - The client **waits** until it sees the ACK, then sends the **next bit**.
- **Timeouts**:
  - The client should not wait forever. Add a timeout while waiting for an ACK (the server might have died).
- **Message ACK (optional)**:
  - Sending a final ACK after '\0' creates an ordering risk (final before bit-ACK); handle it by treating the final ACK as the last-bit ACK.
  - Some designs send a **final ACK** (distinct signal) after `'\0'`. This can create a **race** with per-bit ACK for the last bit. If you keep it, design the client’s wait logic to accept the final ACK as satisfying the last per-bit wait.  
  - Simplest approach: **per-bit ACK only** (no final-ACK), avoiding that race.

---

## 8) Race Conditions (the essentials)
- **Coalescing:** identical signals merge; stop-and-wait (per-bit ACK) prevents loss.
- **Reentrancy:** block SIGUSR1/SIGUSR2 in the handler (`sa_mask`) so bit assembly isn’t interrupted mid-update.

---

## 9) Multiple Clients (policy)
- **One at a time:** the server locks to the first sender PID and **ignores others** until it receives `'\0'`.
- Other clients will **timeout** if they try during an active transfer; they can retry after.
- This satisfies “several clients **in a row**” without complex queuing.

---

## 10) Robustness (quick)
- Client validates PID (`kill(pid, 0)`), and has an **ACK timeout** to avoid hanging.
- Server resets state on `'\0'` and never uses non–signal-safe calls in the handler.

---

## 11) One global variable (why)

- POSIX handlers can’t take context and must avoid heap/stdio, so we need a tiny **persistent state**.
- **Server:** one struct `{ sender_pid, current_byte, bit_count }` → lock the sender, assemble 8 bits, detect `'\0'`. It’s **one** global variable.
- **Client:** one `volatile sig_atomic_t ack_flag` → set in the handler on ACK, polled by the sending loop.
- Safety: block `SIGUSR1/SIGUSR2` in `sa_mask`; handler does only trivial stores + `kill`/`write`; reset state on `'\0'`.

## 12) Unicode / UTF-8

- We transmit **bytes**; UTF-8 chars (1–4 bytes) pass through unchanged.
- `'\0'` marks end-of-message (UTF-8 never contains `0x00` inside a code point).
- Tip: use `unsigned char` when extracting bits to avoid sign issues.
