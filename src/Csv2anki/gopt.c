/*      gopt.c
 *
 *  Copyright 2017 Robert L (Bob) Parker rlp1938@gmail.com
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
#include "gopt.h"


options_t process_options(int argc, char **argv)
{
	synopsis =
  "\tSYNOPSIS\n"
  "\tcsv2anki field_list separator_list csv_file\n"
  "\tSends CSV data to stdout from an input CSV file as controlled by\n"
  "\tthe field and separator lists.\n\n"
  "\tField_list is a comma separated list of field numbers in output\n"
  "\torder. Ordering is 1 based. Particular field numbers may be\n"
  "\tdeleted and/or repeated.\n"
  "\tSeparator list is a comma separated list of output separators.\n"
  "\tIf there N iems in the field list there must be N-1 items in the\n"
  "\tseparator list. If the output separators are to be ',' they must\n"
  "\tescaped with '\'. If the end use of the output is to be input to\n"
  "\tAnki the use of ';' as a separator and also ' ' may be used to\n"
  "\tconcatenate 2 fields into one.\n"
  "\tThe CSV file input must use comma as the separator and fields\n"
  "\tcontain embedded ',' protected by enclosing '\"'. This is the\n"
  "\tdefault output of CSV generated by LibreOffice.\n\n"
  ;
	helptext =
  "\tOPTIONS\n"
  "\t-h, --help\n\n"
  "\tOutputs this help message and then quits.\n\n"
  "\tERRORS Apart from normal file system errors, the highest field\n"
  "\tnumber in the field list must not be greater than the number of\n"
  "\tfields in the input CSV file.\n"
  ;

	optstring = ":h";	// initialise

	/* declare and set defaults for local variables. */

	/* set up defaults for opt vars. */
	options_t opts = {0};	// assumes defaults all 0/NULL
	// initialise non-zero defaults below

	int c;

	while(1) {
		int this_option_optind = optind ? optind : 1;
		int option_index = 0;
		static struct option long_options[] = {
		{"help",		0,	0,	'h' },
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

