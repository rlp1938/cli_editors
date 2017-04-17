#!/bin/bash
#
# wp1addsound.sh - script to help link sound files to anki media
#
# Must be in dir ..../EgWannaporn1/ when this runs.
#
tofind=xxx
for z in `seq 1 65`
do
sed -n $i -e s/xxx/$i/ wp1.txt > x
mv x wp1.txt
done
