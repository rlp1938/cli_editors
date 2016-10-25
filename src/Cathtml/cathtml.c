
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
#include "src/Utils/fileops.h"
#include "src/Utils/stringops.h"

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
static fdata outbuf(char **argv);

int main(int argc, char **argv)
{
	if (argc < 2) {
		dohelp(EXIT_FAILURE);
	}
	if ((strcmp("-h", argv[1]) == 0) ||
		(strcmp("--help", argv[1]) == 0)){
		dohelp(EXIT_SUCCESS);
	}
	fdata outdat = outbuf(argv); // buffer to hold input files content.
	int av = 1;	// index to argv
	fdata mydat = readfile(argv[av], 0, 1);
	av++;
	char *eob1 = memmem(mydat.from, mydat.to - mydat.from, "</body>",
						strlen("</body>"));
	if(!eob1) die("Badly formed html file.\n");
	char *copyto = outdat.from;
	size_t sz = eob1 - mydat.from;
	// copies data from top of file to "</body>".
	memcpy(copyto, mydat.from, sz);
	copyto += sz;
	free(mydat.from);
	while (argv[av]) {
		fdata mydat = readfile(argv[av], 0, 1);
		// copies data from inside <body> ... </body> tags
		strdata strdat = getdatafromtagnames(mydat.from, mydat.to,
												"body");
		sz = strdat.to - strdat.from;
		memcpy(copyto, strdat.from, sz);
		copyto += sz;
		free(mydat.from);
		av++;
	}
	memcpy(copyto, "</body>\n", 8);
	copyto += 8;
	writefile("-", outdat.from, copyto, "w");
	free(outdat.from);
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

fdata outbuf(char **argv)
{	/* Calculate total file size for succession of files named in
	 * argv[].
	*/
	size_t tot = 0;
	int av = 1;
	while (argv[av]) {
		// yes this is utterly gross but easy for me.
		fdata mydat = readfile(argv[av], 0, 1);
		tot += mydat.to - mydat.from;
		free(mydat.from);
		// no matter I will read these files again anyway.
		av++;
	}
	fdata out;
	out.from = docalloc(tot, 1, "outbuf");
	out.to = out.from + tot;
	return out;
} // outbuf()
