# Philosophers

> "Five philosophers sit around a table eating sushi with chopsticks. 
> Each needs two chopsticks to eat. Don't let them starve!"

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

# Representing the Problem in C

Each philosopher becomes a **thread** (unit of execution), each chopstick becomes a **mutex** (lockable resource):

```
philosopher[1] ──── thread ──── runs independently
philosopher[2] ──── thread ──── runs independently
philosopher[3] ──── thread ──── runs independently

chopstick[0] ──── mutex ──── only one thread can hold it at a time
chopstick[1] ──── mutex ──── only one thread can hold it at a time
chopstick[2] ──── mutex ──── only one thread can hold it at a time
```

Round table layout:
```
chopstick[0]-philo[1]-chopstick[1]-philo[2]-chopstick[2]-philo[3]-chopstick[3]-philo[4]-chopstick[4]-philo[5]-chopstick[0]
                                                                                        ↑
                                                                             wraps back (% n)
```

---

### The Shared Data Structure

All threads need access to the same chopsticks, timings and stop flag.
Instead of global variables (forbidden), one shared struct `t_data` is passed
to every thread via a pointer:

```
          t_data (shared by all threads)
         ┌────────────────────────────────────────────┐
         │ time_to_die   time_to_eat                  │
         │ time_to_sleep must_eat_count               │
         │ start_time    dead_flag                    │
         │                                            │
         │ chopsticks → [mutex0][mutex1][mutex2]...   │
         │                                            │
         │ print_mutex   death_mutex                  │
         │                                            │
         │ philos → [philo1][philo2][philo3]...       │
         └────────────────────────────────────────────┘
              ↑           ↑           ↑
           thread1     thread2     thread3
           (philo1)     (philo2)    (philo3)
```

Each philosopher struct holds its own state:
```
t_philo
┌───────────────────────────────────┐
│ id                 = 1            │
│ left_chopstick     = 0            │  → index into data->chopsticks[]
│ right_chopstick    = 1            │  → index into data->chopsticks[]
│ last_meal_time                    │  → updated every meal
│ meals_eaten        = 0            │  → incremented each meal
│ *data         ───────────────────────→ points back to t_data
└───────────────────────────────────┘
```

---

### Memory & Initialization

Two heap allocations happen during init 
everything else lives on the stack inside t_data:

```
STACK                          HEAP
─────                          ────
t_data                         
┌──────────────────┐           
│ n_philos = 5     │           
│ time_to_die      │           
│ dead_flag = 0    │           
│                  │           
│ *chopsticks ──────────────────→  [mutex0][mutex1][mutex2][mutex3][mutex4]
│                  │                  40 bytes each (Linux x86_64)
│ print_mutex      │           
│ death_mutex      │           
│                  │           
│ *philos ─────────────────→  [t_philo][t_philo][t_philo][t_philo][t_philo]
└──────────────────┘           id=1      id=2     id=3     id=4     id=5
                                │
                                └── each t_philo:
                                    ┌──────────────────────┐
                                    │ id                   │
                                    │ pthread_t thread     │ ← thread ID stored here
                                    │ left_chopstick       │ ← index into chopsticks[]
                                    │ right_chopstick      │ ← index into chopsticks[]
                                    │ last_meal_time       │
                                    │ meals_eaten          │
                                    │ *data ─────────────────→ points back to t_data
                                    └──────────────────────┘
```

---

### Why *data inside each philosopher

`pthread_create` only accepts ONE argument: `&philos[i]`.
But each thread needs access to shared resources:
```
thread receives → &philos[i]
                      │
                      └── philos[i].data ──→ t_data
                                                ├── chopsticks[]   (to eat)
                                                ├── print_mutex    (to print)
                                                ├── death_mutex    (to check state)
                                                ├── time_to_die    (timing)
                                                └── dead_flag      (stop signal)
```

Embedding `*data` inside `t_philo` solves this
one pointer gives the thread access to everything it needs.

---

### Shared Resources
```
chopsticks[]       → the eating tools: philosophers compete to lock them
                   one mutex per chopstick, only one philosopher holds it at a time

print_mutex   → stdout: only one thread can printf at a time
                without it: two threads print simultaneously → garbled output

death_mutex   → simulation state: protects dead_flag, last_meal_time, meals_eaten
                written by: philosopher threads (last_meal_time, meals_eaten)
                            monitor (dead_flag)
                read by:    philosopher threads (dead_flag)
                            monitor (last_meal_time, meals_eaten)
```

---

### Monitor, not a thread, runs in main

```
after create_threads() returns:

main thread  →  monitor_simulation()   ← loops here watching for deaths
thread[1]    →  philosopher_routine()  ← eat / think / sleep
thread[2]    →  philosopher_routine()  ← eat / think / sleep
thread[3]    →  philosopher_routine()  ← eat / think / sleep
...

monitor has no pthread_t — it IS the main thread
no entry in t_data — it uses t_data directly via pointer
```

---

### The Routine: philosopher_routine

A routine is the function a thread executes. 
Passed to `pthread_create` as a function pointer.
When the thread starts, it calls this function.
When the function returns, the thread exits.

```
pthread_create(&philos[i].thread, NULL, philosopher_routine, &philos[i])
                                         ↑
                                   function pointer
                                   thread starts executing here
```

The routine signature is always the same for pthreads:
```
void *routine(void *arg)
  ↑                ↑
  returns void*    receives void* (cast to t_philo* inside)
```

philosopher_routine is the lifetime of one philosopher:
```
philosopher_routine(&philos[i])
│
├── if even id → usleep(1ms)    ← stagger start to prevent deadlock (minimal delay so they don't fight for the chopstick at the same time)
│
└── loop until simulation_ended():
        │
        ├── think  → print "is thinking"
        │
        ├── eat    → take chopsticks (lock 2 mutexes)
        │            print "is eating"
        │            update last_meal_time + meals_eaten
        │            sleep time_to_eat
        │            drop chopsticks (unlock 2 mutexes)
        │
        └── sleep  → print "is sleeping"
                     sleep time_to_sleep
```

Each philosopher thread runs this loop independently and in parallel.
They share chopsticks (mutexes) and check dead_flag to know when to stop
but never communicate directly with each other.
```
thread[1]: think → eat → sleep → think → eat → ...
thread[2]:    think → eat → sleep → think → eat → ...
thread[3]:       think → eat → sleep → think → ...
               ↑ all running at the same time, independently
```

---


### Cleanup Order (reverse of init)
```
1. pthread_mutex_destroy for each chopstick    (n destroys)
2. pthread_mutex_destroy print_mutex
3. pthread_mutex_destroy death_mutex
4. free(chopsticks)                            (heap)
5. free(philos)                                (heap)
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

philo[1] grabs chopstick[0] → waits for chopstick[1]
philo[2] grabs chopstick[1] → waits for chopstick[2]
philo[3] grabs chopstick[2] → waits for chopstick[0]
→ circular wait → nobody moves → deadlock!!!

WITH ordering (always lock lower index first):

philo[1]: lock chopstick[0] → lock chopstick[1]
philo[2]: lock chopstick[1] → lock chopstick[2]
philo[3]: lock chopstick[0] → lock chopstick[2]  (right < left → reversed)
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
                  even if philosopher died at 10ms

ft_usleep_check:  █ check █ check █ check → dead_flag=1 → exit
                  wakes every 500µs, exits as soon as simulation ends
```
