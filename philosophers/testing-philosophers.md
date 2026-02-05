# Philosophers

## Overview

The **Philosophers** project is an implementation of the classic **Dining Philosophers Problem**, which teaches:
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

- **number_of_philosophers**: Number of philos and forks
- **time_to_die** (ms): If a philo doesn't start eating within this time, they die
- **time_to_eat** (ms): Time it takes to eat
- **time_to_sleep** (ms): Time spent sleeping
- **number_of_times_each_must_eat** (optional): Simulation stops when all have eaten this many times

### The Lifecycle

1. Philosopher **thinks**
2. Philosopher gets **hungry**, tries to pick up forks
3. Philosopher **eats** (resets their death timer)
4. Philosopher puts down forks
5. Philosopher **sleeps**
6. Repeat from step 1

---

## Test Cases

### Test 1: Basic Survival Test
```
// [number_of_philosophers] [time_to_die] [time_to_eat] [time_to_sleep] [number_of_times_each_must_eat]
./philo 5 800 200 200
```

**Why test this:**
- Standard case with 5 philosophers
- `time_to_die (800ms)` > `time_to_eat (200ms)` + `time_to_sleep (200ms)`
- Should run indefinitely without deaths
- Tests basic synchronization

**What to check:**
- No philosopher should die
- All philosophers should eat regularly
- No timestamps should be negative or out of order


### Test 2: One Philosopher (Edge Case)
```
// [number_of_philosophers] [time_to_die] [time_to_eat] [time_to_sleep] [number_of_times_each_must_eat]
./philo 1 800 200 200
```

**Why test this:**
- Only ONE fork available
- Philosopher cannot eat (needs 2 forks)
- Should die after 800ms

**What to check:**
- Philosopher should die at exactly ~800ms
- Should print "died" message


### Test 3: Tight Timing - Should Survive
```
// [number_of_philosophers] [time_to_die] [time_to_eat] [time_to_sleep] [number_of_times_each_must_eat]
./philo 4 410 200 200
```

**Why test this:**
- Very tight timing: 410ms to die, 200ms eat + 200ms sleep = 400ms
- Only 10ms margin for error
- Tests precision of timing

**What to check:**
- No philosopher should die
- All should eat on time
- Tests if implementation has efficient fork pickup


### Test 4: Impossible Timing - Should Die
```
// [number_of_philosophers] [time_to_die] [time_to_eat] [time_to_sleep] [number_of_times_each_must_eat]
./philo 4 310 200 200
```

**Why test this:**
- Impossible: 310ms to die, but eating + sleeping = 400ms
- At least one philosopher MUST die

**What to check:**
- A philosopher should die
- Death should be reported accurately


### Test 5: Limited Meals
```
// [number_of_philosophers] [time_to_die] [time_to_eat] [time_to_sleep] [number_of_times_each_must_eat]
./philo 5 800 200 200 7
```

**Why test this:**
- Should stop when all philosophers eat 7 times
- Tests the optional 5th parameter
- Should NOT print "died"

**What to check:**
- Simulation should end cleanly
- Check that all philosophers ate 7 times
- No death message


### Test 6: Two Philosophers (Minimal Case)
```
// [number_of_philosophers] [time_to_die] [time_to_eat] [time_to_sleep] [number_of_times_each_must_eat]
./philo 2 800 200 200
```

**Why test this:**
- Simplest case with multiple philosophers
- Each has access to 2 forks alternately
- Should work smoothly

**What to check:**
- Both should eat alternately
- No deaths
- Clean synchronization

### Test 7: Large Number of Philosophers
```
// [number_of_philosophers] [time_to_die] [time_to_eat] [time_to_sleep] [number_of_times_each_must_eat]
./philo 200 800 200 200
```

**Why test this:**
- Stress test with many threads (200 threads!)
- Tests if your implementation scales
- More chance of race conditions appearing

**What to check:**
- No philosopher should die
- No data races
- Performance should remain acceptable

### Test 8: Data Race Detection
```
valgrind --tool=helgrind ./philo 5 800 200 200
```

**Why test this:**
- 800ms to die, 200+200=400ms cycle (comfortable timing so philosophers survive)

**What to check:**
- Look for "Possible data race" in Helgrind output
- Check for "Lock order" violations (potential deadlock)
- Should show `ERROR SUMMARY: 0 errors from 0 contexts` if clean

**Example of clean output:**
```
==12345== ERROR SUMMARY: 0 errors from 0 contexts
```

**Example of data race found:**
```
==12345== Possible data race during write of size 4
==12345==    at 0x401234: philosopher_routine (philo.c:42)
==12345==  This conflicts with a previous read of size 4
==12345==    at 0x401198: check_death (philo.c:67)
```

> Saving the output to file for easier analysis:
```
valgrind --tool=helgrind --log-file=helgrind.log ./philo 5 800 200 200
grep "Possible data race" helgrind.log
```

---

