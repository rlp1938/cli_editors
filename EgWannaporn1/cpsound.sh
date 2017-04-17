#!/bin/bash
#
# cpsound.sh - script to link sound files to anki media
#
# Must be in dir ..../EgWannaporn1/ when this runs.
#
if [[ -d tmp ]]
then
	rm -rf tmp
	sync
fi

mkdir -p tmp/
# there are files named porn-<number>.mp3, and ...<number>a|b.mp3
for i in `seq 1 65`
do
	fr=porn-1/porn-"$i".mp3
	to=tmp/wp1-"$i".mp3
	if [[ -f "$fr" ]]
	then
		ln "$fr" "$to"
	fi
	fr=porn-1/porn-"$i"a.mp3
	to=tmp/wp1-"$i"a.mp3
	if [[ -f "$fr" ]]
	then
		ln "$fr" "$to"
	fi
	fr=porn-1/porn-"$i"b.mp3
	to=tmp/wp1-"$i"b.mp3
	if [[ -f "$fr" ]]
	then
		ln "$fr" "$to"
	fi
done
