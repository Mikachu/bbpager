// resource.hh for bbpager - a pager tool for Blackbox
//
//  Copyright (c) 1998-2000 John Kennis, jkennis@chello.nl
//
//  this program is free software; you can redistribute it and/or modify
//  it under the terms of the gnu general public license as published by
//  the free software foundation; either version 2 of the license, or
//  (at your option) any later version.
//
//  this program is distributed in the hope that it will be useful,
//  but without any warranty; without even the implied warranty of
//  merchantability or fitness for a particular purpose.  see the
//  gnu general public license for more details.
//
//  you should have received a copy of the gnu general public license
//  along with this program; if not, write to the free software
//  foundation, inc., 675 mass ave, cambridge, ma 02139, usa.
//
// (see the included file copying / gpl-2.0)
//


#ifndef __RESOURCE_HH
#define __RESOURCE_HH

#include "bbpager.h"
#include "Baseresource.h"

class BaseResource;

enum {none=0, border=1, texture=2};

struct FRAME {
    int width;
    int height;
    bt::Texture texture;
    int bevelWidth;
    XFontStruct *font;
    int desktopMargin;
};

struct POSITION {
  int x;
  int y;
  int mask;
  bool vertical;
  bool horizontal;
};

struct SIZE {
  unsigned int width;
  unsigned int height;
};

struct REPORT {
  bool auto_raise;
};

struct BBPAGERWIN {
  bt::Texture texture;
  bt::Texture focusedTexture;
  bt::Color activeColor;
  int activeWidth;
  bt::Color inactiveColor;
  int inactiveWidth;
};

class ToolWindow;

class Resource : public BaseResource {

public:
  Resource(ToolWindow *toolwindow, const std::string &blackbox_rc_file, const std::string &rc_file);
  ~Resource(void);

  struct FRAME frame;
  struct POSITION position;
  struct SIZE desktopSize;
  struct REPORT report;
  struct BBPAGERWIN pagerwin;
  struct BBPAGERWIN desktopwin;
  int columns;
  int rows;
  int getFocusStyle(void) { return pager_focus_style; }
  int getDesktopFocusStyle(void) { return desktop_focus_style; }
  unsigned int getWindowMoveButton(void) { return window_move_button; }
  unsigned int getDesktopChangeButton(void) { return desktop_change_button; }
  unsigned int getWindowRaiseButton(void) { return window_raise_button; }
  unsigned int getWindowFocusButton(void) { return window_focus_button; }


    void clean(void);
    void load(void); 


private:
  void Frame(void);
  void SizeAndPosition(void);
  void PagerWin(void);
  int pager_focus_style;
  int desktop_focus_style;
  unsigned int window_move_button;
  unsigned int desktop_change_button;
  unsigned int window_raise_button;
  unsigned int window_focus_button;
  void Clean(void);

  ToolWindow *bbtool;
};
#endif /* __RESOURCE_HH */
