// Baseresource.cc for bbtools - tools to display resources in X11.
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

#include <string>
#include "Baseresource.h"
#include <stdio.h>
#include "Menu.hh"

BaseResource::BaseResource(bt::Application &_app, unsigned int _screen, const std::string &blackbox_rc_filename, const std::string &filename) : 
    app(_app), display(_app.display())
{
    const bt::ScreenInfo& screeninfo = _app.display().screenInfo(_screen);
    screen = _screen;

    // get blackbox configuration file
    if (!blackbox_rc_filename.empty())
    {
        bt_resource.load(blackbox_rc_filename);
    }
    else
    {
        bt_resource.load(std::string("~/.blackboxrc"));
    }
    
    if (bt_resource.valid()) 
    {
        const std::string blackbox_stylefile = bt_resource.read("session.styleFile", "Session.StyleFile", "");

        colors_per_channel = readInt("session.colors_per_channel", "Session.Colors_per_channel", 4);
        if (colors_per_channel < 2) colors_per_channel = 2;
        if (colors_per_channel > 6) colors_per_channel = 6;

        image_dither = readBool("session.imageDither", "Session.ImageDither", true);

        if (image_dither &&
            screeninfo.visual()->c_class == TrueColor &&
                screeninfo.depth() >= 24)
            image_dither = false;

        if (!blackbox_stylefile.empty()) 
        {
            bt_resource.load(blackbox_stylefile);
        }
#if 0
        bool use_default = true;
        if (!filename.empty()) {
            if (!bt_resource.merge(filename)) {
                fprintf(stderr, "warning: Cannot open resource file %s, using default\n", filename.c_str());
            } else {
                use_default = false;
            }
        }
        
        if (use_default) {
            if (!bt_resource.merge(std::string(BBTOOL_LOCAL))) {
                if (!bt_resource.merge(std::string(BBTOOL_GLOBAL))) {
                    fprintf(stderr, "Warning: Cannot open resource files, using internal defaults\n");
                }
            }
        }
#endif
    }
    if (!filename.empty()) {
        bt_resource.merge(filename);
    } else {
        bt_resource.merge(std::string(BBTOOL_LOCAL));
        bt_resource.merge(std::string(BBTOOL_GLOBAL));
    }
}

BaseResource::~BaseResource()
{
  
}

std::string BaseResource::readString(const std::string &rname, const std::string &rclass, const std::string &default_val)
{
    return(bt_resource.read(rname, rclass, default_val));
}

std::string BaseResource::readString(const std::string &rname, const std::string &rclass,
                     const std::string &alt_rname, const std::string &alt_rclass,
                     const std::string &default_val)
{
    std::string tmp = bt_resource.read(rname, rclass, "");
    if (tmp.empty())
        return(bt_resource.read(alt_rname, alt_rclass, default_val));
    return(tmp);
}

int BaseResource::readInt(const std::string &rname, const std::string &rclass, int default_val)
{
    int ret_val;
    std::string tmp = bt_resource.read(rname, rclass, "0");
    if ((tmp.empty()) || (sscanf(tmp.c_str(), "%d", &ret_val) != 1))
    {
        return(default_val);
    }
    return(ret_val);
}

unsigned int BaseResource::readUInt(const std::string &rname, const std::string &rclass, unsigned int default_val)
{
    int ret_val;
    std::string tmp = bt_resource.read(rname, rclass, "");
    if ((tmp.empty()) || (sscanf(tmp.c_str(), "%u", &ret_val) != 1))
    {
        return(default_val);
    }
    return(ret_val);
}

unsigned int BaseResource::readUInt(const std::string &rname, const std::string &rclass, 
                    const std::string &alt_rname, const std::string &alt_rclass, 
                    unsigned int default_val)
{
    int ret_val;
    std::string tmp = bt_resource.read(rname, rclass, "");
    if (tmp.empty()) {
        std::string tmp = bt_resource.read(alt_rname, alt_rclass, "");
        if ((tmp.empty()) || (sscanf(tmp.c_str(), "%u", &ret_val) != 1))
            return(default_val);
    }
    return(ret_val);
}

bool BaseResource::readBool(const std::string &rname, const std::string &rclass, bool default_val)
{
    std::string tmp = bt_resource.read(rname, rclass, "");
    if (!strcasecmp(tmp.c_str(),"true")) 
        return(true); 
    if (!strcasecmp(tmp.c_str(),"false"))
        return(false);

    return(default_val);
}

bt::Color BaseResource::readColor(const std::string &rname,const std::string &rclass,
                              const std::string &default_color)
{
    std::string rcolor = bt_resource.read(rname, rclass, default_color);
    bt::Color color;
    color = bt::Color::namedColor(display, screen, rcolor);
    return(color);
}


bt::Color BaseResource::readColor(const std::string &rname,const std::string &rclass,
                  const std::string &alt_rname, const std::string &alt_rclass,
                              const std::string &default_color)
{
    std::string rcolor = bt_resource.read(rname, rclass, "");
    if (rcolor.empty()) {
        rcolor = bt_resource.read(alt_rname, alt_rclass, default_color);
    }
    bt::Color color;
    color = bt::Color::namedColor(display, screen, rcolor);
    return(color);
}

bt::Texture BaseResource::readTexture(const std::string &rname, 
                      const std::string &rclass,
                      const std::string &default_texture,
                      const std::string &default_color,
                      const std::string &default_colorTo)
{
  bt::Texture dtexture;
  dtexture.setDescription(default_texture);
  dtexture.setColor1( bt::Color::namedColor(display, screen, default_color) );
  dtexture.setColor2( bt::Color::namedColor(display, screen, default_colorTo) );

  return bt::textureResource(display, screen, bt_resource, rname, rclass, dtexture);
}


bt::Texture BaseResource::readTexture(const std::string &rname, 
                      const std::string &rclass,
                      const std::string &alt_rname,
                      const std::string &alt_rclass,
                      const std::string &default_texture,
                      const std::string &default_color,
                      const std::string &default_colorTo)

{
  bt::Texture dtexture;
  dtexture.setDescription(default_texture);
  dtexture.setColor1( bt::Color::namedColor(display, screen, default_color) );
  dtexture.setColor2( bt::Color::namedColor(display, screen, default_colorTo) );

  std::string rtexture = bt_resource.read(rname, rclass, 
                                          bt_resource.read(rname + ".appearance",
                                                           rclass + "Appearance", ""));

  bt::Texture texture;

  if (!rtexture.empty()) {
    texture = bt::textureResource(display, screen, bt_resource, rname, rclass, dtexture);
  }
  else {
    texture = bt::textureResource(display, screen, bt_resource, alt_rname, alt_rclass, dtexture);
  }

  return texture;
}

bt::Font BaseResource::readFont(const std::string &rname, 
                const std::string &rclass,
                const std::string &alt_rname,
                const std::string &alt_rclass)
{
    std::string font_name = bt_resource.read(rname, rclass, "");
    if (font_name.empty()) {
        font_name = bt_resource.read(alt_rname, alt_rclass, "");
    }
    bt::Font font(font_name);
    return(font);
}

void BaseResource::loadMenuStyle(void)
{
    bt::MenuStyle::get(app, screen)->load(bt_resource);
}


std::string BaseResource::getColorName(const bt::Color &color)
{
    char color_name[8];

    snprintf(color_name, 8, "#%02x%02x%02X", color.red(), color.green(), color.blue());
    return std::string(color_name);
}


