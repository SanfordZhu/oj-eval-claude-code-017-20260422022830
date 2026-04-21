#!/bin/bash

echo "Testing train ticket system..."

# Test 1: Clean
echo "Test 1: clean"
echo "clean" | ./code

# Test 2: Add first user (should succeed with privilege 10)
echo -e "\nTest 2: add first user"
echo "add_user -c admin -u user1 -p pass123 -n 张三 -m a@b.com -g 5" | ./code

# Test 3: Login
echo -e "\nTest 3: login"
echo "login -u user1 -p pass123" | ./code

# Test 4: Query profile
echo -e "\nTest 4: query profile"
echo "query_profile -c user1 -u user1" | ./code

# Test 5: Add train
echo -e "\nTest 5: add train"
echo "add_train -i T1 -n 3 -m 100 -s 上海|南京|北京 -p 100|200 -x 08:00 -t 60|120 -o 5 -d 06-01|08-31 -y G" | ./code

# Test 6: Release train
echo -e "\nTest 6: release train"
echo "release_train -i T1" | ./code

# Test 7: Query train
echo -e "\nTest 7: query train"
echo "query_train -i T1 -d 07-01" | ./code

# Test 8: Exit
echo -e "\nTest 8: exit"
echo "exit" | ./code