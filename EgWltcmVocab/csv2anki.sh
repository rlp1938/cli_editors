#!/bin/bash
#
# csv2anki.sh - script to convert a csv file to anki text input
#
# Copyright 2017 Robert L (Bob) Parker rlp1938@gmail.com
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

# this relies on all quote protected commas being replaced by '123xyz'
# in geany.
cp anki.csv.bak anki.csv

sed s/','/';'/ anki.csv > x

# should do only the first comma

mv x anki.csv
sed s/','/'  '/ anki.csv > x
# second comma

mv x anki.csv

sed s/123xyz/','/g anki.csv > x
# put quote protected , back
mv x anki.csv
