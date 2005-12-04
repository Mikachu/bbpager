// Baseresource.hh for bbtools - tools to display resources in X11.
//
//  Copyright (c) 1998-2003 John Kennis, jkennis@chello.nl
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

#ifndef __BASERESOURCE_HH
#define __BASERESOURCE_HH

#include "Font.hh"
#include "Resource.hh"
#include "Color.hh"
#include "Texture.hh"
#include "Application.hh"
#include "Display.hh"

class BaseResource
{

	public:
		BaseResource(bt::Application &_app, 
                     unsigned int _screen, c
                     onst std::string &blackbox_rc_filename, 
                     const std::string &filename);

		virtual ~BaseResource(void);


	protected:
		std::string readString(const std::string &rname, const std::string &rclass, const std::string &default_val);
		std::string readString(const std::string &rname, const std::string &rclass,
				       const std::string &alt_rname, const std::string &alt_rclass,
				       const std::string &default_val);

		int readInt(const std::string &rname, const std::string &rclass, int default_val);
		unsigned int readUInt(const std::string &rname, const std::string &rclass, unsigned int default_val);
		unsigned int readUInt(const std::string &rname, const std::string &rclass,
				      const std::string &alt_rname, const std::string &alt_rclass,
				      unsigned int default_val);
	
		bool readBool(const std::string &rname, const std::string &rclass, bool default_val);
		bt::Color readColor(const std::string &rname,const std::string &rclass,
	                             	  const std::string &default_color);
		bt::Color readColor(const std::string &rname,const std::string &rclass,
				    const std::string &alt_rname, const std::string &alt_rclass,
	                            const std::string &default_color);
		bt::Texture readTexture(const std::string &rname, 
					const std::string &rclass,
				       	const std::string &default_texture,
				 	const std::string &default_color,
				 	const std::string &default_colorTo);
		bt::Texture readTexture(const std::string &rname, 
					const std::string &rclass,
					const std::string &alt_rname,
					const std::string &alt_rclass,
				       	const std::string &default_texture,
				 	const std::string &default_color,
				 	const std::string &default_colorTo);
		bt::Font readFont(const std::string &rname, 
				  const std::string &rclass,
				  const std::string &alt_rname,
				  const std::string &alt_rclass);
		std::string getColorName(const bt::Color &color);

		void loadMenuStyle(void);
		
private:
	bt::Resource bt_resource;
	bt::Application &app;
	unsigned int screen;
	const bt::Display &display;

	int colors_per_channel;
	bool image_dither;
};

#endif /* __BASERESOURCE_HH */
