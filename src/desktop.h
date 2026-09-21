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

	bool focused(void) { return m_focused; }
	void setFocus(void);
	void clearFocus(void);

	int windowX(void) { return(_window_x); }
	int windowY(void) { return(_window_y); }
	int windowWidth(void) { return(_window_width); }
	int windowHeight(void) { return(_window_height); }

	int desktopX(void) { return(_desktop_x); }
	int desktopY(void) { return(_desktop_y); }
	int desktopWidth(void) { return(_desktop_width); }
	int desktopHeight(void) { return(_desktop_height); }

	void redraw(void);

	virtual void buttonPressEvent(const XButtonEvent * const event);
	virtual void buttonReleaseEvent(const XButtonEvent * const event);
	virtual void motionNotifyEvent(const XMotionEvent * const event);
	virtual void exposeEvent(const XExposeEvent * const event);
	Pixmap pixmap(void) { return m_pixmap; }

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

	int screen;
	::Display *display;

	ToolWindow *bbtool;

	bool m_focused;
	Pixmap m_pixmap;
	Pixmap m_pixmapFocused;


	unsigned int desktop_id;
	int _cell_x;
	int _cell_y;
	int _cell_width;
	int _cell_height;
	int _window_x;
	int _window_y;
	int _window_width;
	int _window_height;
	int _desktop_x;
	int _desktop_y;
	int _desktop_width;
	int _desktop_height;
	unsigned int desktop_nr;

	void calcGeometry(void);
};

#endif // __DESKTOP_H

