### README.md
*This project has been created as part of the 42 curriculum by eanass*

______

### Description

**Codexion**  a multithreaded synchronization project inspired by the classic dining philosophers problem.

The simulation models a set of coders competing for shared USB dongles in order to compile their code.
Each coder is represented by a POSIX thread and must repeatedly follow this cycle:

take_dongles -> compile -> release_dongles -> debug -> refactor -> repeat

To compile, a coder must acquire two dongles at the same time: one on the left and one on the right.
Since dongles are shared resources, the main challenge is to manage synchronization correctly and avoid race conditions, deadlocks, starvation, and incorrect logging.

each coder has to get hold of 2 dongles, their left and right dongle, and perform a compiling cycle:
take_dongles > compile > release_dongles > debug > refactor > repeat * required_times

The core challenge is managing synchronization and avoiding race conditions while implementing two different scheduling algorithms: **First-In-First-Out (FIFO)** for standard queuing and **Earliest Deadline First (EDF)** for priority-based management.

### Instructions

##### Compilation

The project uses a standard Makefile. Run the following command in the root directory:
 
 ```bash
 make
 ```

To clean up files:

Removes object files.
```bash 
make clean
```


Removies object files and the executable.
```bash
make fclean
```


Performs a full rebuild
```bash
make re
```


##### Execution

The program takes 8 arguments:

```./codexion [number_of_coders] [time_to_burnout] [time_to_compile] [time_to_debug] [time_to_refactor] [required_compiles] [dongle_cooldown] [scheduler]```

scheduler: Must be ```fifo``` or ```edf```.

```bash
./codexion 5 800 200 200 200 10 50 edf
```



### Blocking cases handled

##### Deadlock Prevention

First before we get into how we prevent deadlock in our project, we need to understand what we mean by Deadlock.

**deadlock** is when two or more tasks never make progress because each is waiting for some resource held by another process.

There are four conditions which must hold for deadlock to occur as classically defined. These are known as Coffman's conditions

1- **Mutual exclusion**. there some resource that can't be shared between processes.

2- **Hold and wait**. Some process must be holding one resource while waiting for another.

3- **No preemption**. only the process holding a resource can release it.

4- **Circular wait**. there must be some cycle of waiting processes. each process must wait for a resource held by the next process in the cycle.



***Why Circulat Wait occurs in Codexion?***

In this project, circular waits occurs because of the resouce acquisition pattern


Imagine a scenario where you have $N$ coders and $N$ dongles arranged in a circle. 

Each coder is assigned:
Left Dongle: Index i
Right Dongle: Index (i + 1) % N

If every single coder simultaneously picks up their "left" dongle first, the following happens:

1- Coder 0 holds Dongle 0 and waits for Dongle 1.

2- Coder 1 holds Dongle 1 and waits for Dongle 2.

3- ...

At this point, every coder is holding one resource and is blocked waiting for the next one in the circle. No one can proceed, no one will release the resource they currently hold, and a complete cycle of dependency is formed. This is the definition of Circular Wait.

#####  Preventing deadlock

We break this cycle by enforce an asymmetric approach based on the Coder ID:

- Odd-numbered coders must pick up their Left dongle first, then their Right.

- Even-numbered coders must pick up their Right dongle first, then their Left.


When we flip the acquisition order for half the coders, we break the symmetry. this creates a conflict at the dongle.

Consider Dongle X (located between coder 1 and coder 2).
- coder 1 odd wants to pick it as their left.
- coder 2 even want to pick as their right.

Because both are trying acquire the same resource in different "roles" (as left vs as right), they cannot both hold their first resource and wait for the other indefinitely.

so in short:
We prevent circular wait by enforcing an asymmetric resource hierarchy. Coders are categorized into 'even' and 'odd' groups with distinct acquisition orders (Right-first vs. Left-first). This ensures that no circular dependency chain can be formed, as the resource contention is resolved at the individual dongle level rather than allowing a uniform global acquisition pattern


##### Starvation Prevention

By utilizing heap-based priority queuing for the EDF scheduler, we ensure that coders with urgent deadlines are prioritized, preventing indefinite blocking of specific threads.


##### Cooldown Handling

Dongles implement a cooldown period after use, preventing rapid-fire acquisition and ensuring the simulation reflects realistic hardware constraints.


##### Burnout Detection

The monitor thread continuously tracks the last_compile timestamp for every coder against time_to_burnout. If a thread exceeds this threshold, the simulation enters a stop state.

##### Log Serliazation

The monitor thread continuously tracks the last_compile timestamp for every coder against time_to_burnout. If a thread exceeds this threshold, the simulation enters a stop state.

### Thread synchronization mechanisms

Our implementation relies on standard POSIX threads (pthread) to coordinate state:

- ```pthread_mutex_t```:
    
    - ```dongle_mutex:``` Guards individual dongles, ensuring only one coder accesses a shared resource at a time.


    - ```print_mutex:``` Serializes access to printf to prevent garbled logs.

    - ```stop_mutex:``` Synchronizes access to the stop flag, allowing threads to check the termination state atomically.

    - ```sched_mutex:``` Portecs the scheduler's heap structure during heap_push and heap_pop operations

Race conditions are prevented by ensuring that any access to shared memory (the ```t_data``` struct or ```t_dongle``` queues) is preceded by a mutex lock. Communication between coders and the monitor is achieved through shared memory flags, protected by the ```stop_mutex```.


### Basic Test

Basic test I run to test my code:

1-  To test for any memory leak

```bash

valgrind ./codexion 4 400 50 50 50 3 50 edf
```

2- To test for Race Condition

```bash
valgrind --tool=helgrind ./codexion 4 400 50 50 50 3 50 edf
```

3- To test if all threads run their required_compiles

```bash
./codexion 4 400 50 50 50 3 50 edf | grep compil | wc -l
```

4- limit testing the program multiple times with bigger coder number and check if they run their required_compile 

```bash
for i in {1..10}; do valgrind ./codexion 300 500 50 50 50 1 50 edf | grep compil | wc -l; done
```


### Resources

[Unix Threads in C (Playlist)](https://youtube.com/playlist?list=PLfqABt5AS4FmuQf70psXrsMLEDQXNkLq2&si=5iUcoJWyEDH3TWx4)

[CPU Scheduling Basics (Playlist)](https://youtu.be/Jkmy2YLUbUY?si=Fho9G6wimwpsAxde)

[Concurrency_I](https://www.usna.edu/Users/cs/wcbrown/courses/IC221/classes/L14/Class.html)
[Concurrency_II](https://www.usna.edu/Users/cs/wcbrown/courses/IC221/classes/L15/Class.html)
[Concurrency_III](https://www.usna.edu/Users/cs/wcbrown/courses/IC221/classes/L18/Class.html)

### AI Usage Declaration

AI was used as a support tool to:

improve the structure of the README
clarify concurrency explanations
rewrite technical descriptions in a cleaner way
verify that the required sections were present
The final text was reviewed and understood before being used.