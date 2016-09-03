
/*      csv2html.c
 *
 *  Copyright Robert L Parker 2016 rlp1938@gmail.com
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *  MA 02110-1301, USA.
*/

#include "fileops.h"
#include "stringops.h"
#include "gopt.h"
#include "firstrun.h"

static char *initctlfile(int optind, char **argv);
static void sanitycheck(int optind, char **argv);
static void initsetup(void);
static void initfilecheck(int optind, char **argv);
static int thisyear(void);
static char *getcfgfile(char *subdir, char *fn);
static void freelist(char **list);
static void freevlist(char *item1, ...);

int main(int argc, char **argv)
{
	options_t opts = process_options(argc, argv);
	if (checkfirstrun("csv2html")) initsetup();
	char *ctlfile = NULL;
	char *csvfile = NULL;
	if (opts.controlfile) {
		ctlfile = initctlfile(optind, argv);
		exit(EXIT_SUCCESS);
	}
	
	/* check files exist */
	sanitycheck(optind, argv);
	
	
	return 0;
} // main()

char *initctlfile(int optind, char **argv)
{	// control file must be named but must not exist.
	initfilecheck(optind, argv);
	char *fn = argv[optind];
	// gather the data I need to init the control file.
	int yy = thisyear();
	char **cfglines = readcfg("~/.config/csv2html/csv2html.cfg");
	char *author = getcfgvalue("author", cfglines);
	char *email = getcfgvalue("email", cfglines);
	char *pws = getcfgvalue("pagewidth", cfglines);
	int pw = strtol(pws, NULL, 10);
	char *phs = getcfgvalue("pageheight", cfglines);
	int ph = strtol(phs , NULL, 10);
	char *masterxml = getcfgfile("csv2html", "master.xml");
	fdata mydat = readfile(masterxml, 0, 1);
	free(masterxml);
	strdata strdat = getdatafromtagnames(mydat.from, mydat.to,
											"cformat");
	*strdat.to = 0;	// now it is a C string
	// now write that control file
	FILE * fpo = dofopen(fn, "w");
	fprintf(fpo, strdat.from, yy, author, email, pw, ph);
	dofclose(fpo);
	freevlist(phs, pws, email, author, NULL);
	free(mydat.from);
	freelist(cfglines);

	return fn;
}

void sanitycheck(int optind, char **argv)
{
	int ok = 1;
	ok = (ok && (argv[optind]));
	ok = (ok && (argv[optind+1]));
	ok = (ok && (!(fileexists(argv[optind]))));
	ok = (ok && (!(fileexists(argv[optind+1]))));
	if (!(ok)) {
		fprintf(stderr, "You must provide control file and csv file"
						" names, and both files must exist.\n");
		exit(EXIT_FAILURE);
	}
}

void initsetup(void)
{	// invokes firstrun() 
	firstrun("csv2html", "csv2html.cfg", "master.html", "master.xml",
				"master.css", NULL);
	fprintf(stdout, "Please edit %s/.config/csv2html/csv2html.cfg",
					getenv("HOME"));
} // initsetup()

void initfilecheck(int optind, char **argv)
{
	int ok = 1;
	ok = (ok && (argv[optind]));
	ok = (ok && (fileexists(argv[optind])));
	if (!(ok)) {
		fprintf(stderr, "You must provide a control file name.\n"
				"If you want to renew the file delete it first.\n");
		exit(EXIT_FAILURE);
	}
}

int thisyear(void)
{
	time_t now = time(NULL);
	struct tm *timedata = localtime(&now);
	return timedata->tm_year + 1900;
}

char *getcfgfile(char *subdir, char *fn)
{
	size_t plen = 0;
	plen += strlen(getenv("HOME"));
	plen += strlen(subdir);
	plen += strlen(".config/");
	plen += strlen(fn);
	plen += 4;	// I have no idea if some compoments end with '/'
	char *res = docalloc(plen, 1, "getcfgfile");
	strcpy(res, getenv("HOME"));
	if (res[strlen(res)-1] != '/') strcat(res, "/");
	strcat(res, ".config/");
	strcat(res, subdir);
	if (res[strlen(res)-1] != '/') strcat(res, "/");
	strcat(res, fn);
	return res;
}

void freelist(char **list)
{
	int i;
	for (i = 0; list[i]; i++) {
		free(list[i]);
	}
	free(list);
}

void freevlist(char *item1, ...)
{	/* every item in the list is expected to be char* */
	va_list ap;
	va_start(ap, item1);
	free(item1);	// inside the loop it starts on the item after
	while (1) {
		char *cp = va_arg(ap, char *);
		if (!cp) break;	// last arg must be NULL
		free(cp);
	}
	va_end(ap);
}
