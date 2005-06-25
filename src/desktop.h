// desktop.h for bbpager - an pager tool for Blackbox.
//
//  Copyright (c) 1998-2004 by John Kennis, jkennis@chello.nl
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


#ifndef __DESKTOP_H
#define __DESKTOP_H

#include "bbpager.h"

class PagerWindow;

class DesktopWindow : public bt::EventHandler
{
public:
	DesktopWindow(ToolWindow *toolwindow, unsigned int _desktop_nr);
	~DesktopWindow(void);

	Window window(void) { return win; }

    unsigned int desktopId(void) { return desktop_id; }

	void reconfigure(void);
	void buildWindow(bool reconfigure);

	void setFocus(void);
	void clearFocus(void);

    int x(void) { return(_x); }
    int y(void) { return(_y); }
    int width(void) { return(_width); }
    int height(void) { return(_height); }

    void redraw(void);
	
    virtual void buttonPressEvent(const XButtonEvent * const event);
    virtual void buttonReleaseEvent(const XButtonEvent * const event);
    virtual void motionNotifyEvent(const XMotionEvent * const event);
    virtual void exposeEvent(const XExposeEvent * const event);

private:
	Window win;
    Window grabbedWindow;
    PagerWindow *moveWindow;
    Window realWindow;
    Window pagerWindow;

    int grabbed_x;
    int grabbed_y;
    int move_x;
    int move_y;
    bool moved;
    bool m_focused;

    int screen;
	::Display *display;
    Resource *resource;
	Pixmap pixmap;
	Pixmap pixmap_focused;
	
    ToolWindow *bbtool;
	unsigned int desktop_id;
	int _x;
	int _y;
	int _width;
	int _height;
    unsigned int desktop_nr;
   
    void calcPosition(void);

};

#endif // __DESKTOP_H

