#!/bin/bash
#
# updcfg.sh - script to update configs during development
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

cfgdir=/home/bob/.config/csv2html/
mkdir -p "$cfgdir"
#cp csv2html.cfg "$cfgdir"
cp master.xml "$cfgdir"
cp master.css "$cfgdir"
cp master.html "$cfgdir"
cp master.css ./td1/css/
cp master.css ./td2/css/
cp master.css ./td3/css/
cp master.css ./td4/css/

