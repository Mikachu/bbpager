// bbpager.hh for bbpager - an pager tool for Blackbox.
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


#ifndef __BBPAGER_HH
#define __BBPAGER_HH

// Blackbox library includes
#include "Application.hh"
#include "Timer.hh"
#include "Texture.hh"
#include "PixmapCache.hh"
#include "Pen.hh"
#include "EventHandler.hh"

#include "main.h"
#include "resource.h"
#include "wminterface.h"
#include "Netwm.hh"

#include <list>

class Resource;
class WMInterface;

struct GEOM {
  unsigned int height;
  unsigned int width;
  int x;
  int y;
};

class PagerWindow 
{
	
public:
	PagerWindow(ToolWindow *toolwindow, Window _win);		
	~PagerWindow(void);
  
	Window window(void) { return pwin; }
	Window realWindow(void) { return win; }
	
	int initWindowGeometry(void);
	void reconfigure(void);
	void buildWindow(bool reconfigure);

	void setFocus(void);
	void clearFocus(void);

	int desktopId(void) { return desktop_id; }
    void setDesktopId(int id) { desktop_id = id; }

    bool isSticky(void) { return(sticky); }
    void setSticky(bool val) { sticky = val; }

    bool isFocused(void) { return(focused); }
    bool isShaded(void) { return(shaded); }

    int x(void) { return pager_x; }
    int y(void) { return pager_y; }
    void x(int val) { pager_x = val; }
    void y(int val) { pager_y = val; }

    void width(int w) { pager_width = w; }
    void height(int h) { pager_height = h; }
    int width(void) { return(pager_width); }
    int height(void) { return(pager_height); }

    bool isMarked(void) { bool tmp = marked; marked = false; return(tmp); }
    void mark(bool val) { marked = val; }
   
private:
    ToolWindow *bbtool;
    int screen;
    Resource *resource;
    ::Display *display;
	Window win;
	Window pwin;

	Pixmap pixmap;
	Pixmap pixmap_focused;

	int window_x;
	int window_y;
	unsigned int window_width;
	unsigned int window_height;

	int desktop_id;
	
	bool icon;
	bool focused;
	bool shaded;
    bool marked;

	int pager_x;
	int pager_y;
	int pager_width;
	int pager_height;
	int desktop_nr;
	bool sticky;
};

class DesktopWindow : public bt::EventHandler
{
public:
	DesktopWindow(ToolWindow *toolwindow);
	~DesktopWindow(void);

	Window window(void) { return win; }

	int desktopId(void) { return desktop_id; }

	void reconfigure(void);
	void buildWindow(bool reconfigure);

	void setFocus(void);
	void clearFocus(void);

    int x(void) { return(_x); }
    int y(void) { return(_y); }
    int width(void) { return(_width); }
    int height(void) { return(_height); }
	
    virtual void buttonPressEvent(const XButtonEvent * const event);
    virtual void buttonReleaseEvent(const XButtonEvent * const event);
    virtual void motionNotifyEvent(const XMotionEvent * const event);
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
    Resource *resource;
	Pixmap pixmap;
	Pixmap pixmap_focused;
	
    ToolWindow *bbtool;
	int desktop_id;
	int _x;
	int _y;
	int _width;
	int _height;
};


class FrameWindow : public bt::EventHandler
{
public:
	FrameWindow(ToolWindow *toolwindow);
	~FrameWindow(void);

	Window window(void) { return win; }
    void resize(void);
    void buildWindow(bool reconfigure);
    int x(void) { return(fx); }
    int y(void) { return(fy); }
    int desktopX(void) { return(ldx); }
    int desktopY(void) { return(ldy); }
    void x(int val) { fx = val; }
    void y(int val) { fy = val; }
    void setXY(int x, int y) { fx = x; fy = y; }
    void addSticky(PagerWindow *tmp);
    void removeSticky(Window win,int keep_on_desktop);

    // message handlers
    virtual void buttonPressEvent(const XButtonEvent * const event);
    virtual void configureNotifyEvent(const XConfigureEvent * const event);
    virtual void clientMessageEvent(const XClientMessageEvent * const event);

private:
	Window win;
    ToolWindow *bbtool;
	Pixmap pixmap;
    int screen;
    ::Display *display;
    int fx;
    int fy;
    int ldx;
    int ldy;
    unsigned int fwidth;
    unsigned int fheight;

    int current_column;
    int current_row;

    
    void calcSize(void);
    void calcDesktopPosition(void);

	bool lower;

};

class ToolWindow : public bt::Application {

public:
	ToolWindow(Configuration options);
	~ToolWindow(void);
 
    const bt::ScreenInfo *getCurrentScreenInfo(void) { return(&current_screen_info); }
	Resource *resource;
	int desktop_nr;

	std::list<PagerWindow> pagerWindowList(void) { return(pager_window_list); }
	std::list<DesktopWindow> desktopWindowList(void) { return(desktop_window_list); }

	void MakeWindow(bool);
	void addDesktopWindow(DesktopWindow *, bool);
	void addFrameWindow(PagerWindow *, Window, bool);
	void removeDesktopWindow(void);
	void reconfigure(void);
	int getDesktop(Window);
	int getWindowGeometry(PagerWindow *);
	void changeDesktop(int);
	void removeWindow(Window);
	void changeWindow(Window);
	void raiseWindow(Window);
	void lowerWindow(Window);
	void focusWindow(Window);
	void desktopChange(int );
    DesktopWindow *findDesktopWindow(int desktop_nr);
    DesktopWindow *findDesktopWindow(Window win);
	int winOnDesktop(Window);
	bool isIcon(Window);
  
	void changeWinDesktop(Window,int);
    void moveWinToDesktop(Window win, DesktopWindow *desktop);
    PagerWindow *findPagerWindow(Window win, std::list<PagerWindow>::iterator return_it = NULL );

	int getCurrentDesktopNr(void) { return(current_desktop_nr); }
	int getNumberOfDesktops(void) { return(number_of_desktops); }
	void setNumberOfDesktops(int n) { number_of_desktops=n; }

	void setBlackboxInit(void) { wm_init = true; }
//	struct PIXMAP getPixmap(void) { return pixmap; }
	Resource *getResource(void) { return resource; }
	int getCurrentScreen(void) { return current_screen; }
    FrameWindow *frameWindow(void) { return frame_window; }

    Configuration &configuration(void) { return(config); }


    int numberOfDesktops(void) { return(number_of_desktops); }
    Atom wmDeleteWindow(void) { return(wm_delete_window); }
    void addDesktopWindow(void);
   
	WMInterface *wminterface;
    
    virtual void shutdown(void);
    bt::Netwm *netwm(void) { return _netwm; }
   
//protected:
//	virtual void process_event(XEvent *);
  
private:

	std::list<PagerWindow> pager_window_list;
	std::list<DesktopWindow> desktop_window_list;

	bool wm_init;
	int number_of_desktops;
	int current_desktop_nr;
//	PIXMAP  pixmap;
//	GEOM frame;
//	GEOM label;
//	GEOM lbutton;
//	GEOM rbutton;
//	fd_set rfds;
	char **iargv;
	int iargc;
	int row_last,column_last;
	
    bt::Netwm *_netwm;
    PagerWindow *focuswin;
  
    Atom wm_delete_window;

	const bt::ScreenInfo &current_screen_info;
	int current_screen;

    FrameWindow *frame_window;
    Configuration &config;
};

#endif /* __BBPAGER_HH */
