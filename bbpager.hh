// bbpager.hh for bbpager - an pager tool for Blackbox.
//
//  Copyright (c) 1998-2000 by John Kennis, jkennis@chello.nl
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


#ifndef __MAIN_HH
#define __MAIN_HH

#include "Image.hh"
#include "Basewindow.hh"
#include "resource.hh"
#include "wminterface.hh"

class Resource;
class BaseResource;
class Basewindow;
class WMInterface;

struct PIXMAP {
  Pixmap 	frame;
  Pixmap	desktop;
  Pixmap  focusedDesktop;
  Pixmap  window;
  Pixmap  focusedWindow;
};

struct GEOM {
  unsigned int height;
  unsigned int width;
  int x;
  int y;
};

struct WindowList {
  Window win;
  int x_position;
  int y_position;
  unsigned int width;
  unsigned int height;
  bool icon;
  bool focused;
  bool shaded;
  Window pager_win;
  int pager_x;
  int pager_y;
  int pager_width;
  int pager_height;
  int desktop_nr;
  bool sticky;
};

struct DesktopList {
    Window win;
    int desktop_nr;
    int x;
    int y;
    int width;
    int height;
  };


class ToolWindow : public Basewindow {

public:
  ToolWindow(int argc,char **argv,struct CMDOPTIONS *);
  ~ToolWindow(void);

  XGCValues gcv;
  GC frameGC;

  Window framewin;
  Resource *resource;
  int desktop_nr;

  LinkedList<WindowList> *windowList;
  LinkedList<DesktopList> *desktopList;

  void MakeWindow(bool);
  void addDesktopWindow(struct DesktopList *,bool);
  void addFrameWindow(struct WindowList *,Window,bool);
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
  void moveWinToDesktop(Window,DesktopList *);
  int getCurrentDesktopNr(void) { return current_desktop_nr; }
  int getNumberOfDesktops(void) { return number_of_desktops; }
  void setNumberOfDesktops(int n) { number_of_desktops=n; }

  void setBlackboxInit(void) { wm_init = True; }
  struct PIXMAP getPixmap(void) { return pixmap; }
  Resource *getResource(void) { return resource; }

protected:
  virtual void process_event(XEvent *);
  
private:

  bool lower;
  bool wm_init;
  int day,month,year;
  int number_of_desktops;
  int current_desktop_nr;
  PIXMAP  pixmap;
  GEOM frame;
  GEOM label;
  GEOM lbutton;
  GEOM rbutton;
  fd_set rfds;
  char **iargv;
  int iargc;
  int row_last,column_last;
  
  Window focuswin;
  
  WMInterface *wminterface;

void CheckConfig(void);

};

#endif /* __MAIN_HH */
