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

#include "Image.hh"
#include "Basewindow.hh"
#include "resource.hh"
#include "wminterface.hh"

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
	PagerWindow(Window _win);		
	~PagerWindow(void);
  
	Window window(void) { return win; }
	Window pagerWindow(void) { return pwin; }
	
	int initWindowGeometry(void);
	void reconfigure(void);
	void buildWindow(bool reconfigure);

	void setFocus(void);
	void clearFocus(void);

	int desktopId(void) { return desktop_id; }


private:
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

	int pager_x;
	int pager_y;
	int pager_width;
	int pager_height;
	int desktop_nr;
	bool sticky;
};

class DesktopWindow
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
	
private:
	Window win;

	Pixmap pixmap;
	Pixmap pixmap_focused;
	
	int desktop_id;
	int x;
	int y;
	int width;
	int height;
};


class FrameWindow
{
public:
	FrameWindow(ToolWindow *toolwindow);
	~FrameWindow(void);

	Window window(void) { return win; }


private:
	Window win;

	Pixmap pixmap;
};

class ToolWindow : public bt::Application {

public:
	ToolWindow(Configuration options);
	~ToolWindow(void);

	Resource *resource;
	int desktop_nr;

	std::list<PagerWindow> pagerWindowList(void) { return pager_window_list; }
	std::list<DesktopWindow> desktopWindowList(void) { return desktop_window_list; }

	void MakeWindow(bool);
	void addDesktopWindow(DesktopWindow *, bool);
	void addFrameWindow(PagerWindow *, Window, bool);
	void removeDesktopWindow(void);
	void reconfigure(void);
	int getDesktop(Window);
	int getWindowGeometry(struct WindowList *);
	void changeDesktop(int);
	void removeWindow(Window);
	void changeWindow(Window);
	void raiseWindow(Window);
	void lowerWindow(Window);
	void focusWindow(Window);
	void desktopChange(int );
	int winOnDesktop(Window);
	bool isIcon(Window);
	void changeWinDesktop(Window,int);
	void moveWinToDesktop(Window, DesktopList *);
	int getCurrentDesktopNr(void) { return current_desktop_nr; }
	int getNumberOfDesktops(void) { return number_of_desktops; }
	void setNumberOfDesktops(int n) { number_of_desktops=n; }

	void setBlackboxInit(void) { wm_init = true; }
//	struct PIXMAP getPixmap(void) { return pixmap; }
	Resource *getResource(void) { return resource; }

protected:
	virtual void process_event(XEvent *);
  
private:

	std::list<PagerWindow> pager_window_list;
	std::list<DesktopWindow> desktop_window_list;

	bool lower;
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
  
	PagerWindow &focuswin;
  
	WMInterface *wminterface;
};

#endif /* __BBPAGER_HH */
