#!/bin/bash
# nooptim.sh - build a program with no compiler optimisations.

gcc -Wall -Wextra -g -O0 -D_GNU_SOURCE=1 -c src/Hexsed/hexsed.c
gcc -Wall -Wextra -g -O0 -D_GNU_SOURCE=1 -c src/Utils/fileops.c
gcc -Wall -Wextra -g -O0 -D_GNU_SOURCE=1 -c src/Hexsed/gopt.c
gcc -Wall -Wextra -g -O0 -D_GNU_SOURCE=1 -c src/Utils/stringops.c

gcc -g hexsed.o fileops.o gopt.o stringops.o -o hexsed
rm *.o
