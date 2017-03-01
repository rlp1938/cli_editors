#!/bin/bash
#
# addsound.sh - script to add multimedia controls to a html file
# using hexsed as required.
#
# Copyright 2016 Robert L (Bob) Parker rlp1938@gmail.com
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.# See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
# MA 02110-1301, USA.
#

# The html file comprises paragraphs with lines inside the para split by
# <br />. The are: 1. Thai script, 2. Transliterated tone marked script,
# and 3. The meaning or meanings in English, a line each, And,
# occasionally explanatory English.
# The objective is to add one or more <audio controls> elements at the
# bottom of each paragraph. It's easy to do because each para is
# numbered in sequence at the beginning.

# Html file
hfn=wannaporns-65-thai-phrases.html
# Backup of html file. Make this manually once the html file is made.
bfn=wannaporns-65-thai-phrases.html.bak
# Start each execution on a pristine text file
cp "$bfn" "$hfn"

# Now add audio controls. I could do this in 1 hit maybe but I won't.
# Part 1
tofind=`hexsed -s '</p>'`
torepl=\
`hexsed -s '<audio controls>\n\t<source src=xxx />\n</audio><br />\n</p>'`
hexsed /"$tofind"/"$torepl"/s "$hfn" > x
mv x "$hfn"

# Part 2
tofind=`hexsed -s 'xxx'`
torepl=`hexsed -s '"porn-audio/porn-xxx.mp3" type="audio/mpeg"'`
hexsed /"$tofind"/"$torepl"/s "$hfn" > x
mv x "$hfn"

# Now replace the 'xxx' in the audio source
tofind=787878	# xxx
for i in `seq 1 65`
do
	torepl=`hexsed -s "$i"`
	# prefixing the find expression with '=1' means only the first
	# instance will be replaced.
	hexsed =1/"$tofind"/"$torepl"/s "$hfn" > x
	mv x "$hfn"
done

# Some sound files exist as xxxa.mp3 and xxxb.mp3 so for these I must
# repeat the audio controls and put on the a|b suffixes.
# 1. repeat those controls
begin=`hexsed -s '<audio controls>\n\t<source src="porn-audio/porn-'`
end=`hexsed -s '.mp3" type="audio/mpeg" />\n</audio><br />'`
#echo end "$end"
for i in 33 34 38 39 40 41 42 43 44 45
do
	middle=`hexsed -s "$i"`
	tofind="$begin""$middle""$end"
#	echo tofind $tofind
	torepl="$tofind"0A"$tofind"0A
#	echo torepl $torepl
	hexsed /"$tofind"/"$torepl"/s "$hfn" > x
	mv x "$hfn"
done

# 2. Put the suffixes a|b after the mp3 base names.
for i in 33 34 38 39 40 41 42 43 44 45
do

	tofind=`hexsed -s "$i".mp3`
	torepl=`hexsed -s "$i"a.mp3`
	hexsed =1/"$tofind"/"$torepl"/s "$hfn" > x
	mv x "$hfn"
	torepl=`hexsed -s "$i"b.mp3`
	hexsed =1/"$tofind"/"$torepl"/s "$hfn" > x
	mv x "$hfn"
done

# get rid of doubled line feeds
hexsed /0A0A/0A/s "$hfn" > x
mv x "$hfn"
