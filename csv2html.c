
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
	return argv[optind];
}

void sanitycheck(int optind, char **argv)
{
	int ok = 1;
	ok = (ok && (argv[optind]));
	ok = (ok && (argv[optind+1]));
	ok = (ok && (!(fileexists(argv[optind]))));
	ok = (ok && (!(fileexists(argv[optind+1]))));
	if (!(ok)) {
		fprintf(stderr, "You must provide a control file and a csv file"
						" name, and both files must exist.\n");
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
