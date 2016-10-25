
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

#include "../Utils/fileops.h"
#include "../Utils/stringops.h"
#include "../Utils/firstrun.h"
#include "gopt.h"

typedef struct ts {
	char *opn;
	char *cls;
} ts;

typedef struct csvdata {
	int rowsthistable;
	int curtablerow;
	int rowsinfile;
	int curfilerow;
	int tabs;
	int pageno;
	int isdone;
	char *tabsx10;
	char *dataline;
	char *dataend;
} csvdata;

typedef struct tagstruct {
	char *tagopn;
	char *data;
	char *tagcls;
} tagstruct;

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
	char *tablespec;
	char *trowodd;
	char *troweven;
	char *divpagetop;
	char *divrecto;
	char *divverso;
	char *divtrecto;
	char *divtverso;
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
static void whitespace2spacechar(char *in);
static char *tagstr(char *from, char *to, char *tagname);
static char *multiplespace2single(char *in, char *out);
static void comment2space(char *from, char *to);
static void titlepage(htdata *htd, int stoptp);
static void htmlend(const char *fn);
static void generatetables(csvdata *csvd, htdata *htd,
							const char *csvfile);
static int dat2cstr(char *from, char *to);
static void writenewpage(csvdata *csvd, htdata *htd, FILE *fpo);
static int getrows(char *list);
static void tagout(csvdata *csvd, char *tag, FILE *fpo);
static void writefooter(csvdata *csvd, htdata *htd, char *htags,
						FILE *fpo);
static void textout(csvdata *csvd, char *text, FILE *fpo);
static void	checkhavecss(void);
static void writetable(csvdata *csvd, htdata *htd, FILE* fpo);
static tagstruct *maketagstruct(void);
static void destroytagstruct(tagstruct *tstr);
static tagstruct *gettaggroup(char *taglist, char *tagname, int fatal);
static char *data2str(char *fr, char *to);
static void writeheader(csvdata *csvd, htdata *htd, char *htags,
							FILE *fpo);
static void writebody(csvdata *csvd, htdata *htd, char *htags,
						FILE *fpo);
static void writerow(csvdata *csvd, htdata *htd, char *htags,
						FILE *fpo, char *thtd);
static void writefixedtd(csvdata *csvd, char *htags, FILE *fpo,
							char *thtd);
static char *insertclass(char *tag, char *class);
static csvdata *makecsvdata(void);
static void destroycsvdata(csvdata *csvd);
static void writedatarow(csvdata *csvd, htdata *htd, char *htags,
							FILE *fpo);
static char *getnextdatarow(csvdata *csvd);
static char **maketaglist(char *htags, char *tagname, char **closer);
static void destroytaglist(char **taglist, char *closer);
static char *linepart(char *line, int segment);
static void writedatadetail(csvdata *csvd, char *htags, FILE *fpo);
static void makeblank(htdata *htd);

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
	checkhavecss();
	htdata *htd = makehtdata(ctlfile);
	csvdata *csvd = makecsvdata();
	titlepage(htd, opts.notitle);
	if (opts.blankpage) {
		makeblank(htd);
	}
	generatetables(csvd, htd, csvfile);
	htmlend(htd->htfn);
	destroycsvdata(csvd);
	destroyhtdata(htd);
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
	char *c2hmasterxml = getcfgfile("htmledit", "c2hmaster.xml");
	fdata mydat = readfile(c2hmasterxml, 0, 1);
	free(c2hmasterxml);
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
	firstrun("htmledit", "csv2html.cfg", "c2hmaster.html", "c2hmaster.xml",
				"c2hmaster.css", NULL);
	fprintf(stdout, "Please edit %s/.config/htmledit/csv2html.cfg",
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
	fdata mydat = readfile(fn, 1, 1);
	*(mydat.to) = 0;	// now a C string
	comment2space(mydat.from, mydat.to);
	whitespace2spacechar(mydat.from);
	multiplespace2single(mydat.from, mydat.from);
	htdata *htd = docalloc(sizeof(htdata), 1, "mkhtdata");
	htd->author = tagstr(mydat.from, mydat.to, "author");
	tagstruct *tsx = gettaggroup(mydat.from, "tabdata", 1);
	htd->tablespec = dostrdup(tsx->data);
	destroytagstruct(tsx);
	htd->divpagetop = dostrdup(
	"<div style=\"page-break-before: always\">");
	htd->divrecto = dostrdup("<div class=\"recto\"");
	htd->divverso = dostrdup("<div class=\"verso\"");
	htd->email = tagstr(mydat.from, mydat.to, "email");
	htd->htfn = tagstr(mydat.from, mydat.to, "htmlfn");
	htd->pagerowslist = tagstr(mydat.from, mydat.to, "rowlist");
	// the 2 below are number+"mm" as string. Convert when calcs needed.
	htd->ph = tagstr(mydat.from, mydat.to, "height");
	htd->pw = tagstr(mydat.from, mydat.to, "width");
	htd->rowsperpage = tagstr(mydat.from, mydat.to, "rowsperpage");
	// following 2 are allowed to be absent
	htd->title = tagstr(mydat.from, mydat.to, "title");
	htd->troweven = dostrdup("<tr class=\"even\"");
	htd->trowodd = dostrdup("<tr class=\"odd\"");
	htd->yy = tagstr(mydat.from, mydat.to, "year");
	htd->divtrecto = dostrdup("<div class=\"trecto\">");
	htd->divtverso = dostrdup("<div class=\"tverso\">");
	free(mydat.from);
	return htd;
}

void destroyhtdata(htdata *htd)
{	// can't use freevlist 'cos it will quit at first NULL element found
	if (htd->tablespec) free(htd->tablespec);
	if (htd->htfn) free(htd->htfn);
	if (htd->title) free(htd->title);
	if (htd->author) free(htd->author);
	if (htd->email) free(htd->email);
	if (htd->pw) free(htd->pw);
	if (htd->ph) free(htd->ph);
	if (htd->rowsperpage) free(htd->rowsperpage);
	if (htd->pagerowslist) free(htd->pagerowslist);
	if (htd->divpagetop) free(htd->divpagetop);
	if (htd->divrecto) free(htd->divrecto);
	if (htd->divverso) free(htd->divverso);
	if (htd->trowodd) free(htd->trowodd);
	if (htd->troweven) free(htd->troweven);
	if (htd->yy) free(htd->yy);
	if (htd->divtrecto) free(htd->divtrecto);
	if (htd->divtverso) free(htd->divtverso);
	free(htd);
}

void whitespace2spacechar(char *in)
{	// everything isspace() becomes ' '
	char *cp = in;
	while (*cp) {
		if (isspace(*cp)) *cp = ' ';
		cp++;
	}
}

char *multiplespace2single(char *in, char *out)
{
	char *buf = docalloc(strlen(in), 1, "multiplespace2single");
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
	ep++;	// return to last space char
	*ep = 0;
	while (cp < ep && *cp == ' ') cp++;
	// NB there is no reason why the caller can't use same buffer for
	// in/out, out can only be smaller than in or the same.
	strcpy(out, cp);
	free(buf);
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

void titlepage(htdata *htd, int stoptp)
{	// Writes the HTML title page
	char *hfn = getcfgfile("htmledit", "c2hmaster.html");
	fdata mydat = readfile(hfn, 0, 1);
	free(hfn);
	strdata strdat = getdatafromtagnames(mydat.from, mydat.to,
											"cformat");
	*strdat.to = 0;	// C string now
	int margintop = strtol(htd->ph, NULL, 10) / 2 - 25;	// units mm
	FILE *fpo = dofopen(htd->htfn, "w");
	fprintf(fpo, strdat.from, htd->htfn, htd->yy, htd->author,
				htd->email, htd->author, htd->title, margintop);
	if (!stoptp) {
		fprintf(fpo,
		"\t<h1 style=\"margin-top %dmm; text-align: center\">%s</h1>\n",
		margintop, htd->title);
	}

	dofclose(fpo);
	free(mydat.from);
}

void htmlend(const char *fn)
{
	writefile(fn, "</body>\n</html>\n", NULL, "a");
}

int dat2cstr(char *from, char *to)
{
	int rc = 0;
	char *cp = from;
	while (cp < to) {
		char *eol = memchr(from, '\n', to - from);
		rc++;
		*eol = 0;
		cp = eol + 1;
	}
	return rc;
}

int getrows(char *list)
{	/* Avoid clobbering list */
	static int offset = 0;

	char *cp = list + offset;
	int ret = strtol(cp, NULL, 10);
	if (ret) {	// calculate offset to next number
		char *guard = list + strlen(list);
		while(!isdigit(*cp) && cp < guard) cp++;
		while(isdigit(*cp) && cp < guard) cp++;	// past the last number
		while(!isdigit(*cp) && cp < guard) cp++;
	}
	offset = cp - list;
	return ret;
}

void tagout(csvdata *csvd, char *tag, FILE *fpo)
{
	int opener, closer;
	if (strncmp(tag, "</", 2) == 0) {
		closer = 1;
		opener = 0;
	} else {
		closer = 0;
		opener = 1;
	}

	char *buf = docalloc(strlen(tag) + 32, 1, "tagout");
	char tablist[16];
	strcpy(tablist, csvd->tabsx10);
	if (opener) csvd->tabs++;
	tablist[csvd->tabs] = 0;
	sprintf(buf, "%s%s>\n", tablist, tag);
	char *cp = strstr(buf, ">>");
	if (cp) {	// deal with tags that were never open ended.
		*cp = 0;
		strcat(buf, ">\n");
	}
	fputs(buf, fpo);
	if (closer) csvd->tabs--;
	free(buf);
}

void textout(csvdata *csvd, char *text, FILE *fpo)
{
	char tablist[16];
	strcpy(tablist, csvd->tabsx10);
	csvd->tabs++;
	tablist[csvd->tabs] = 0;
	fprintf(fpo, "%s%s\n", tablist, text);
	csvd->tabs--;
}

void checkhavecss(void)
{
	if (direxists("./css/")) {
		mkdir("css", 775);
	}
	if (fileexists("./css/c2hmaster.css")) {
		char path[NAME_MAX];
		sprintf(path, "%s/.config/htmledit/c2hmaster.css", getenv("HOME"));
		fdata mydat = readfile(path, 0, 1);
		writefile("./css/c2hmaster.css", mydat.from, mydat.to, "w");
		free(mydat.from);
	}
}

tagstruct *gettaggroup(char *taglist, char *tagname, int fatal)
{	/* The generated tags here are only used for search, the actual
	 * tags output will be those written in the searched data. This
	 * means that any 'style=...' in such data will be preserved.
	*/
	size_t len = strlen(taglist);
	char *tablespec = docalloc(len + 1, 1, "gettaggroup");
	strcpy(tablespec, taglist);
	char tagbuf[NAME_MAX];
	len = strlen(tagname);
	if (len > NAME_MAX - 4) {
		fprintf(stderr, "Tag name too long\n%s\n", tagname);
		exit(EXIT_FAILURE);	// should never happen
	}
	sprintf(tagbuf, "<%s", tagname);
	char *cp = strstr(tablespec, tagbuf);
	if (!cp) {
		if (fatal) {
			fprintf(stderr, "No tag found: %s\n", tagname);
			exit(EXIT_FAILURE);
		} else {
			free(tablespec);
			return (tagstruct*) NULL;
		}
	}
	tagstruct *lts = maketagstruct();
	char *te = strchr(cp, '>');
	lts->tagopn = data2str(cp, te+1);
	sprintf(tagbuf, "</%s>", tagname);
	char *ct = strstr(cp, tagbuf);
	lts->tagcls = dostrdup(tagbuf);
	lts->data = data2str(te + 1, ct);
	free(tablespec);
	return lts;
} // gettaggroup()

char *data2str(char *fr, char *to)
{
	int len = to - fr;
	if (len == 0) {
		return dostrdup("");
	}
	char *buf = docalloc(len + 1, 1, "data2str");
	strncpy(buf, fr, len);
	buf[len] = 0;
	char *cp = buf;
	char *ep = buf + len -1;
	while(*cp == ' ') cp++;
	while(*ep == ' ') ep--;
	char *ret;
	if (cp >= ep) {	// all ' '
		ret = dostrdup("");
		free(buf);
		return ret;
	}
	ep++;	// get back to the ' ' or where it started from.
	*ep = 0;
	ret = dostrdup(cp);
	free(buf);
	return ret;
}

tagstruct *maketagstruct(void)
{
	tagstruct *tstr = docalloc(sizeof(tagstruct), 1, "maketagstruct");
	tstr->data = NULL;
	tstr->tagopn = NULL;
	tstr->tagcls = NULL;
	return tstr;
}

void destroytagstruct(tagstruct *tstr)
{
	if(tstr->data) free(tstr->data);
	if(tstr->tagcls) free(tstr->tagcls);
	if(tstr->tagopn) free(tstr->tagopn);
	free(tstr);
}

char *insertclass(char *tag, char *class)
{
	size_t buflen = strlen(class) + strlen(tag) + 3;
	char *buf = docalloc(buflen, 1, "insertclass");
	strcpy(buf, tag);
	char *cp = strchr(buf, ' ');
	char *tail;
	if (cp) {
		tail = dostrdup(cp);
		*cp = 0;
	} else {
		cp = strchr(buf, '>');
		*cp = 0;
		tail = dostrdup(">");
	}
	char *classp = strchr(class, ' ');
	if (!classp) {
		strcat(buf, " ");
		classp = class;
	}
	strcat(buf, classp);
	strcat(buf, tail);
	free(tail);
	char *ret = dostrdup(buf);
	free(buf);
	return ret;
} // insertclass()

csvdata *makecsvdata(void)
{
	csvdata *csvd = docalloc(sizeof(csvdata), 1, "makecsvdata");
	csvd->tabsx10 = "\t\t\t\t\t\t\t\t\t\t";	// is read only.
	csvd->isdone = 0;
	return csvd;
}

void destroycsvdata(csvdata *csvd)
{
	free(csvd);	// that's all folks.
}

char *getnextdatarow(csvdata *csvd)
{
	if(csvd->isdone) return NULL;
	char *cp = csvd->dataline;
	if(!cp) return cp;	// it was NULL'd last time.
	size_t len = strlen(cp);
	csvd->dataline += len + 1;
	if (csvd->curfilerow > csvd->rowsinfile) {
		cp = NULL;
	}
	return cp;
} // getnextdatarow()

char **maketaglist(char *htags, char *tagname, char **closer)
{	/* The opening tags can differ in value due to maybe having
	 * different styling but they will be closed by tags all having
	 * the same value. eg "<td style=....>data</td>"
	*/
	size_t tnlen = strlen(tagname);
	char *clstag = docalloc(tnlen + 4, 1, "maketaglist");
	sprintf(clstag, "</%s>", tagname);
	char srch[NAME_MAX];
	sprintf(srch, "<%s", tagname);	// to count the openers
	int tcount = 0;
	char *cp = htags;
	while ((cp = strstr(cp, srch))) {
		tcount++;
		cp = strstr(cp, clstag);
	}
	char **ret = docalloc((tcount + 1) * sizeof(char*), 1,
							"maketaglist");
	int i;
	cp = htags;
	for (i = 0; (cp = strstr(cp, srch)) ; i++) {
		char *ep = strchr(cp, '>');
		if(!ep) break;
		char *buf = docalloc(ep - cp + 2, 1, "maketaglist");
		strncpy(buf, cp, ep-cp+1);
		ret[i] = buf;
		cp = strstr(cp, clstag);
	}
	*closer = clstag;
	return ret;
} // maketaglist()

void destroytaglist(char **taglist, char *closer)
{
	int i;
	for (i = 0; taglist[i]; i++) {
		free(taglist[i]);
	}
	free(closer);
	free(taglist);
} // destroytaglist()

void generatetables(csvdata *csvd, htdata *htd, const char *csvfile)
{
	fdata mydat = readfile(csvfile, 0, 1);
	csvd->rowsinfile = dat2cstr(mydat.from, mydat.to);
	csvd->dataline = mydat.from;
	csvd->dataend = mydat.to;
	FILE *fpo = dofopen(htd->htfn, "a");
	csvd->pageno = 1;	// odd pages recto, even verso.
	/* pagination is never conditional for this job, there is simply
	 * a series of pages on each of which is a table.
	*/
	csvd->curfilerow = csvd->curtablerow = csvd->tabs = 0;
	csvd->rowsthistable = getrows(htd->pagerowslist);
	while ((csvd->rowsthistable)) {
		writenewpage(csvd, htd, fpo);
		csvd->rowsthistable = getrows(htd->pagerowslist);
	}
	// TODO: put summary of lines written on stdout.
	dofclose(fpo);
	free(mydat.from);
}

void writenewpage(csvdata *csvd, htdata *htd, FILE *fpo)
{	/* writes the divs that force the new page and brings the page
	 * title line.
	*/
	if(csvd->isdone) return;
	tagout(csvd, htd->divpagetop, fpo);
	char *pd = (csvd->pageno % 2 != 0) ? htd->divrecto : htd->divverso;
	tagout(csvd, pd, fpo);
	textout(csvd, htd->title, fpo);
	tagout(csvd, "</div>", fpo);	// page title line
	tagout(csvd, "</div>", fpo);	// force page break;
	// NB different div names
	pd = (csvd->pageno % 2 != 0) ? htd->divtrecto : htd->divtverso;
	tagout(csvd, pd, fpo);
	writetable(csvd, htd, fpo);
	tagout(csvd, "</div>", fpo);
	csvd->pageno++;
	csvd->curtablerow = 0;
}

void writetable(csvdata *csvd, htdata *htd, FILE* fpo)
{
	if(csvd->isdone) return;
	tagstruct *tts = gettaggroup(htd->tablespec, "table", 1);
	tagout(csvd, tts->tagopn, fpo);
	writeheader(csvd, htd, tts->data, fpo);
	writebody(csvd, htd, tts->data, fpo);
	writefooter(csvd, htd, tts->data, fpo);
	tagout(csvd, tts->tagcls, fpo);
	destroytagstruct(tts);
}

void writebody(csvdata *csvd, htdata *htd, char *htags, FILE *fpo)
{
	if(csvd->isdone) return;
	tagstruct *wbts = gettaggroup(htags, "tbody", 1);
	tagout(csvd, wbts->tagopn, fpo);
	writedatarow(csvd, htd, wbts->data, fpo);
	tagout(csvd, wbts->tagcls, fpo);
	destroytagstruct(wbts);
} // writebody()

void writefixedtd(csvdata *csvd, char *htags, FILE *fpo, char *thtd)
{	/* Writes the td|th lines available in the user's generated xml.
	 * This is no use for the generated data from the csv file.
	*/
	char srchfr[4];	//"<td" | "<th"
	char srchbk[6];	// "</td>" | "</th>"
	sprintf(srchfr, "<%s", thtd);
	sprintf(srchbk, "</%s>", thtd);
	char *cp = strstr(htags, srchfr);
	while (cp) {
		tagstruct *wrft = gettaggroup(cp, thtd, 1);
		tagout(csvd, wrft->tagopn, fpo);
		textout(csvd, wrft->data, fpo);
		tagout(csvd, wrft->tagcls, fpo);
		cp = strstr(cp, srchbk);
		if(cp) cp = strstr(cp, srchfr);
		destroytagstruct(wrft);
	}
} // writefixedtd()

void writeheader(csvdata *csvd, htdata *htd, char *htags, FILE *fpo)
{
	if(csvd->isdone) return;
	tagstruct *wtts = gettaggroup(htags, "thead", 0);
	// Absence of this tag is OK.
	if (!wtts) return;	// nothing to clean up;
	tagout(csvd, wtts->tagopn, fpo);
	writerow(csvd, htd, wtts->data, fpo, "th");
	tagout(csvd, wtts->tagcls, fpo);
	destroytagstruct(wtts);
}

void writefooter(csvdata *csvd, htdata *htd, char *htags, FILE *fpo)
{
	tagstruct *wttf = gettaggroup(htags, "tfoot", 0);
	// Absence of this tag is OK.
	if (!wttf) return;	// nothing to clean up;
	tagout(csvd, wttf->tagopn, fpo);
	writerow(csvd, htd, wttf->data, fpo, "td");
	tagout(csvd, wttf->tagcls, fpo);
	destroytagstruct(wttf);
}

void writerow(csvdata *csvd, htdata *htd, char *htags, FILE *fpo,
					char *thtd)
{
	if(csvd->isdone) return;
	tagstruct *wrts = gettaggroup(htags, "tr", 1);
	char *rowclass = (csvd->curtablerow %2 == 0)
						? htd->troweven : htd->trowodd;
	char *opntag = insertclass(wrts->tagopn, rowclass);
	tagout(csvd, opntag, fpo);
	free(opntag);
	writefixedtd(csvd, wrts->data, fpo, thtd);
	tagout(csvd, wrts->tagcls, fpo);
	destroytagstruct(wrts);
	csvd->curtablerow++;
}

void writedatarow(csvdata *csvd, htdata *htd, char *htags,
							FILE *fpo)
{	/* There is only 1 instance of <tr...> within the <tbody> group,
	 * so I retrieve it and manipulate it just once for all rows in
	 * each table/page.
	*/
	if(csvd->isdone) return;
	tagstruct *wdrts = gettaggroup(htags, "tr", 1);
	char *oddtag = insertclass(wdrts->tagopn, htd->trowodd);
	char *evntag = insertclass(wdrts->tagopn, htd->troweven);
	while (csvd->curtablerow < csvd->rowsthistable) {
		char *opntag = (csvd->curtablerow %2) ? oddtag : evntag;
		tagout(csvd, opntag, fpo);
		writedatadetail(csvd, wdrts->data, fpo);
		tagout(csvd, wdrts->tagcls, fpo);
		if(csvd->isdone) break;	// has written empty <tr></tr>
		csvd->curtablerow++;
		csvd->curfilerow++;
	}
	free(evntag);
	free(oddtag);
	destroytagstruct(wdrts);
} // writedatarow()

void writedatadetail(csvdata *csvd, char *htags, FILE *fpo)
{	/* A tagstruct is not useful here because I may have several <td...>
	 * so different functions to serve up both the tags and data details
	 * will be used instead.
	*/
	if(csvd->isdone) return;
	char *clstag;
	char **taglist = maketaglist(htags, "td", &clstag);
	char *line = getnextdatarow(csvd);
	if (!line || !strlen(line)) {
		csvd->isdone = 1;
		return;
	}
	int i = 0;
	while (taglist[i]) {
		tagout(csvd, taglist[i], fpo);
		char *text = linepart(line, i);
		textout(csvd, text, fpo);
		free(text);
		tagout(csvd, clstag, fpo);
		i++;
	}
	destroytaglist(taglist, clstag);
}

char *linepart(char *line, int segment)
{
	char *buf = docalloc(strlen(line) +1, 1, "linepart");
	strcpy(buf, line);
	char *list[100];	// should be enough
	int protected = 0;
	int hasbeenprot = 0;
	char *cp = buf;
	int i = 0;
	list[i] = cp;
	i++;
	while (*cp) {
		switch (*cp) {
			case '"':
				if (protected) {
					protected = 0;
				} else {
					protected = 1;
					hasbeenprot = 1;
				}
				break;
			case ',':
				if (!protected) {
					*cp = 0;
					list[i] = cp + 1;
					i++;
				}

			default:
				break;
		} // switch()
		cp++;
	} // while()
	if (hasbeenprot) {
	/* get rid of '"' inserted to protect embedded commas found when
	 * converting to csv format.
	*/
		cp = buf;
		char *ep = cp + strlen(line);
		// must be able to get past embedded '\0' in buf[].
		while (cp < ep) {
			if (*cp == '"') *cp = ' ';	// extraneous spaces in html ok.
			cp++;
		}
	}
	char *ret = dostrdup(list[segment]);
	free(buf);
	return ret;
} // linepart()

void makeblank(htdata *htd)
{	/* make a blank page actionable by wkhtmltopdf */
	char *text =
	"<div style=\"page-break-before: always\">\n<p>&nbsp;</p></div>\n";
	writefile(htd->htfn, text, NULL, "a");
}
