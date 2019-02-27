Dana Nygeun
Margaret Schmit

Compilation Notes:
Any file that intends to use the lock() and unlock() functions in our code should include the line:
#include "lock.h"
The lock.h file is in the main directory of this project.

Our tests are filed under the directories 'tests', 'basic_test_suite', and 'intense_tests'. We wrote these to thoroughly test our code.
As the name implies, the intense_tests are the most intricate and meant to break the code. As of our last running, our code passed all of these tests.

Project Notes:
Our biggest bug was that all of our threads were blocking each other, resulting in a deadlock situation where the signal handler would not be able to find an unblocked thread to jump to. This was caused because we had a sleep function in sem_wait, causing all of the threads to sleep forever, thus blocking all of the other threads. When we took this sleep function out, the threads did not all block each other. Additionally, we found that it was crucial to save the state of each thread in sem_wait and sem_post, so that they could run again. 
