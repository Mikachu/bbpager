// pager.h for bbpager - an pager tool for Blackbox.
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


#ifndef __PAGER_H
#define __PAGER_H

#include "bbpager.h"

class DesktopWindow;

class PagerWindow : public bt::EventHandler 
{
	
public:
	PagerWindow(ToolWindow *toolwindow, Window _win);		
	~PagerWindow(void);
 
    Window window(void);
    Window window(int desktop);

	Window realWindow(void) { return win; }
	
	int initWindowGeometry(void);
	void reconfigure(void);
	void buildWindow(bool reconfigure);

	void setFocus(void);
	void clearFocus(void);

	unsigned int desktopId(void) { return desktop_id; }
    void setDesktopId(unsigned int id) { desktop_id = id; }

    bool isSticky(void) { return(sticky); }
    void setSticky(bool val) { sticky = val; }

    bool isFocused(void) { return(focused); }
    bool isShaded(void) { return(shaded); }
    bool isSkipped(void) { return(skip); }

    int x() { return pager_x; }
    int y() { return pager_y; }

    //void width(int w) { pager_width = w; }
    //void height(int h) { pager_height = h; }
    int width(void) { return focused ? pager_focus_width : pager_unfocus_width; }
    int height(void) { return focused ? pager_focus_height : pager_unfocus_height; }

    bool isMarked(void) { bool tmp = marked; marked = false; return(tmp); } 
    void mark(bool val) { marked = val; }
    void configureNotifyEvent(const XConfigureEvent * const event);
    void propertyNotifyEvent(const XPropertyEvent * const event);
    void exposeEvent(const XExposeEvent * const event);
    
    void raise(void);
    void lower(void);
    void redraw(void);

    Pixmap getPixmap(void) { return(pixmap); }
    Pixmap getFocusedPixmap(void) { return(pixmap_focused); }
    
    bt::Texture getTexture(void);
    bt::Texture getFocusedTexture(void);
    
private:
    ToolWindow *bbtool;
    bt::EWMH *ewmh;
    int screen;
    ::Display *display;
	Window win;
	Window *pwin;

	Pixmap pixmap;
	Pixmap pixmap_focused;

	int window_x;
	int window_y;
	unsigned int window_width;
	unsigned int window_height;

    unsigned int desktop_id;
	
	bool hidden;
    bool iconic;
	bool focused;
	bool shaded;
    bool marked;
    bool skip;

	int pager_x;
	int pager_y;
	int pager_focus_width;
	int pager_focus_height;
	int pager_unfocus_width;
	int pager_unfocus_height;
	unsigned int desktop_nr;
	bool sticky;
    unsigned int number_of_desktops;

    void buildPagerWindow(bool reconfigure, unsigned int nr);
    void showWindow();
    void hideWindow();
    void destroyWindow();
    void calcGeometry();

    DesktopWindow *m_pDesktop;
};

#endif // __PAGER_H

