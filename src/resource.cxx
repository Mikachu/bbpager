// resource.cc for bbpager - a pager tool for Blackbox
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

#include "resource.h"
#include "blackboxstyle.h"

Resource::Resource(ToolWindow *toolwindow, const std::string &blackbox_rc_file, const std::string &rc_file): 
		BaseResource(*toolwindow, toolwindow->getCurrentScreen(), blackbox_rc_file, rc_file), bbtool(toolwindow)
{
	load();
}

Resource::~Resource() 
{
	clean();
}

void Resource::clean() 
{
}



void Resource::load(void) 
{
	unsigned int button = 0;
	WHICH_BUTTON move_default = LEFT_BUTTON;

	button = readUInt("bbpager.desktopChangeButton", "Bbpager.Desktopchangebutton", MIDDLE_BUTTON);
	if (button > 5 || button < 1) {
		button = MIDDLE_BUTTON;
	}
	desktop_change_button = static_cast<WHICH_BUTTON>(button);
	if (desktop_change_button == LEFT_BUTTON) {
		move_default = MIDDLE_BUTTON;
	}

	button = readUInt("bbpager.windowMoveButton", "Bbpager.WindowMovebutton", move_default);
	if (button > 5 || button < 1) {
		button = move_default;
	}
	window_move_button = static_cast<WHICH_BUTTON>(button);

	button = readUInt("bbpager.windowFocusButton", "Bbpager.WindowFocusbutton", INVALID_BUTTON);
	if (button > 5 || button < 1) {
		button = INVALID_BUTTON;
	}

	button = readUInt("bbpager.windowRaiseButton", "Bbpager.WindowRaisebutton", INVALID_BUTTON);
	if (button > 5 || button < 1) {
		button = INVALID_BUTTON;
	}

	Frame();
 
	SizeAndPosition();

	PagerWin();
}


void Resource::Frame() 
{
	frame.texture = readTexture("bbpager.frame","BbPager.Frame",BB_FRAME, BB_C_FRAME,
				    "Raised Gradient Vertical Bevel1", "slategrey","darkslategrey");

	frame.bevelWidth = readUInt("bbpager.bevelWidth","Bbpager.BevelWidth", 
                                readUInt( "bbpager.margin","Bbpager.margin", 4));
	if (frame.bevelWidth == 0)
		frame.bevelWidth = 4;

	frame.desktopMargin = readUInt("bbpager.desktop.bevelWidth","Bbpager.desktop.BevelWidth", 
                                readUInt( "bbpager.desktop.margin","Bbpager.desktop.margin", frame.bevelWidth));
	if (frame.desktopMargin == 0)
		frame.desktopMargin = 4;
}


void Resource::SizeAndPosition() 
{
	unsigned int w, h;

	if (!(bbtool->configuration().isWithdrawn())) 
		bbtool->configuration().setWithdrawn(readBool("bbpager.withdrawn", "Bbpager.Withdrawn", false));

	if (!(bbtool->configuration().isShaped())) 
		bbtool->configuration().setShaped(readBool("bbpager.shape", "Bbpager.Shape", false /*bbtool->configuration().isWithdrawn() */));

	if (bbtool->configuration().geometry().empty()) {
		std::string positionstring = readString("bbpager.position","Bbpager.Position", "-0-0");
		position.mask = XParseGeometry(positionstring.c_str(), &position.x, &position.y, &w, &h);
		if (!(position.mask & XValue))
			position.x = 0;
		if (!(position.mask & YValue))
			position.y = 0;
	} else {
    	position.mask = XParseGeometry(bbtool->configuration().geometry().c_str(), &position.x, &position.y, &w, &h);
		if (!(position.mask & XValue))
			position.x = 0;
		if (!(position.mask & YValue))
			position.y = 0;

    }

    position.horizontal = false;
    position.vertical = false;
 	std::string orientation = readString( "bbpager.desktop.orientation", "Bbpager.Desktop.Orientation", "horizontal");

    if (orientation == "vertical") {
       position.vertical = true;
    } else {
            position.horizontal = true;
    }   


  	columns = readUInt("bbpager.desktop.columns", "Bbpager.Desktop.Columns", 0xFFFF);
	if (columns == 0) 
	  columns = 0xFFFF;

	rows = readUInt("bbpager.desktop.rows","Bbpager.Desktop.Rows", 0XFFFF);
	if (rows == 0) 
		rows = 0xFFFF;
  
	int default_width;
	if (!bbtool->configuration().isWithdrawn())
		default_width = 40;
	else
		default_width = 64;

	desktopSize.width = readUInt("bbpager.desktop.width", "Bbpager.Desktop.Width", default_width);
      
	int default_height;
	if (!bbtool->configuration().isWithdrawn())
		default_height = 30;
	else
		default_height = 48;

	desktopSize.height = readUInt("bbpager.desktop.height", "Bbpager.Desktop.Height", default_height);
}

void Resource::PagerWin() 
{
	std::string focus_style;

	focus_style = readString( "bbpager.desktop.focusStyle", "Bbpager.Desktop.FocusStyle", "border");

    if (strcasecmp("texture", focus_style.c_str()) == 0) {
		desktop_focus_style = texture;
    } else if (! strcasecmp("none", focus_style.c_str())) 
		desktop_focus_style = none;
	else 
		desktop_focus_style = border;
  
	desktopwin.texture = readTexture("bbpager.desktop", "Bbpager.Desktop",
			                 BB_LABEL,BB_C_LABEL,
					 "Sunken Gradient Diagonal",
					 "slategrey","darkslategrey");

	if (desktop_focus_style == texture)
		desktopwin.focusedTexture = readTexture("bbpager.desktop.focus", "Bbpager.Desktop.Focus",
						       "Sunken Gradient Diagonal",
						       "darkslategrey","slategrey");

	std::string window_focus_style = readString("bbpager.window.focusStyle", "Bbpager.Window.FocusStyle", 
						    "texture");
	
   	if (! strcasecmp("border", window_focus_style.c_str()))
		pager_focus_style = border;
	else if (! strcasecmp("none", window_focus_style.c_str())) 
		pager_focus_style = none;
	else 
		pager_focus_style = texture;

	pagerwin.texture = readTexture("bbpager.window", "Bbpager.Window",
			               BB_WINDOW_UNFOCUS, BB_C_WINDOW_UNFOCUS,
				       "Raised Gradient Diagonal",
				       "rgb:c/9/6","rgb:8/6/4");

	if (pager_focus_style == texture) 
		pagerwin.focusedTexture = readTexture("bbpager.window.focus","Bbpager.Window.Focus",
						       BB_WINDOW_FOCUS,BB_C_WINDOW_FOCUS,
		  				      "Raised Vertical Gradient",
						      "rgb:c/9/6","rgb:8/6/4");
  
 	pagerwin.activeColor = readColor("bbpager.active.window.borderColor",
			                 "Bbpager.active.Window.BorderColor",
				          "LightGrey");

	pagerwin.inactiveColor = readColor("bbpager.inactive.window.borderColor",
					   "Bbpager.inactive.Window.BorderColor",
					   "black");

	desktopwin.activeColor = readColor("bbpager.active.desktop.borderColor",
					   "Bbpager.Active.desktop.BorderColor",
					   "LightGrey");

}

