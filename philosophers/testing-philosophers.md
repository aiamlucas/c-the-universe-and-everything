# Philosophers — Testing Guide

## Overview

The **Philosophers** project is an implementation of the classic **Dining Philosophers Problem**, which is about:
- Threading and processes
- Mutexes and synchronization
- Race conditions and deadlocks
- Resource sharing

### The Problem Setup

- **N philosophers** sit around a circular table
- Each philosopher alternates between **eating**, **thinking** and **sleeping**
- There are **N forks** (one between each pair of philosophers)
- A philosopher needs **2 forks** to eat (left and right)
- Philosophers must **not starve** (die from not eating in time)
- The simulation stops when a philosopher dies OR all philosophers have eaten enough times

### Key Parameters

```
./philo [number_of_philosophers] [time_to_die] [time_to_eat] [time_to_sleep] [number_of_times_each_must_eat]
```

### The Lifecycle

```
1. think  → philosopher thinks (no forks needed)
2. hungry → tries to pick up left and right fork
3. eat    → holds both forks, resets death timer
4. done   → puts down both forks
5. sleep  → sleeps for time_to_sleep ms
6. repeat from step 1
```

---

## Compilation

To test for data races with Helgrind, add `-g3` `-O0` to your compilation flags.

```
-g3   → adds full debug symbols
        Helgrind shows exact file names and line numbers
        without it: "at 0x4007A1: ???"
        with it:    "at 0x4007A1: philosopher_routine (philosopher.c:31)"

-O0   → disables compiler optimizations
        without it: compiler reorders/inlines code
                    Helgrind points to wrong lines
        with it:    Helgrind points to exactly what you wrote
```

---

## Test Cases

### Test 1: Basic Survival Test

```
./philo 5 800 200 200 (Should survive)
```

**Why:**
- `time_to_die (800ms)` > `time_to_eat (200ms)` + `time_to_sleep (200ms)` = 400ms

**Expected output:**
```
0 1 is thinking
0 3 is thinking
...
(runs indefinitely, no death message)
```

**What to check:**
- No philosopher dies
- All philosophers eat regularly
- Timestamps increase monotonically (never go backwards)
- No two status lines overlap/interleave

---

### Test 2: One Philosopher (Should die)

```
./philo 1 800 200 200
```

**Why:**
- Only ONE fork available, philosopher can never eat (needs 2)
- Must always die

**Expected output:**
```
0 1 has taken a fork
800 1 died
```

---

### Test 3: Tight Timing  (Should Survive)

```
./philo 4 410 200 200
```

**Why:**
- Very tight margin: `time_to_die=410ms`, cycle=`200+200=400ms`
- Only 10ms margin for error
- Tests timing precision, a slow implementation will fail this

**Expected output:**
```
(runs indefinitely, no death message)
```

**What to check:**
- Common causes of failure: using raw `usleep` instead of checked sleep,
  not updating `last_meal_time` immediately when eating starts

---

### Test 4: Impossible Timing  (Should die)

```
./philo 4 310 200 200
```

**Why:**
- Impossible: `time_to_die=310ms` but cycle=`200+200=400ms`
- At least one philosopher MUST die, tests death detection

**Expected output:**
```
...
XXX N died
```

---

### Test 5: Limited Meals  (Clean Stop after 7 meals)

```
./philo 5 800 200 200 7
```

**Why:**
- Tests the optional 5th argument
- Simulation must stop when ALL philosophers have eaten 7 times
- Must NOT print "died"

**Expected output:**
```
...
(simulation stops cleanly, no death message)
```
---

### Test 6: Two Philosophers (Should Survive)

```
./philo 2 800 200 200
```

---

### Test 7: Large Number of Philosophers (Should Survive)

```
./philo 200 800 200 200
```

**Why:**
- Stress test, 200 threads running simultaneously
- More threads = more chance of race conditions appearing

---

### Test 8: Death Timing Accuracy (Should Die whithin 10ms)

```
./philo 4 310 200 100
```

**Why:**
- Tests if death is reported within 10ms (subject requirement)
- `time_to_die=310ms` is impossible to survive

**What to check:**
```
death reported at Xms
last meal was at Yms
X - Y must be <= time_to_die + 10ms  (subject allows 10ms tolerance)
```

---

### Test 9: Input Validation

```
# wrong number of arguments
./philo
./philo 5
./philo 5 800 200

# negative values
./philo -1 800 200 200
./philo 5 -800 200 200

# zero values
./philo 0 800 200 200
./philo 5 0 200 200

# overflow
./philo 2147483648 800 200 200
./philo 5 999999999999999999999 200 200

# non-numeric
./philo abc 800 200 200
./philo 5 800 200 abc

# signs
./philo +5 800 200 200
./philo 5 +800 200 200
```

**What to check:**
- All invalid inputs print a clear error message
- Program exits with failure (non-zero exit code)
- No crash, no undefined behavior

---

### Test 10: Data Race Detection with Helgrind

```
# compile with debug flags first
make HELGRIND=1

# run helgrind
valgrind --tool=helgrind ./philo 5 800 200 200 20
```

**What to look for:**

The only line that matters:
```
ERROR SUMMARY: 0 errors from 0 contexts (suppressed: X from Y)
```

```
0 errors   → clean
X errors   → data races or lock order violations
suppressed → internal glibc/pthread noise — always ignore
```

**Example of clean output:**
```
==12345== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 86457 from 124)
```

**Example of data race found:**
```
==12345== Possible data race during write of size 4
==12345==    at 0x401234: philosopher_routine (philosopher.c:42)
==12345==  This conflicts with a previous read of size 4
==12345==    at 0x401198: check_phil_death (monitor.c:67)
```

**Example of lock order violation:**
```
==12345== Thread #2: lock order "death_mutex before print_mutex" violated
==12345==    Holding death_mutex
==12345==    Trying to acquire print_mutex
==12345==    But thread #1 previously acquired them in reverse order
```

Lock order violation means: mutexes are locked in different orders in
different places, this will eventually cause a deadlock even if the
program seems to work fine during testing.

**Save output to file for easier analysis:**
```
valgrind --tool=helgrind --log-file=helgrind.log ./philo 5 800 200 200 5

# search for specific issues:
grep "data race" helgrind.log
grep "lock order" helgrind.log
grep "ERROR SUMMARY" helgrind.log
```

---

### Test 11: Helgrind with multiple scenarios

```
# test with must_eat_count (clean stop)
valgrind --tool=helgrind ./philo 5 800 200 200 5

# test with death
valgrind --tool=helgrind ./philo 2 500 300 300
```

All should show `ERROR SUMMARY: 0 errors`.
