
/*     cathtml.c
 *
 * Copyright 2016 Robert L (Bob) Parker rlp1938@gmail.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
*/


#include <stdio.h>
#include "fileops.h"
#include "stringops.h"

char *helptext =
  "\tcathtml concatenates a number of html files into one.\n"
  "\tIt writes the concatenated result to stdout.\n"
  "\n"
  "\tSYNOPSIS\n"
  "\tcathtml htmlfile1 htmlfile2 ... > outputhtmlfile\n"
  "\n"
  "\tEverything from htmlfile1 up to and including the <body ...> tag\n"
  "\twill be output to the final result.\n"
  "\n"
  "\tOPTIONS\n"
  "\t-h, --help writes this help. There are no other options.\n"
  ;
static void dohelp(int forced);
static void die(const char *msg);

int main(int argc, char **argv)
{
	if (argc < 2) {
		dohelp(EXIT_FAILURE);
	}
	if ((strcmp("-h", argv[1]) == 0) ||
		(strcmp("--help", argv[1]) == 0)){
		dohelp(EXIT_SUCCESS);
	}
	int av = 1;	// index to argv
	fdata mydat = readfile(argv[av], 0, 1);
	char *eob1 = memmem(mydat.from, mydat.to - mydat.from, "</body>",
						strlen("</body>"));
	if(!eob1) die("Badly formed html file.\n");
	writefile("-", mydat.from, eob1, "w");
	free(mydat.from);
	return 0;
}

void dohelp(int forced){
	fputs(helptext, stderr);
	exit(forced);
}

void die(const char *msg)
{
	fputs(msg, stderr);
	exit(EXIT_FAILURE);
}
