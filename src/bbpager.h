// bbpager.h for bbpager - an pager tool for Blackbox.
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


#ifndef __BBPAGER_H
#define __BBPAGER_H

#include <X11/Xutil.h>
#include <X11/Xatom.h>

// Blackbox library includes
#include "Display.hh"
#include "Application.hh"
#include "Texture.hh"
#include "PixmapCache.hh"
#include "EventHandler.hh"
#include "Netwm.hh"

//bbpager includes
#include "main.h"
#include "resource.h"
#include "wminterface.h"
#include "pager.h"
#include "desktop.h"


#include <list>

class Resource;
class WMInterface;
class DesktopWindow;

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
    void x(int val) { fx = val; }
    void y(int val) { fy = val; }
    void setXY(int x, int y) { fx = x; fy = y; }

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
    unsigned int current_row;
	bool lower;
    
    void calcSize(void);


};

class ToolWindow : public bt::Application {

public:
	ToolWindow(Configuration options);
	~ToolWindow(void);
 
    const bt::ScreenInfo *getCurrentScreenInfo(void) { return(&current_screen_info); }
	Resource *resource;
	int desktop_nr;

	std::list<PagerWindow *> &pagerWindowList(void) { return(pager_window_list); }
	std::list<DesktopWindow *> &desktopWindowList(void) { return(desktop_window_list); }

	void MakeWindow(bool);
	void addDesktopWindow(DesktopWindow *, bool);
	void addFrameWindow(PagerWindow *, Window, bool);
	void removeDesktopWindow(void);
	void reconfigure(void);
	int getDesktop(Window);
	int getWindowGeometry(PagerWindow *);
	void changeDesktop(int);
	void changeWindow(Window);
	void raiseWindow(Window);
	void lowerWindow(Window);
	void focusWindow(Window);
	void desktopChange(unsigned int desktop_nr);
    DesktopWindow *findDesktopWindow(unsigned int desktop_nr);
    DesktopWindow *findDesktopWindow(Window win);
	int winOnDesktop(Window);
	bool isIcon(Window);
  
	void changeWinDesktop(Window,int);
    void moveWinToDesktop(Window win, DesktopWindow *desktop);
    void moveWinToDesktop(PagerWindow *pager_window, unsigned int desktop_nr);

    PagerWindow *findPagerWindow(Window win);
    PagerWindow *findPPagerWindow(Window win);
    PagerWindow *findFocusedPagerWindow();

	int getCurrentDesktopNr(void) { return(current_desktop_nr); }
	int getNumberOfDesktops(void) { return(number_of_desktops); }
	void setNumberOfDesktops(int n) { number_of_desktops=n; }

	void setBlackboxInit(void) { wm_init = true; }
	Resource *getResource(void) { return resource; }
	int getCurrentScreen(void) { return current_screen; }
    FrameWindow *frameWindow(void) { return frame_window; }

    Configuration &configuration(void) { return(config); }


    unsigned int numberOfDesktops(void) { return(number_of_desktops); }
    Atom &wmDeleteWindowAtom(void) { return(xa_wm_delete_window); }
    Atom &wmStateAtom(void) { return(xa_wm_state); }

    void addDesktopWindow(unsigned int nr);
   
	WMInterface *wminterface;
    
    virtual void shutdown(void);
    bt::Netwm *netwm(void) { return _netwm; }
   
    Window root_window;
	std::list<PagerWindow *> pager_window_list;
private:

	std::list<DesktopWindow *> desktop_window_list;

	bool wm_init;
	unsigned int number_of_desktops;
	unsigned int current_desktop_nr;
	char **iargv;
	int iargc;
	int row_last,column_last;
	
    bt::Netwm *_netwm;
    
    Atom xa_wm_delete_window;
    Atom xa_wm_state;
	const bt::ScreenInfo &current_screen_info;
	int current_screen;

    FrameWindow *frame_window;
    Configuration &config;
};

#endif // __BBPAGER_H

