//  main.cc for bbtools.
//
//  Copyright (c) 1998-2003 by John Kennis, jkennis@chello.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
// (See the included file COPYING / GPL-2.0)
//

#include "bbpager.hh"
#include "main.hh"
#include "i18n.hh"
#include "config.h"

bt::I18n bt::i18n; // initialized in main

Configuration::Configuration(int iargc, char **iargv)
{
	withdrawn = false;
	decorated = false;
	shape = false;
	_argc = iargc;
	_argv = iargv;
	app_name = iargv[0];
}

Configuration::~Configuration(void)
{
}

void Usage()
{
	fprintf(stdout,"\n%s version %s \n", PACKAGE, VERSION);
	fprintf(stdout,"Usage: %s [options]\n",PACKAGE);
	fprintf(stdout,"Options:\n");
	fprintf(stdout,"-display <display name>     X server to connect to\n");
	fprintf(stdout,"-c[onfig] <filename>        Alternate config file\n");
	fprintf(stdout,"-v[ersion]                  Display version number\n");
	fprintf(stdout,"-h[elp]                     Display this help\n");
	fprintf(stdout,"-geom[etry] <geometry>      Set geometry of window\n");
	fprintf(stdout,"-d[ecorated]                Show 'normal' decorated window\n");
	fprintf(stdout,"-w[ithdrawn]                Place bbtool in the Slit\n");
	fprintf(stdout,"-s[hape]                    Don't display groundplate\n");
}


int main(int argc,char **argv)
{
	int i;
	Configuration options(argc, argv);

	for(i = 1; i < argc; i++) {
		if ((!strcmp(argv[i],"-display"))) {
			if(++i==argc)  {
				Usage();
				exit(2);
			};
			options.setDisplayName(argv[i]);
		} else if ((!strcmp(argv[i],"-config"))|(!strcmp(argv[i],"-c"))) {
			if(++i==argc)  {
				Usage();
				exit(2);
			};
			options.setRcFilename(argv[i]);
		} else if ( (!strcmp(argv[i],"-v")) || (!strcmp(argv[i],"-version"))) {
			fprintf(stderr," %s version %s\n", PACKAGE, VERSION);
			exit(2);
		} else if ( (!strcmp(argv[i],"-h")) || (!strcmp(argv[i],"-help"))) {
			Usage();
			exit(2);
		} else if ((!strcmp(argv[i],"-geometry")) || (!strcmp(argv[i],"-geom"))) {
			if(++i==argc)  {
				Usage();
				exit(2);
			};
			options.setGeometry(argv[i]);
		}
		else if ((!strcmp(argv[i],"-withdrawn")) || (!strcmp(argv[i],"-w"))) {
			options.setWithdrawn(true);
		} else if ((!strcmp(argv[i],"-shape")) || (!strcmp(argv[i],"-s"))) {
			options.setShaped(true);
		} else if ((!strcmp(argv[i],"-decorated")) || (!strcmp(argv[i],"-d"))) {
			options.setDecorated(true);
		}
	}

	ToolWindow bbpager(options);
	bbpager.eventLoop();
}
