/*    csv2anki.c
 *
 * Copyright 2017 Robert L (Bob) Parker rlp1938@gmail.com
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
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/stat.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include <limits.h>
#include <libgen.h>

// delete below as not needed

#include "../Utils/fileops.h"
#include "../Utils/stringops.h"
#include "gopt.h"

enum typ{DR, FIL, STR, OTHR};
static char *validatearg(char *arg, char *text, int typ);
static int countfields(char *line);
static void freeup(void *one, ...);
static void freelist(char **list);
static int *list2intarray(char *list);
static char **list2strarray(char *list);
static void editcsv(fdata csvdat, int *fldlist, char **seplist,
					int fieldcount);
static void printcsv(char *line, int *fldlist, char **seplist, int cls);
static char **lexcsv(char *line, int cols);
static char **stripqt(char **fields, int cols);

int main(int argc, char **argv)
{
	options_t opts = process_options(argc, argv);

	// now process the non-option arguments
	char *fieldlist = validatearg(argv[1], "field list", STR);
	char *seplist = validatearg(argv[2], "separator list", STR);
	char *csvfile = validatearg(argv[3], "csv file", FIL);
	fdata csvdat = readfile(csvfile, 0, 1);
	int lines;
	(void)mem2str_n(csvdat.from, csvdat.to, &lines);
	if (countfields(fieldlist) - countfields(seplist) != 1) {
		fprintf(stderr, "There must be just 1 less item in"
						" separator list than in field list.\n");
		exit(EXIT_FAILURE);
	}
	int fieldcount = countfields(csvdat.from);
	int *outfieldnum = list2intarray(fieldlist);
	char **outlist = list2strarray(seplist);	// "<br />" is possible.
	// TODO check fieldlist that it does not index a field out of range.
	editcsv(csvdat, outfieldnum, outlist, fieldcount);
	freelist(outlist);
	freeup(outfieldnum, csvdat.from, csvfile, seplist, fieldlist, NULL);
	return 0;
}//main()

char *validatearg(char *arg, char *text, int typ)
{	/* add code as needed */
	if (!arg) {
		fprintf(stderr, "No %s provided.\n", text);
		exit(EXIT_FAILURE);
	}
	switch (typ)
	{
		case DR:
			if (direxists(arg) == -1) {
				perror(arg);
				exit(EXIT_FAILURE);
			}
			break;
		case FIL:
			if (fileexists(arg) == -1) {
				perror(arg);
				exit(EXIT_FAILURE);
			}
			break;
		case STR:
			if (strlen(arg) == 0) {
				fprintf(stderr, "Zero length string provided.\n");
				exit(EXIT_FAILURE);
			}
			break;
		case OTHR:
			// Custom code here
			break;
	} // switch(typ)
	return dostrdup(arg);
} // validatearg()

int countfields(char *line)
{	/* count the number of comma separated fields in a string */
	int count = 0;
	char *cp = line;
	char *ep = cp + strlen(line);
	int inqt = 0;
	while (cp < ep) {
		switch (*cp)
		{
			case '"':
				if (inqt) {
					inqt = 0;
				} else {
					inqt = 1;
				}
				break;
			case ',':
				if (!inqt) {
					count++;
				}
				break;
		}
		cp++;
	}
	return count+1;	// 1 field more than separators.
} // countfields()

void freeup(void *one, ...)
{	/* free lots of stuff */
	va_list ap;
	void *vp;
	va_start(ap, one);
	while (1) {
		vp = va_arg(ap, void *);	// last arg MUST be NULL
		if(!vp) break;
		free(vp);
	}
	va_end(ap);
} // freeup()

void freelist(char **list)
{
	int i = 0;
	while (list[i]) {
		free(list[i]);
		i++;
	}
	free(list);
} // freelist()

int *list2intarray(char *list)
{	/* field numbers are always comma separated */
	int sz = countfields(list);
	int *res = docalloc((sz + 1), sizeof(int), "list2intarray()");
	res[sz] = -1;
	char *buf = dostrdup(list);	// will butcher the copy
	char *cp = buf;
	int idx = 0;
	while (1) {
		char *comma = strchr(cp, ',');
		if (!comma) {
			res[idx] = strtol(cp, NULL, 10);
			break;
		}
		*comma = 0;
		res[idx] = strtol(cp, NULL, 10);
		idx++;
		cp = comma + 1;
	}
	free(buf);
	return res;
} // list2intarray()

char **list2strarray(char *list)
{	/* field separators are always comma separated */
	int sz = countfields(list);
	char **res = docalloc((sz + 1), sizeof(char *), "list2strarray()");
	char *buf = dostrdup(list);	// will butcher the copy
	char *cp = buf;
	int idx = 0;
	while (1) {
		char *comma = strchr(cp, ',');
		if (!comma) {
			res[idx] = dostrdup(cp);
			break;
		}
		*comma = 0;
		res[idx] = dostrdup(cp);
		idx++;
		cp = comma + 1;
	}
	free(buf);
	return res;
} // list2strarray()

void editcsv(fdata csvdat, int *fldlist, char **seplist, int fieldcount)
{	/* Count fields in each row and report missmatches. Write out the
	 * rows that have matching field counts.
	*/
	char *line = csvdat.from;
	int rowcount = 0;
	while (line < csvdat.to) {
		int colcount = countfields(line);
		if (colcount != fieldcount) {	// report only, not fatal.
			fprintf(stderr,
			"Row %d has %d columns but requires %d columns.\n",
				rowcount+1, colcount, fieldcount);
		} else {	// print out fields under control of lists.
			char *buf = dostrdup(line);
			printcsv(buf, fldlist, seplist, colcount);
			free(buf);
		}
		rowcount++;
		line += strlen(line) + 1;
	}
} // editcsv()

void printcsv(char *line, int *fldlist, char **seplist, int cls)
{	/* Extract the columns and print them as required by fldlst */
	char **fields = lexcsv(line, cls);
	int i = 0;
	while (1) {
		fputs(fields[fldlist[i] -1], stdout);
		i++; if (fldlist[i] == -1) break;
		fputs(seplist[i - 1], stdout);
	}
	fputs("\n", stdout);
	free(fields);
} // printcsv()

char **lexcsv(char *line, int cols)
{	/* Return array of char * comprising CSV fields in line.
	 * Take "" wrapping off fields if it exists.
	 * Line is a copy, so it is safe to alter it.
	*/
	char **fields = docalloc(cols+1, sizeof(char *), "printcsv()");
	char *cp = line;
	char *ep = line + strlen(line);
	int inqt = 0;
	while (cp < ep) {
		switch (*cp) {
			case '"':
				if (inqt) { inqt = 0; } else { inqt = 1; } break;
			case ',':
				if (!inqt) { *cp = 0; } break;
		} // switch(*cp)
		cp++;
	} // while()
	cp = line;
	int i;
	for (i = 0; i < cols; i++) {
		fields[i] = cp;
		cp += strlen(cp) + 1;
	}
	return stripqt(fields, cols);
} // lexcsv()

char **stripqt(char **fields, int cols)
{	/* Strip quote marks off each field if they exist. */
	int i = 0;
	for (i = 0; i < cols; i++) {
		char *cp = fields[i];
		if(*cp == '"') {
			*cp = 0; cp++;
			char *ep = strrchr(cp, '"');
			if (ep) *ep = 0;
			fields[i] = cp;
		} // if()
	} // for(i...)
	return fields;
} // stripqt()
