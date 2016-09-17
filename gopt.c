
/*      gopt.c
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


options_t process_options(int argc, char **argv)
{
	synopsis =
  "\tcsv2html reads tabular data from a CSV file and writes\n"
  "\tcorresponding tables to a HTML file under the control of an XML\n"
  "\tcontrol file.\n"
  "\n"
  "\tSYNOPSIS:\n"
  "\tcsv2html -i|--init-control-file controlfile\n"
  "\tcsv2html controlfile csvfile\n"
  "\n"
  "\tDESCRIPTION:\n"
  "\tThe generated HTML has page break data inserted into the\n"
  "\tbeginning of as many tables as it takes to display the tabular\n"
  "\tdata from the CSV file. Within the XML control file you need to\n"
  "\tspecify the number of rows for each and every table on each page.\n"
  "\tThe idea is that your HTML is to be printable with your\n"
  "\t(optional) headers and/or footers provided on each page. There\n"
  "\twill be a page break before each table.\n"
  "\n"
  ;
	helptext =
  "\t-h, --help\n"
  "\tPrint this message and exit.\n"
  "\n"
  "\t-i, --init-control-file\n"
  "\tWhen selected a partly completed XML control file is generated.\n"
  "\tYou need to edit that file before use and run the program again.\n"
  "\tThe generated file is commented to show what needs to be added.\n"
  "\tIt is an error if this file exists when selecting this option.\n"
  "\n"
  ;

	optstring = ":hi";

	/* declare and set defaults for local variables. */

	/* set up defaults for opt vars. */
	options_t opts;
	opts.controlfile = 0;

	int c;

	while(1) {
		int this_option_optind = optind ? optind : 1;
		int option_index = 0;
		static struct option long_options[] = {
		{"help",	0,	0,	'h' },
		{"init-control-file",	0,	0,	'i' },
		{0,	0,	0,	0 }
			};


		c = getopt_long(argc, argv, optstring,
                        long_options, &option_index);

		if (c == -1)
			break;

		switch (c) {
		case 0:
			switch (option_index) {
			} // switch()
		break;
		case 'h':
		dohelp(0);
		break;
		case 'i':
			opts.controlfile = 1;
		break;

		case ':':
			fprintf(stderr, "Option %s requires an argument\n",
					argv[this_option_optind]);
			dohelp(1);
		break;
		case '?':
			fprintf(stderr, "Unknown option: %s\n",
					 argv[this_option_optind]);
			dohelp(1);
		break;
		} // switch()
	} // while()
	return opts;
} // process_options()

void dohelp(int forced)
{
  if(strlen(synopsis)) fputs(synopsis, stderr);
  fputs(helptext, stderr);
  exit(forced);
} // dohelp()

