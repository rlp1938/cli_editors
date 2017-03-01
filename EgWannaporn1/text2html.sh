#!/bin/bash
#
# text2html.sh - script to convert a text file to html using hexsed.
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

# The target file has been generated from a pdf file using 'pdftotxt'.
# The pdf file in question contains Thai characters and also
# transliterated Thai using combining chars as tone marks. As output the
# text file contains a UTF-8 string 'e2 80 a8' string which is labeled
# text editor 'geany' as 'LS' which I guess means 'line separator'.
# There is also an object shown as 'FF' which is simply '0c' or ascii FF.

# LS is followed by a normal Unix line feed so the LS may be simply
# deleted as also the FF and I use 'hexsed' for this purpose.
# NB the LS string is not UTF-8 Thai, all Thai UTF-8 begins with 'e0'.

# Text file
fn=wannaporns-65-thai-phrases.txt
# Html file
hfn=wannaporns-65-thai-phrases.html
# Backup of text file - at the outset make this by hand.
bfn=wannaporns-65-thai-phrases.bak
# Start each execution on a pristine text file
cp "$bfn" "$fn"

# get rid of 'LS'.
hexsed /e280a8/d "$fn" > x
mv x "$fn"

# get rid of 'FF'.
hexsed /0c/d "$fn" > x
mv x "$fn"

# line feed pairs to become single.
hexsed /0a0a/0a/s "$fn" > x
mv x "$fn"

# the end point is to be a html file, and I want '<br />' at each eol.
hexsed /0a/3c6272202f3e0a/s "$fn" > x
mv x "$fn"

# initialise html file
cat <<EOT > "$hfn"
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8" />
<title>wanaporn</title>
</head>
<body>
EOT

cat "$fn" >> "$hfn"

cat <<EOT >> "$hfn"
</body>
</html>
EOT

# There are no <p> tags in the html, fix this.
# 1. the top of the list
tofind=`hexsed -s '\n1.'`
torepl=`hexsed -s '\n<p>'`"$tofind"
hexsed /"$tofind"/"$torepl"/s "$hfn" > x
mv x "$hfn"

# 2. almost the rest of them.
for i in `seq 2 65`
do
	tofind=`hexsed -s '\n'"$i"'.'`	# find only at start of line.
	torepl=`hexsed -s '\n</p>\n<p>'`"$tofind"
	hexsed /"$tofind"/"$torepl"/s "$hfn" > x
	mv x "$hfn"
done

# 3. the last.
tofind=`hexsed -s '\n</body>'`
torepl=`hexsed -s '\n</p>\n</body>'`
hexsed /"$tofind"/"$torepl"/s "$hfn" > x
mv x "$hfn"
