# Server

## Why global ? 

In server.c a global variable called g_server, stores the current state of the communication:

- things like which client is sending signals
- which character is being built
- how many bits have been received so far
 
It's global because of how signals work in UNIX:

- Signals are asynchronous, meaning they can arrive at any time interrupting the normal flow of the program.
- When a signal arrives, the operating system pauses what the program was doing and immediately calls the signal handler.

Signal handler:

- A signal handler in C can’t directly receive normal parameters or return values
- it’s just a function that gets called by the kernel with very limited information.

Conclusion:

So if I need to keep track of something between different signals for example, how many bits I’ve already received I need to store that information somewhere accessible to both the main program and the handler. 
For that I use my global state variable

---

## Sigaction struct

```
struct sigaction sa;

sa.sa_sigaction = handle_signal;     // tells which function to call when the signal arrives
// start with an empty signal set (clears any garbage values using sigemptyset)
sigemptyset(&sa.sa_mask);            // defines which signals to block while running the handler
sigaddset(&sa.sa_mask, SIGUSR1);     // add SIGUSR1 to the mask → block it while the handler runs
sigaddset(&sa.sa_mask, SIGUSR2);     // add SIGUSR2 to the mask → block it while the handler runs
sa.sa_flags = SA_SIGINFO;            // use the advanced 3-parameter-handler version to get sender information (PID)

```

***sa.sa_flags = 0 → uses the simple handler version:***
```void handler(int sig);```

***sa.sa_flags = SA_SIGINFO → uses the advanced 3-parameter-handler version:***
```void handle_signal(int sig, siginfo_t *info, void *context);```
- The advanced handler gives extra info such as the PID of the sender (info->si_pid), which is crucial so the server knows who to send the acknowledgment to.


## Sigaction function

> Note, sigaction struct is different than the sigaction() function

- The sigaction struct is the configuration (handler, mask and flags); 
- Sigaction() is the system call that applies that configuration to the operating system.

```
sigaction(SIGUSR1, &sa, NULL);   // apply our configuration for SIGUSR1
sigaction(SIGUSR2, &sa, NULL);   // apply our configuration for SIGUSR2
```

When the server receives ***SIGUSR1*** or ***SIGUSR2***,
the kernel checks the configuration using sigaction()
and then calls your handler function.

### more about sigaction() function

```int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact);```
signum → which signal you’re configuring (e.g., SIGUSR1)
act → pointer to your struct (&sa) that describes how to handle it
oldact → optional; if not NULL, it stores the previous configuration (you use NULL here)

### more about 3-parameter-handler 

```void handle_signal(int sig, siginfo_t *info, void *context);```
- sig → which signal was received (SIGUSR1 or SIGUSR2)
- info → structure with details about the sender (includes info->si_pid)
- context → system-level context information (not needed here, so ignored with (void)context

---

## handle_signal()

1)
If a client is already connected (client_pid != 0)
and this signal comes from a different client PID → ignore it
This prevents mixing messages from multiple clients

2)
// If the server is idle (no client currently communicating),
// store the PID of the new client (who sent the first signal)

3)
process_bit();

4)
// Send acknowledgment (SIGUSR2) back to the client
```kill(g_server.client_pid, SIGUSR2);```

5)
// When 8 bits are received → we have a complete character
// process_complete_char();

---

## process_bit()

Each time we receive a signal, shift the current byte to the left
and add the new bit at the rightmost position.

```
	if (sig == SIGUSR1)
		// SIGUSR1 represents bit = 0 → just shift left (adds 0 on the right)
		g_server.current_char <<= 1;
	else if (sig == SIGUSR2)
		// SIGUSR2 represents bit = 1 → shift left and add 1 on the right (using bitwise OR)
		g_server.current_char = (g_server.current_char << 1) | 1;
```

---

## process_complete_char()

// Called whenever 8 bits have been received → one full character completed

---

# Client

## Global variable:

```static volatile sig_atomic_t	g_ack_received = 0;```

***'volatile'***        → tells the compiler that this variable can change unexpectedly (for example, inside a signal handler).
                          It prevents compiler optimizations that could assume the value never changes, forcing the program to always read the real memory value.

***'sig_atomic_t'***    → guarantees atomic read/write (safe to modify inside a signal handler).
                          It’s a special integer type defined by the C standard that ensures the variable is
                          read or written in a single, indivisible operation, not in multiple CPU steps.
                          This prevents data corruption if a signal interrupts the program while accessing it. 
...

## Send_char()

```
bit_value = (c & (1 << bit_position));
```

**Idea:**  
Take one bit from the character by creating a mask with only that bit set to `1`.

**Example:**  
Character `A` → 65 → `01000001`

```
| bit_position | (1 << bit_position) | c (01000001) | c & mask | bit_value (0/1) |
|      7       | 10000000            | 01000001     | 00000000 |       0         |
|      6       | 01000000            | 01000001     | 01000000 |       1         |
|      5       | 00100000            | 01000001     | 00000000 |       0         |
|      4       | 00010000            | 01000001     | 00000000 |       0         |
|      3       | 00001000            | 01000001     | 00000000 |       0         |
|      2       | 00000100            | 01000001     | 00000000 |       0         |
|      1       | 00000010            | 01000001     | 00000000 |       0         |
|      0       | 00000001            | 01000001     | 00000001 |       1         |
```

**Result:**  
→ `bit_value` = 1 for positions 6 and 0 → these bits are sent as **SIGUSR2**  
→ all others are 0 → sent as **SIGUSR1**

**Explanation:**
- Sends one character bit by bit (MSB → LSB).
- `(1 << bit_position)` creates a mask to isolate each bit.
- `c & mask` checks if that bit is 0 or 1.
- Sends `SIGUSR1` for 0, `SIGUSR2` for 1.
- Waits for server acknowledgment after each bit.

## Wait_for_ack()

- The client sends one bit, then calls `wait_for_ack()`.
- It waits until `g_ack_received` becomes `1` —  
  this happens when the server replies with SIGUSR2,  
  and the signal handler `handle_ack()` sets the flag.
- If the server doesn’t reply after many small waits (≈5 seconds total),  
  it prints an error and exits → prevents infinite waiting.
- After acknowledgment, it resets the flag back to 0 for the next bit.

**Flow:**
1. Client sends bit → `kill(server_pid, SIGUSR1/2)`
2. Server receives and sends ACK → `kill(client_pid, SIGUSR2)`
3. Client’s `handle_ack()` runs → sets `g_ack_received = 1`
4. `wait_for_ack()` loop ends → next bit can be sent.
