
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

typedef struct htdata {
	char *htfn;
	char *title;
	char *yy;
	char *author;
	char *email;
	char *pw;
	char *ph;
	char *rowsperpage;
	char *pagerowslist;
	char *divpagetop_break;
	char *divpagetop;
	char *divrecto;
	char *divverso;
	char *thead;
	char *trowodd;
	char *troweven;
	char *tfoot;
} htdata;


static char *initctlfile(int optind, char **argv);
static void sanitycheck(int optind, char **argv);
static void initsetup(void);
static void initfilecheck(int optind, char **argv);
static int thisyear(void);
static char *getcfgfile(char *subdir, char *fn);
static void freelist(char **list);
static void freevlist(char *item1, ...);
static htdata *makehtdata(const char *fn);
static void destroyhtdata(htdata *htd);
static void isspace2spch(char *in);
static char *tagstr(char *from, char *to, char *tagname);
static char *tagstrnull(char *from, char *to, char *tagname);
static char *multisp2single(char *in, char *out);
static void comment2space(char *from, char *to);
static void titlepage(htdata *htd);
static void htmlend(const char *fn);

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
	ctlfile = argv[optind];
	csvfile = argv[optind+1];
	htdata *htd = makehtdata(ctlfile);
	titlepage(htd);
	// tables
	htmlend(htd->htfn);
	//destroyhtdata(htd);
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

htdata *makehtdata(const char *fn)
{
	fdata mydat = readfile( fn, 1, 1);
	*(mydat.to-1) = 0;	// now a C string
	comment2space(mydat.from, mydat.to);
	isspace2spch(mydat.from);
	multisp2single(mydat.from, mydat.from);
	htdata *htd = docalloc(sizeof(htdata), 1, "mkhtdata");
	htd->author = tagstr(mydat.from, mydat.to, "author");
	htd->divpagetop = "<div>";	// yes weird
	htd->divpagetop_break = 
	"<div style=\"page-break-before: always\">";
	htd->divrecto = "<div class=\"recto\">";
	htd->divverso = "<div class=\"verso\">";
	htd->email = tagstr(mydat.from, mydat.to, "email");
	htd->htfn = tagstr(mydat.from, mydat.to, "htmlfn");
	htd->pagerowslist = tagstr(mydat.from, mydat.to, "rowlist");
	// the 2 below are number+"mm" as string. Convert when calcs needed.
	htd->ph = tagstr(mydat.from, mydat.to, "height");
	htd->pw = tagstr(mydat.from, mydat.to, "width");
	htd->rowsperpage = tagstr(mydat.from, mydat.to, "rowsperpage");
	// following 2 are allowed to be absent
	htd->tfoot = tagstrnull(mydat.from, mydat.to, "tfoot");
	htd->thead = tagstrnull(mydat.from, mydat.to, "thead");
	htd->title = tagstr(mydat.from, mydat.to, "title");
	htd->troweven = "<tr class=\"even\">";
	htd->trowodd = "<tr class=\"odd\">";
	htd->yy = tagstr(mydat.from, mydat.to, "year");
	free(mydat.from);
	return htd;
}

void destroyhtdata(htdata *htd)
{	// can't use freevlist 'cos it will quit at first NULL element found
	if (htd->htfn) free(htd->htfn);
	if (htd->title) free(htd->title);
	if (htd->author) free(htd->author);
	if (htd->email) free(htd->email);
	if (htd->pw) free(htd->pw);
	if (htd->ph) free(htd->ph);
	if (htd->title) free(htd->title);
	if (htd->rowsperpage) free(htd->rowsperpage);
	if (htd->pagerowslist) free(htd->pagerowslist);
	if (htd->divpagetop_break) free(htd->divpagetop_break);
	if (htd->divpagetop) free(htd->divpagetop);
	if (htd->divrecto) free(htd->divrecto);
	if (htd->divverso) free(htd->divverso);
	if (htd->thead) free(htd->thead);
	if (htd->trowodd) free(htd->trowodd);
	if (htd->troweven) free(htd->troweven);
	if (htd->tfoot) free(htd->tfoot);
	if (htd->yy) free(htd->yy);
	free(htd);
}

void isspace2spch(char *in)
{	// everything isspace() becomes ' '
	char *cp = in;
	while (*cp) {
		if (isspace(*cp)) *cp = ' ';
		cp++;
	}
}

char *multisp2single(char *in, char *out)
{
	char *buf = docalloc(strlen(in), 1, "multisp2single");
	char *cp = in;
	char *bp = buf;
	int inspace = 0;
	while (*cp) {
		switch (*cp)
		{
			case ' ':
				if (!inspace) {	// copy single space.
					*bp = *cp;
					bp++;
				}
				inspace = 1;
				break;
			default:
				*bp = *cp;
				bp++;
				inspace = 0;
				break;
		}
		cp++;
	}
	// There may still be leading and/or trailing space
	cp = buf;
	char *ep = buf + strlen(buf) - 1;
	while (ep > cp && *ep == ' ') ep--;
	*ep = 0;
	while (cp < ep && *cp == ' ') cp++;
	// NB reason why the caller can't use same buffer for in/out,
	// out can only be smaller than in or the same.
	strcpy(out, cp);
	return out;
}

char *tagstr(char *from, char *to, char *tagname)
{
	strdata strdat = getdatafromtagnames(from, to, tagname);
	// there may be ' ' in front and/or back of everything.
	char *cp = strdat.from;
	char *ep = strdat.to - 1;
	while (*cp == ' ') cp++;
	while (*ep == ' ') ep--;
	ep++;
	/* Deal with aberrant conditions:
	 * <tag></tag>				returns NULL
	 * <tag>       </tag>		returns NULL
	 * <tag>  some_text  </tag>	returns "some_text"
	*/
	int buflen = ep - cp;	// must deal with cp >= ep
	if (buflen < 1) return NULL;
	char *buf = docalloc(buflen + 1, 1, "tagstr");
	strncpy(buf, cp, buflen);
	buf[buflen] = 0;
	return buf;
}

char *tagstrnull(char *from, char *to, char *tagname)
{	// in this case an absent tag may be a choice, not a fatal error
	char tagfro[NAME_MAX];
	sprintf(tagfro, "<%s>", tagname);
	char *cp = memmem(from, to - from, tagfro, strlen(tagfro));
	if (!cp) return NULL;
	// else
	return tagstr(from, to, tagname);
}

void comment2space(char *from, char *to)
{	// comment_text_to_space() seems to be buggy - this is a rewrite
	char *begin, *end;
	begin = from;
	while (1) {
		begin = memmem(begin, to - from, "/*", 2);
		if(!begin) break;
		end = memmem(begin, to - begin, "*/", 2);
		if (!end) {
			fprintf(stderr, "No terminator:\n%.70s\n", begin);
			exit(EXIT_FAILURE);
		}
		memset(begin, ' ', end - begin +2);
		begin = end;
	}
	begin = from;
	while (1) {
		begin = memmem(begin, to - from, "//", 2);
		if(!begin) break;
		end = memchr(begin, '\n', to - begin);
		if (!end) {
			fprintf(stderr, "No terminator:\n%.70s\n", begin);
			exit(EXIT_FAILURE);
		}
		memset(begin, ' ', end - begin);
		begin = end;
	}	
}

void titlepage(htdata *htd)
{	// Writes the HTML title page
	char *hfn = getcfgfile("csv2html", "master.html");
	fdata mydat = readfile(hfn, 0, 1);
	strdata strdat = getdatafromtagnames(mydat.from, mydat.to,
											"cformat");
	*strdat.to = 0;	// C string now
	int margintop = strtol(htd->ph, NULL, 10) / 2 - 25;	// units mm
	FILE *fpo = dofopen(htd->htfn, "w");
	fprintf(fpo, strdat.from, htd->htfn, htd->yy, htd->author,
				htd->email, htd->author, htd->title, margintop,
				htd->title);
	dofclose(fpo);
	free(mydat.from);
}

void htmlend(const char *fn)
{
	writefile(fn, "</body>\n</html>\n", NULL, "a");
}
