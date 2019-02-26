#!/bin/sh
echo " "
echo "TESTING BASIC LOCKING"
echo " "
./test_lock
echo " "
echo "TESTING BASIC JOIN"
echo " "
./test_join
echo " "
echo "TESTING SEM_INIT"
echo " "
./test_sem_init
