#/bin/bash
# annooptim.sh - build a program with no compiler optimisations.


gcc -Wall -Wextra -g -O0 -D_GNU_SOURCE=1 -c src/Csv2anki/csv2anki.c
gcc -Wall -Wextra -g -O0 -D_GNU_SOURCE=1 -c src/Utils/fileops.c
gcc -Wall -Wextra -g -O0 -D_GNU_SOURCE=1 -c src/Csv2anki/gopt.c
gcc -Wall -Wextra -g -O0 -D_GNU_SOURCE=1 -c src/Utils/stringops.c
gcc csv2anki.o fileops.o gopt.o stringops.o -o csv2anki
rm *.o
