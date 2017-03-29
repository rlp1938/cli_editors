
/*      hexsed.c - stream editor that uses hex strings
 *
 *	Copyright 2016 Bob Parker rlp1938@gmail.com
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *	MA 02110-1301, USA.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <stdint.h>
#include "../Utils/fileops.h"
#include "../Utils/stringops.h"
#include "gopt.h"

typedef struct sedex {
	int op;
	int flen;
	int rlen;
	int edcount;
	char *tofind;
	char *toreplace;
} sedex;

static char *eslookup(const char *tofind);
static sedex validate_expr(const char *expr);
static char *str2hex(const char *str);
static void editfile(const char *fn, sedex mysx, int quiet);
static int countdelimiters(char *line);
static int getoperator(char *buf, int delims);
static int getcount(char *buf);
static void getstrings(sedex *mysx, char *buf, int delims);
static char *checkhexstr(char *tocheck, char *typ);
static char *strcodepoint2hex(char *uinp);
static size_t getcodepoint(char *userinput);
static void upper(char *line);
static unsigned char *convert(char *userinput);
static size_t getbytecount(size_t codepoint);
static char *prlist(unsigned char *list);

int main(int argc, char **argv)
{
	options_t opts = process_options(argc, argv);
	// 3 opts vars are processed here, the other is dealt with in gopt.
	int quiet = opts.quiet;
	if (opts.line) {
		char *cp = str2hex(opts.line);
		fprintf(stdout, "%s\n", cp);
		free(cp);
		free(opts.line);
		exit(EXIT_SUCCESS);
	}
	if (opts.uinp) {
		char *cp = strcodepoint2hex(opts.uinp);
		fprintf(stdout, "%s\n", cp);
		free(cp);
		free(opts.uinp);
		exit(EXIT_SUCCESS);
	}
	if (opts.esc) {
		fprintf(stdout, "%s\n", eslookup(opts.esc));
		free(opts.esc);
		exit(EXIT_SUCCESS);
	}

	// now process the non-option arguments

	// 1.Check that argv[optind] exists.
	if (!(argv[optind])) {
		fprintf(stderr, "No expression provided\n");
		dohelp(1);
	}

	// 2. Check that it's meaningful, a valid expression.
	sedex mysx = validate_expr(argv[optind]);	// no return if error

	optind++;
	// 3.Check that argv[optind] exists.
	if (!(argv[optind])) {
		fprintf(stderr, "No file name provided\n");
		dohelp(1);
	}

	// 4. Check that it's meaningful, ie file exists.
	if (fileexists(argv[optind]) == -1) {
		fprintf(stderr, "No such file: %s\n", argv[optind]);
		dohelp(1);
	}
	// now do the edits
	char *edfile = argv[optind];
	editfile(edfile, mysx, quiet);
	return 0;
}//main()

char *eslookup(const char *tofind)
{
	static char result[3];
	const char *escseq = "abfnrtv\\'\"?";
	const char *esvalue = "07080C0A0D090B5C27223F";
	if (strlen(tofind) != 2) {
		fprintf(stderr, "Badly formed parameter: %s\n", tofind);
		exit(EXIT_FAILURE);
	}
	char *cp = strchr(escseq, tofind[1]);
	if (!cp) {
		fprintf(stderr, "Unknow escape sequence: %s\n", tofind);
		exit(EXIT_FAILURE);
	}
	int idx = (cp - escseq) * 2;
	strncpy(result, esvalue + idx, 2);
	result[2] = 0;
	return result;
} //

sedex validate_expr(const char *expr)
{	/*
	 * Fill in the sedex struct with proper values from expr or abort
	 * if anything in expr is malformed.
	*/
	char *buf;
	buf = dostrdup(expr);
	sedex mysx = {0};
	int delims = countdelimiters(buf);
	mysx.op = getoperator(buf, delims);
	mysx.edcount = getcount(buf);
	getstrings(&mysx, buf, delims);
	free(buf);
	return mysx;
} // validate_expr()

char *str2hex(const char *str)
{	/* For each byte in str, return the 2 byte hex code.
	 * Handle embedded escape sequences.
	 * */
	size_t len = strlen(str);
	char *buf = calloc(2 * len + 1, 1);
	if (!buf) {
		perror("Could not get memory in str2hex()");
		exit(EXIT_FAILURE);
	}
	size_t i, idx;	// need 2 indexes to handle escape sequences.
	for (idx=0, i=0; i < len; i++, idx++) {
		char res[3] = {0};
		if (str[i] == '\\') {
			strncpy(res, &str[i], 2);
			char *cp = eslookup(res);
			strncpy(res, cp, 2);
			i++;	// get past the escaped char
		} else {
			sprintf(res, "%X", str[i]);
		}
		strncpy(&buf[2*idx], res, 2);
	}
	return buf;
}

void editfile(const char *fn, sedex mysx, int quiet)
{
	int fcount = 0;
	fdata mydat = readfile(fn, 0, 0);
	char *cp = mydat.from;
	while (cp < mydat.to) {
		char *found = memmem(cp, mydat.to - cp, mysx.tofind, mysx.flen);
		int doit;
		// number of edits may be limited by count.
		doit = (found && (fcount < mysx.edcount));
		if (!doit) {
			// write out the rest of the file.
			fwrite(cp, 1, mydat.to - cp, stdout);
			cp = mydat.to;
		} else {
			fcount++;
			// write the file content up to the find string
			fwrite(cp, 1, found - cp, stdout);
			switch (mysx.op)
			{
				case 'a':	// append to find string
					fwrite(mysx.tofind, 1, mysx.rlen, stdout);
					fwrite(mysx.toreplace, 1, mysx.rlen, stdout);
					break;
				case 'i':	// insert before find string
					fwrite(mysx.toreplace, 1, mysx.rlen, stdout);
					fwrite(mysx.tofind, 1, mysx.rlen, stdout);
					break;
				case 'd':	// delete find string
					// do nothing
					break;
				case 'r':	// replace find string.
					fwrite(mysx.toreplace, 1, mysx.rlen, stdout);
					break;
			} // switch()
			cp = found + mysx.flen;
		}
	} // while()
	if (!quiet) {
		char *what = (mysx.op == 'd') ? "deletions" : "substitutions";
		fprintf(stdout, "Did %i %s.\n", fcount, what);
	}

	free(mydat.from);
	if (mysx.toreplace) free(mysx.toreplace);
	free(mysx.tofind);

} // editfile()

int countdelimiters(char *line)
{	/* count '/' in line. Abort with error unless there are 2 or 3 */
	int count = 0;
	char *cp = line;
	while ((cp=strchr(cp, '/'))) {
		count++;
		cp++;
	}
	if (count == 2 || count == 3) {
		return count;
	}
	fprintf(stderr, "Badly formed expression: %s\n", line);
	exit(EXIT_FAILURE);
} // countdelimiters()

int getoperator(char *buf, int delims)
{	/* get the command operator. Abort with error if it's not valid. */
	char *cp = strchr(buf, '/');	// I know that '/' does exist.
	cp --;	// look at char before '/'
	int oper = *cp;
	switch (oper)
	{
		case 'a':	// requires 3 delimiters
		case 'i':
		case 'r':
		if (delims != 3) {
			fprintf(stderr,
			"Operator %c requires find and replace strings:\n%s\n",
					oper, buf);
			exit(EXIT_FAILURE);
		}
			break;	// requires 2 delimiters
		case 'd':
		if (delims != 2) {
			fprintf(stderr,
			"Operator %c requires find string only:\n%s\n", oper, buf);
			exit(EXIT_FAILURE);
		}
			break;
		default:
		fprintf(stderr, "Illegal operator: %c\n", oper);
		exit(EXIT_FAILURE);
			break;
	}
	return oper;
} // getoperator()

int getcount(char *buf)
{	/* If there is a count, return it or set default value */
	char *line = dostrdup(buf);	// will butcher it.
	char *cp = strchr(line, '/');	// truncate line at '/'
	*cp = 0;
	int ret = INT_MAX;
	cp = strchr(line, '=');
	if (cp) {
		cp++;
		ret = strtol(cp, NULL, 10);
	}
	free(line);
	return ret;
} // getcount()

void getstrings(sedex *mysx, char *buf, int delims)
{	/* Return validated find string and replacement string if required
	 * into mysx.
	* */
	char *line = dostrdup(buf);
	char *begin = strchr(line, '/');
	*begin = 0;
	begin++;	// look past '/'
	char *end = strchr(begin, '/');
	*end = 0;
	mysx->tofind = checkhexstr(begin, "find");
	mysx->flen = strlen(mysx->tofind);
	if (delims == 3) {
		begin = end + 1;
		end = strchr(begin, '/');
		*end = 0;
		mysx->toreplace = checkhexstr(begin, "replace");
		mysx->rlen = strlen(mysx->toreplace);
	}
	free(line);
} // getstrings()

char *checkhexstr(char *tocheck, char *typ)
{	/* make sure that tocheck is valid hex and if so return ascii */
	size_t len = strlen(tocheck);
	if (len == 0) {
		fprintf(stderr, "%s string is zero length.\n", typ);
		exit(EXIT_FAILURE);
	}
	if (len %2 != 0) {
		fprintf(stderr, "%s hex input must be paired, 00, 01, 0f..."
					", %s\n", typ, tocheck);
		exit(EXIT_FAILURE);
	}
	char *teststr = dostrdup(tocheck);	// It's double the required size.
	memset(teststr, 0, len);
	size_t idx, i;
	for (idx = 0, i = 0; idx < len; idx+=2, i++) {
		char num[3] = {0};
		strncpy(num, &tocheck[idx], 2);
		int x = strtol(num, NULL, 16);
		if (x == 0x0 && strcmp("00", num) != 0) {
			fprintf(stderr, "Invalid hex chars at %lu in %s string.\n",
						idx, typ);
			exit(EXIT_FAILURE);
		}
		teststr[i] = x;
	}
	char *res = dostrdup(teststr);
	free(teststr);
	return res;
} // checkhexstr()

size_t getcodepoint(char *userinput)
{	/* Return codepoint as input by the user. Many formats allowed.*/
	size_t len = strlen(userinput);
	char *line = strdup(userinput);
	upper(line);
	size_t res;
	if (strncmp(line, "<U+", 3) == 0 && line[len-1] == '>') {
		// canonical expression of unicode
		res = strtol(line+3, NULL, 16);
	} else if (strncmp(line, "&#X", 3) == 0 && line[len-1] == ';') {
		// the next 2 are what might happen inside html
		line[1] = '0';
		res = strtol(line+1, NULL, 16);
	} else if (strncmp(line, "&#", 2) == 0 && line[len-1] == ';') {
		res = strtol(line+2, NULL, 10);
	} else {	// and this for a simple unadorned number;
		// nnnn for decimal, 0nnnn for octal, and 0xnnnn for hex.
		res = strtol(line, NULL, 0);
	}
	return res;
} // getcodepoint()

void upper(char *line)
{	/* convert any a..z to A..Z */
	char *cp = line;
	while (*cp) {
		*cp = toupper(*cp);
		cp++;
	}
} // upper()

unsigned char *convert(char *userinput)
{	/* Convert the user's unicode form into multi-byte hex */
	static unsigned char list[5] = {0};
	size_t codepoint = getcodepoint(userinput);
	size_t bytes = getbytecount(codepoint);
	if (bytes == 1) {
		list[0] = codepoint;
		return list;
	}
	unsigned char andmasks[5];
	andmasks[4] = strtol("00000111", NULL, 2);
	andmasks[3] = strtol("00001111", NULL, 2);
	andmasks[2] = strtol("00011111", NULL, 2);
	unsigned char ormasks[5];
	ormasks[4] = strtol("11110000", NULL, 2);
	ormasks[3] = strtol("11100000", NULL, 2);
	ormasks[2] = strtol("11000000", NULL, 2);
	unsigned char andmask = strtol("00111111", NULL, 2);
	unsigned char ormask = strtol("10000000", NULL, 2);
	size_t i;
	for (i = bytes-1; i > 0 ; i--) {
		list[i] = (codepoint & andmask) | ormask;
		codepoint >>= 6;	// shift 6 bits right;
	}
	list[0] = (codepoint & andmasks[bytes]) | ormasks[bytes];
	return list;
} // convert()

size_t getbytecount(size_t codepoint)
{
	size_t res;
	if (codepoint > 0x10ffff) {
		fprintf(stderr, "Codepoint is greater than utf maximum: %lu",
					codepoint);
		exit(EXIT_FAILURE);
	} else if (codepoint > 0xffff) {
		res = 4;
	} else if (codepoint > 0x7ff) {
		res = 3;
	} else if (codepoint > 0x7f) {
		res = 2;
	} else {
		res = 1;
	}
	return res;
} // getbytecount()

char *prlist(unsigned char *list)
{	/* convert list of chars to list of printable hex values */
	unsigned char *cp = list;
	size_t i = 0;
	while (*cp) {	// must be NULL terminated input
		i++;
		cp++;
	}
	char *outlist = calloc(2*i+1, 1);
	i = 0;
	size_t j=0;
	cp = list;
	while (list[i]) {
		sprintf(&outlist[j], "%.2x", list[i]);
		i++; j+=2;
	}
	return outlist;
} // prlist()

char *strcodepoint2hex(char *uinp)
{	/* uinp may be '<U+nnnn>'(nnnn hex), '&Xnnnn;'(nnnn hex),
	 *'&nnnn;' (nnnn dec), 0xnnnn (nnnn hex), 0nnnn (nnnn oct),
	 * or just nnnn (nnnn dec).
	*/
	unsigned char *list = convert(uinp);
	char *out = prlist(list);
	return out;
} // strcodepoint2hex()
