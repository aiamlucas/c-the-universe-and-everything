# Philosophers

> "Five philosophers sit around a table eating sushi with chopsticks. 
> Each needs two chopsticks to eat. Don't let them starve!"

---

## Table of Contents

1. The Dining Philosophers Problem
2. CPU, Cores and Threads
3. Processes vs Threads
4. POSIX Threads (pthreads)
5. Mutexes (Mutual Exclusion)
6. Race Conditions
7. Deadlock
8. Thread Synchronization Patterns
9. Time Management (`gettimeofday`, `usleep`)
10. Data Races vs Race Conditions

---

## 1) The Dining Philosophers Problem

**What is it?** A classic synchronization problem in computer science, proposed by Edsger Dijkstra in 1965.


### The Round Table Setup

```
                   🥢0
             Phi5        Phi1

           🥢4              🥢1
                    
         Phi4                Phi2
                  
             🥢3            🥢2
                 
                    Phi3


With 5 philosophers arranged in a circle:
- Phi1 needs 🥢0 (left) and 🥢1 (right)
- Phi2 needs 🥢1 (left) and 🥢2 (right)
- Phi3 needs 🥢2 (left) and 🥢3 (right)
- Phi4 needs 🥢3 (left) and 🥢4 (right)
- Phi5 needs 🥢4 (left) and 🥢0 (right) ← wraps around!
```

### The Setup

- **N philosophers** sit at a round table
- **N chopsticks** placed between them (one between each pair)
- Each philosopher alternates between three activities:
  - **EATING** (requires 2 chopsticks: left AND right)
  - **THINKING** (requires no chopsticks)
  - **SLEEPING** (requires no chopsticks)

### The Challenge

- Philosophers **share chopsticks** (limited resources)
- They **don't communicate** with each other (no messaging, etc)
- They **must not starve** (die from not eating)
- The system **must not deadlock**  (all threads stuck waiting for each other in a circle → nobody can proceed)

### Real-World Analogy

Think of it like multiple programs trying to access shared resources (files, memory, network connections). 
If not coordinated properly, they can:

- **Deadlock**: all waiting for each other → nobody ever moves
  *(every philosopher holds one chopstick and waits for the other forever)*
→ technically: every thread is blocked waiting for a resource held by another
  thread in the same chain: since nobody releases, nobody can ever continue

- **Starvation**: some processes never get resources → they die waiting
  *(one philosopher always loses the race to grab chopsticks to faster neighbors)*
→ technically: a thread keeps getting skipped, other threads always get the
  resource first, so this thread waits forever despite never being truly blocked

- **Race condition**: corrupt data by simultaneous access → wrong results
  *(two philosophers both "see" a chopstick as free and grab it at the same time)*
  → technically: two threads read and write the same data at the same time
  without coordination. The result depends on who goes first, which is random/inconsistent

---

## 2) CPU, Cores, and Threads

Before diving into processes and threads, it’s important to understand how the CPU actually runs them.

### The CPU Hierarchy

| **Component**        | **Description**                                                                                                                              |
|----------------------|----------------------------------------------------------------------------------------------------------------------------------------------|
| **CPU (Processor)**  | The physical chip that performs computation. A typical personal computer usually has one CPU                                                 |
| **Core**             | An independent execution unit inside the CPU. Each core can execute its own thread of instructions.                                          |
| **Hardware Thread**  | Also called a *logical core*. Some CPUs use Simultaneous Multithreading (SMT) or Hyper-Threading to run two threads per core simultaneously. |
| **Software Thread**  | The thread you create in your program (e.g., using `pthread_create`). The operating system scheduler assigns these to hardware threads.      |

```
┌─────────────────────────────────────────┐
│                  CPU                    │
│  ┌─────────────┐   ┌─────────────┐      │
│  │   Core 0    │   │   Core 1    │  ... │
│  │ ┌───┐ ┌───┐ │   │ ┌───┐ ┌───┐ │      │
│  │ │T 0│ │T 1│ │   │ │T 2│ │T 3│ │      │
│  │ └───┘ └───┘ │   │ └───┘ └───┘ │      │
│  │  (SMT: 2    │   │  (SMT: 2    │      │
│  │  threads)   │   │  threads)   │      │
│  └─────────────┘   └─────────────┘      │
└─────────────────────────────────────────┘
```

CPU     → the physical chip on your motherboard
Core    → independent brain inside the CPU, has its own
          registers, execution units and L1/L2 cache.
          Two cores never share work, they run truly in parallel.
Thread  → the actual stream of instructions a core executes.
          With SMT (hyper-threading), one core can juggle two
          threads by filling idle execution slots.


### Example: Ryzen 7 CPU

Let’s take an AMD Ryzen 7 5800X as a real-world example:

| **Property**                  | **Value**                  |
|-------------------------------|----------------------------|
| **Physical CPUs**             | 1 (one processor chip)     |
| **Cores per CPU**             | 8 cores                    |
| **Hardware Threads per Core** | 2 (SMT enabled)            |
| **Total Hardware Threads**    | 8 × 2 = 16 logical threads |

So the operating system sees 16 logical CPUs, meaning up to 16 software threads can run truly in parallel.  
If you create more than 16 threads, the OS will time-slice them, giving each a short turn on the cores.

### How Execution Works

1. Each core can run one (or two with SMT/Hyper-Threading) threads at once.  
2. The operating system scheduler decides which software threads run on which cores.  
3. If there are more threads than cores, the OS quickly switches between them (context switching).  
4. This creates:  
   - **True parallelism** when enough cores exist  
   - **Concurrent execution** when threads share cores via time slicing

---

## 3) Processes vs Threads

Before diving into threads, it's important to understand the difference from processes and threads:

| **Aspect**        | **Process**                 | **Thread**                      |
|-------------------|-----------------------------|---------------------------------|
| **Memory Space**  | Separate (isolated)         | Shared within process           |
| **Creation Cost** | Expensive (fork syscall)    | Cheap (just new stack)          |
| **Communication** | IPC needed (pipes, signals) | Direct (shared variables)       |
| **Crash Impact**  | Isolated                    | Can crash entire process        |
| **Use Case**      | Independent programs        | Concurrent tasks in one program |

### Why Threads for Philosophers?
- **Shared memory**: All philosophers can see the same chopsticks (mutex array)
- **Fast**: Creating 200 philosopher threads is much faster than 200 processes
- **Simple communication**: No need for complex IPC


---

## 4) POSIX Threads (pthreads)

### Key Thread Functions

#### `pthread_create()`
Creates and starts a new thread.

```
int pthread_create(pthread_t *thread,             // thread ID (output)
                   const pthread_attr_t *attr,    // attributes (usually NULL)
                   void *(*start_routine)(void*), // function to run
                   void *arg);                    // argument to pass

// Example:
pthread_t philo_thread;
t_philo philo_data = {...};
pthread_create(&philo_thread, NULL, philosopher_routine, &philo_data);
```

- **Returns**: 0 on success, error code otherwise
- **`start_routine`**: The function the thread will execute
- **`arg`**: Pointer passed to your function (your philosopher's data)

#### `pthread_join()`
Waits for a thread to finish (blocks until thread terminates).

```
int pthread_join(pthread_t thread, void **retval);

// Example:
pthread_join(philo_thread, NULL);  // Wait for philosopher to finish
```

- Like `waitpid()` for processes
- **Main thread must join** all philosopher threads before exiting
- **Retval**: Can capture thread's return value (usually NULL for this project)


### Thread Lifecycle

```
[CREATED] --pthread_create()--> [RUNNING] --function returns--> [TERMINATED]
                                    |
                                    |--pthread_join()--> [JOINED/CLEANED UP]
```

### Example: Basic Thread

```
void *print_message(void *arg)
{
    char *msg = (char *)arg;
    printf("%s\n", msg);
    return (NULL);
}

int main(void)
{
    pthread_t thread;
    char *message = "Hello from thread!";
    
    pthread_create(&thread, NULL, print_message, message);
    pthread_join(thread, NULL);  // Wait for completion
    return (0);
}
```

---

## 5) Mutexes (Mutual Exclusion)

### What is a Mutex?

A **mutex** (mutual exclusion lock) is like a **lock on a door**. Only one thread can hold the lock at a time.

**In Philosophers**: Each Chopstick is protected by a mutex.

### Why Do We Need Mutexes?

Without mutexes:
```
// Thread 1                    // Thread 2
chopsticks[1] = TAKEN;              chopsticks[1] = TAKEN;  // Both think they got it!
// Both philosophers now "have" Chopstick 1 → CONFLICT
```

With mutexes:
```
// Thread 1                              // Thread 2
pthread_mutex_lock(&chopsticks[1]);      pthread_mutex_lock(&chopsticks[1]);
chopsticks[1] = TAKEN;                   // BLOCKED - waiting for Thread 1
pthread_mutex_unlock(&chopsticks[1]);
                                         // NOW Thread 2 can proceed
                                         chopsticks[1] = TAKEN;
```

### Key Mutex Functions

#### `pthread_mutex_init()`
Initializes a mutex.

```
int pthread_mutex_init(pthread_mutex_t *mutex,
                       const pthread_mutexattr_t *attr);

// Example:
pthread_mutex_t chopstick_mutex;
pthread_mutex_init(&chopstick_mutex, NULL);  // NULL = default attributes
```

#### `pthread_mutex_destroy()`

Destroys a mutex (frees resources).

```
int pthread_mutex_destroy(pthread_mutex_t *mutex);

// Must be called when done with mutex
pthread_mutex_destroy(&chopstick_mutex);
```

#### `pthread_mutex_lock()`

Acquires the mutex (blocks if already locked).

```
int pthread_mutex_lock(pthread_mutex_t *mutex);

// Thread will WAIT here if another thread holds the lock
pthread_mutex_lock(&chopstick_mutex);
// Critical section, only one thread at a time
pthread_mutex_unlock(&chopstick_mutex);
```

#### `pthread_mutex_unlock()`

Releases the mutex.

```
int pthread_mutex_unlock(pthread_mutex_t *mutex);
```

**CRITICAL**: Always unlock what you lock! Forgetting this causes deadlocks.

### Mutex Example: Protecting Shared Data

```
pthread_mutex_t print_mutex;
int shared_counter = 0;

void *increment(void *arg)
{
    for (int i = 0; i < 1000000; i++)
    {
        pthread_mutex_lock(&print_mutex);
        shared_counter++;  // Protected operation
        pthread_mutex_unlock(&print_mutex);
    }
    return (NULL);
}
```

Without the mutex, `shared_counter` would have a **race condition** and the final value would be unpredictable.

---

## 6) Race Conditions

### What is a Race Condition?

When **multiple threads access shared data** and at least one is **writing** and the **outcome depends on timing**.

### Example: The Classic Race

```
// Global variable (shared by all threads)
int balance = 1000;

// Thread 1: Deposit $100        // Thread 2: Withdraw $50
int temp1 = balance;             int temp2 = balance;
temp1 = temp1 + 100;             temp2 = temp2 - 50;
balance = temp1;                 balance = temp2;
```

**Possible outcomes**:
1. Thread 1 first, Thread 2 second → `1050` ✓
2. Thread 2 first, Thread 1 second → `1100` ✓
3. **INTERLEAVED** → Could be `1050`, `950`, or anything!

### Race in Philosophers

```
// BAD: No mutex protection
if (chopsticks[left] == AVAILABLE)  // Check
{
    chopsticks[left] = TAKEN;       // Set
}

// Two philosophers could BOTH pass the check before either sets it!
```

**Solution**: Atomic operation with mutex:
```
pthread_mutex_lock(&Chopstick[left]);
// Now ONLY this thread can access Chopstick[left]
chopsticks[left] = TAKEN;
pthread_mutex_unlock(&Chopstick[left]);
```

---

## 7) Deadlock

### What is Deadlock?

When **all threads are waiting** for resources held by each other → **nobody can proceed**.

### The Classic Philosophers Deadlock

```
Philosopher 1: Holds Chopstick 1, wants Chopstick 2
Philosopher 2: Holds Chopstick 2, wants Chopstick 3
Philosopher 3: Holds Chopstick 3, wants Chopstick 4
Philosopher 4: Holds Chopstick 4, wants Chopstick 1
```

**Everyone is waiting. Forever. Deadlock.**

### Deadlock Conditions (All 4 must be true)

1. **Mutual Exclusion**: Resources can't be shared (chopsticks can't be split)
2. **Hold and Wait**: Thread holds resources while waiting for more
3. **No Preemption**: Can't forcibly take resources from a thread
4. **Circular Wait**: Circular chain of threads waiting for each other

### Breaking Deadlock in Philosophers

**Strategy 1: Different lock order**
```
// Even philosophers: left Chopstick first
if (philo_id % 2 == 0)
{
    pthread_mutex_lock(&chopsticks[left]);
    pthread_mutex_lock(&chopsticks[right]);
}
// Odd philosophers: right Chopstick first
else
{
    pthread_mutex_lock(&chopsticks[right]);
    pthread_mutex_lock(&chopsticks[left]);
}
```

**Strategy 2: Global ordering**
```
// Always lock the lower-numbered Chopstick first
int first = (left < right) ? left : right;
int second = (left < right) ? right : left;

pthread_mutex_lock(&chopsticks[first]);
pthread_mutex_lock(&chopsticks[second]);
```

**Strategy 3: Delay odd philosophers**
```
if (philo_id % 2 == 1)
    usleep(100);  // Give even philosophers a head start
```

---

## 8) Thread Synchronization Patterns

### Pattern 1: Stop-and-Wait (Used in Minitalk)

One signal at a time with acknowledgment.

### Pattern 2: Producer-Consumer

One thread produces data, another consumes it

### Pattern 3: All-Start-Together

All threads start simultaneously, but coordinate with timing/mutexes.

```
// Create all threads first
for (int i = 0; i < n_philos; i++)
    pthread_create(&philos[i], NULL, routine, &data[i]);

// They all start running at roughly the same time
```

### The Chopstick Dance: Philosophers Pattern

```
void *philosopher_routine(void *arg)
{
    t_philo *philo = (t_philo *)arg;
    
    while (!simulation_ended())
    {
        think(philo);
        
        // ACQUIRE chopsticks (critical!)
        take_forks(philo);
        
        // EAT (protected by holding chopsticks)
        eat(philo);
        
        // RELEASE chopsticks
        drop_forks(philo);
        
        sleep_philo(philo);
    }
    return (NULL);
}
```

---

## 9) Time Management

### `gettimeofday()`

Gets current time with microsecond precision.

```
#include <sys/time.h>

int gettimeofday(struct timeval *tv, struct timezone *tz);

struct timeval {
    time_t      tv_sec;   // seconds
    suseconds_t tv_usec;  // microseconds
};

// Example: Get timestamp in milliseconds
long long get_time_ms(void)
{
    struct timeval tv;
    
    gettimeofday(&tv, NULL);
    return ((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}
```

### `usleep()`

Suspends execution for microseconds.

```
#include <unistd.h>

int usleep(useconds_t microseconds);

// Example: Sleep for 200ms
usleep(200 * 1000);  // 200,000 microseconds = 200ms
```

**Note**: `usleep` is not perfectly precise. Actual sleep time may be slightly longer due to scheduling.

### Timestamp Pattern in Philosophers

```
// At start of simulation
long long start_time = get_time_ms();

// In philosopher thread
long long current_time = get_time_ms();
long long elapsed = current_time - start_time;
printf("%lld %d is eating\n", elapsed, philo_id);
```

### Death Timer Pattern

```
typedef struct s_philo
{
    long long last_meal_time;  // When they last started eating
    long long time_to_die;     // How long they can survive
    // ... other fields
} t_philo;

// Check if philosopher died
long long time_since_meal = get_time_ms() - philo->last_meal_time;
if (time_since_meal > philo->time_to_die)
{
    printf("%lld %d died\n", get_time_ms() - start_time, philo->id);
    // End simulation
}
```

---

## 10) Data Races vs Race Conditions

### Data Race

**Simultaneous access** to the same memory location by multiple threads, where at least one is writing, **with no synchronization**.

```
// DATA RACE (undefined behavior)
// Thread 1              // Thread 2
x = 1;                   x = 2;

// The value of x is unpredictable
// The program behavior is undefined
```

### Race Condition

**The outcome depends on timing** of thread execution.

```
// RACE CONDITION (logic error)
// Thread 1                          // Thread 2
if (account_balance >= 100)         if (account_balance >= 100)
    account_balance -= 100;             account_balance -= 100;

// If both check before either subtracts, balance could go negative!
```

### In Philosophers

- **Data race**: Reading/writing to `last_meal_time` without mutex protection
- **Race condition**: Two philosophers trying to grab the same Chopstick at once

**Solution for both**: Use mutexes correctly!

---

## Summary: Key Concepts

| **Concept**        | **What**                  | **Why in Philosophers**                     |
|--------------------|---------------------------|---------------------------------------------|
| **Thread**         | Parallel execution flow   | Each philosopher runs independently         |
| **Mutex**          | Lock protecting resource  | Each Chopstick needs protection             |
| **Deadlock**       | All threads stuck waiting | Can happen if all grab left Chopstick first |
| **Race Condition** | Timing-dependent bug      | Multiple philos accessing same Chopstick    |
| **Data Race**      | Unprotected memory access | Reading/writing `last_meal_time`            |
| **gettimeofday**   | Get precise timestamp     | Track when philosophers last ate            |
| **usleep**         | Sleep for microseconds    | Simulate eating/sleeping time               |

---

## Quick Reference: Function Prototypes

```
// Thread functions
int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                   void *(*start_routine)(void*), void *arg);
int pthread_join(pthread_t thread, void **retval);
int pthread_detach(pthread_t thread);

// Mutex functions
int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);
int pthread_mutex_destroy(pthread_mutex_t *mutex);
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);

// Time functions
int gettimeofday(struct timeval *tv, struct timezone *tz);
int usleep(useconds_t microseconds);
```

---


# Representing the Problem in C

Each philosopher becomes a **thread** (unit of execution), each fork becomes a **mutex** (lockable resource):


```
philosopher[1] ──── thread ──── runs independently
philosopher[2] ──── thread ──── runs independently
philosopher[3] ──── thread ──── runs independently

fork[0] ──── mutex ──── only one thread can hold it at a time
fork[1] ──── mutex ──── only one thread can hold it at a time
fork[2] ──── mutex ──── only one thread can hold it at a time
```

Round table layout:
```
fork[0]-philo[1]-fork[1]-philo[2]-fork[2]-philo[3]-fork[3]-philo[4]-fork[4]-philo[5]-fork[0]
                                                                                        ↑
                                                                             wraps back (% n)
```

---

### The Shared Data Structure

All threads need access to the same forks, timings and stop flag.
Instead of global variables (forbidden), one shared struct `t_data` is passed
to every thread via a pointer:

```
          t_data (shared by all threads)
         ┌─────────────────────────────────────┐
         │ time_to_die   time_to_eat            │
         │ time_to_sleep must_eat_count         │
         │ start_time    dead_flag              │
         │                                     │
         │ forks → [mutex0][mutex1][mutex2]...  │
         │                                     │
         │ print_mutex   death_mutex            │
         │                                     │
         │ philos → [philo1][philo2][philo3]... │
         └─────────────────────────────────────┘
              ↑           ↑           ↑
           thread1     thread2     thread3
         (philo1)     (philo2)    (philo3)
```

Each philosopher struct holds its own state:
```
t_philo
┌──────────────────────────────┐
│ id            = 1            │
│ left_fork     = 0            │  → index into data->forks[]
│ right_fork    = 1            │  → index into data->forks[]
│ last_meal_time               │  → updated every meal
│ meals_eaten   = 0            │  → incremented each meal
│ *data         ──────────────────→ points back to t_data
└──────────────────────────────┘
```

---

### Why Locking Is Necessary

Threads run truly in parallel: two threads can read and write the
same variable at the exact same moment.
Without a mutex, the result is unpredictable. 

This is called a data race:

```
WITHOUT MUTEX:

thread1 reads  dead_flag → 0 (running)
thread2 writes dead_flag → 1 (ended)
thread1 still thinks simulation is running → data race!!!

WITH MUTEX:

thread1: lock → reads dead_flag → unlock
thread2: lock → (blocked, waits) ────────→ lock → writes dead_flag → unlock
thread1 always reads a consistent value
```

---

### The Monitor

A monitor is a dedicated watcher: a separate thread (the main thread)
whose only job is to observe the simulation and decide when it ends.
It never eats, thinks or sleeps, it just polls:

```
                    ┌─────────────────┐
                    │  MONITOR        │
                    │  (main thread)  │
                    │                 │
                    │  every 1ms:     │
                    │  check each     │
                    │  last_meal_time │
                    └────────┬────────┘
                             │
              ┌──────────────┼──────────────┐
              ↓              ↓              ↓
         philo[1]        philo[2]       philo[3]
         thread          thread          thread
```

Philosophers never check each other, only the monitor reads
all philosophers and decides when simulation ends.

---

### Deadlock Prevention
```
WITHOUT ordering — deadlock:

philo[1] grabs fork[0] → waits for fork[1]
philo[2] grabs fork[1] → waits for fork[2]
philo[3] grabs fork[2] → waits for fork[0]
→ circular wait → nobody moves → deadlock!!!

WITH ordering (always lock lower index first):

philo[1]: lock fork[0] → lock fork[1]
philo[2]: lock fork[1] → lock fork[2]
philo[3]: lock fork[0] → lock fork[2]  (right < left → reversed)
→ circle is broken → at least one philosopher always eats
```

---

### Output Integrity

printf is not thread-safe. Two threads printing at the same time
produce interleaved output. 
A dedicated mutex ensures only one thread can print at a time:

```
WITHOUT print_mutex:

thread1: printf("42 1 is eati-             ← interrupted
thread2:                    -ng\n47 2 is thinking\n")  ← interleaved

WITH print_mutex:

thread1: lock → printf("42 1 is eating\n")   → unlock
thread2: lock → (blocked) ──────────────────→ printf("47 2 is thinking\n") → unlock
one line at a time, always clean
```

---

### ft_usleep_check — Why Not Just usleep?

usleep sleeps the full duration no matter what happens. 
Threads would keep sleeping even after simulation ends. 
`ft_usleep_check` wakes up every 500µs to check if the simulation ended:

```
usleep(800ms):    ████████████████████████████████ sleeps full 800ms
                  even if philosopher died at 10ms ❌

ft_usleep_check:  █ check █ check █ check → dead_flag=1 → exit
                  wakes every 500µs, exits as soon as simulation ends
```
