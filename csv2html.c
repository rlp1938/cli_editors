
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

static void initctlfile(const char *ctlfile);
static void sanitycheck(int argc, char **argv);

int main(int argc, char **argv)
{
	options_t opts = process_options(argc, argv);
	char *ctlfile = NULL;
	char *csvfile = NULL;
	if (opts.controlfile) {
		if (fileexists(argv[1])) {
			fprintf(stderr, "File exists: %s! Quitting", argv[1]);
			exit(EXIT_FAILURE);
		}
		ctlfile = argv[1];
		initctlfile(ctlfile);
		exit(EXIT_SUCCESS);
	}
	
	/* check files exist */
	sanitycheck(argc, argv);
	
	
	return 0;
} // main()

void initctlfile(const char *ctlfile)
{
	if (checkfirstrun("csv2html")) {
		firstrun("csv2html", "master.xml", "csv2html.cfg", NULL);
		fprintf(stdout, "Please edit: %s/.config/%s and try again\n", 
						getenv("HOME"), "csv2html.cfg");
		exit(EXIT_SUCCESS);
	}
}

void sanitycheck(int argc, char **argv)
{
	if (argc < 3) {
		fprintf(stderr, 
		"You must name a control file and csv data file, quiting.\n");
		dohelp(EXIT_FAILURE);
	}
	char *f1 = argv[1];
	if (fileexists(f1) == -1) {
		fprintf(stderr, "No such file: %s\n", f1);
		dohelp(EXIT_FAILURE);		
	}
	char *f2 = argv[2];
	if (fileexists(f2) == -1) {
		fprintf(stderr, "No such file: %s\n", f2);
		dohelp(EXIT_FAILURE);		
	}
}
