//  main.hh for bbtools.
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
#ifndef __MAIN_H
#define __MAIN_H

class Configuration
{
public:
	Configuration(int iargc, char **argv);
	~Configuration(void);
	
	int argc(void) { return _argc; }
	char **argv(void) { return _argv; }
	
	bool isWithdrawn(void) { return withdrawn; }
	void setWithdrawn(bool _withdrawn) { withdrawn = _withdrawn; }

	bool isDecorated(void) { return decorated; }
	void setDecorated(bool _decorated) { decorated = _decorated; }

	bool isShaped(void) { return shape; }
	void setShaped(bool _shape) { shape = _shape; }

	const std::string &rcFilename(void) { return rc_filename; }
	void setRcFilename(std::string filename) { rc_filename = filename; }

    const std::string &blackboxRcFilename(void) 
        { return blackbox_rc_filename; }
	void setBlackboxRcFilename(std::string filename) 
        { blackbox_rc_filename = filename; }

	const std::string &appName(void) { return app_name; }
	void setAppName(std::string name) { app_name = name; }

	const std::string &displayName(void) { return display_name; }
	void setDisplayName(std::string name) { display_name = name; }

	const std::string &geometry(void) { return _geometry; }
	void setGeometry(std::string geo) { _geometry = geo; }
	
private:
	bool withdrawn;
	bool decorated;
	bool shape;
	int _argc;
	char **_argv;
	std::string _geometry;
	
	std::string rc_filename;
	std::string blackbox_rc_filename;
	std::string app_name;
	std::string display_name;
};

#endif // __MAIN_H
