// resource.cc for bbpager - a pager tool for Blackbox
//
//  Copyright (c) 1998-2000 John Kennis, jkennis@chello.nl
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

#include "resource.hh"
#include "blackboxstyle.hh"

Resource::Resource(ToolWindow *toolwindow): BaseResource(toolwindow) {
  Load();
}

Resource::~Resource() {
  Clean();
}

void Resource::Clean() {}



void Resource::LoadBBToolResource(void) {
  XrmValue value;
  char *value_type;
  unsigned int button = 0;
  WHICH_BUTTON move_default = LEFT_BUTTON;
  if (XrmGetResource(resource_db, "bbpager.desktopChangeButton",
                     "Bbpager.Desktopchangebutton", &value_type, &value)) {
    if (sscanf(value.addr, "%u", &button) != 1 || (button > 5 || button < 1))
      desktop_change_button = MIDDLE_BUTTON;
    else {
      desktop_change_button = (WHICH_BUTTON)button;
	  if (desktop_change_button == LEFT_BUTTON)
        move_default = MIDDLE_BUTTON; // new default to avoid clashes
      button = 0;
    }
  } else
      desktop_change_button = MIDDLE_BUTTON;
  
  if (XrmGetResource(resource_db, "bbpager.windowMoveButton",
                     "Bbpager.WindowMovebutton", &value_type, &value)) {
    if (sscanf(value.addr, "%u", &button) != 1 || (button > 5 || button < 1))
      window_move_button = move_default;
    else {
        window_move_button = (WHICH_BUTTON)button;
    }
  } else
      window_move_button = move_default;

  if (XrmGetResource(resource_db, "bbpager.windowFocusButton",
                     "Bbpager.WindowFocusbutton", &value_type, &value)) {
    if (sscanf(value.addr, "%u", &button) != 1 || (button > 5 || button < 1))
      window_focus_button = INVALID_BUTTON;
    else {
        window_focus_button = (WHICH_BUTTON)button;
    }
  } else
      window_focus_button = INVALID_BUTTON;

  if (XrmGetResource(resource_db, "bbpager.windowRaiseButton",
                     "Bbpager.WindowRaiseButton", &value_type, &value)) {
    if (sscanf(value.addr, "%u", &button) != 1 || (button > 5 || button < 1))
      window_raise_button = INVALID_BUTTON;
    else {
        window_raise_button = (WHICH_BUTTON)button;
    }
  } else
      window_raise_button = INVALID_BUTTON;


  Frame();
  
  SizeAndPosition();

  PagerWin();
}


void Resource::Frame() {
  XrmValue value;
  char *value_type;

  readTexture("bbpager.frame","BbPager.Frame",BB_FRAME,"Toolbar",
             "slategrey","darkslategrey","Raised Gradient Vertical Bevel1",
             &frame.texture);

  if (XrmGetResource(resource_db, "bbpager.bevelWidth","Bbpager.BevelWidth",
                     &value_type, &value)) {
    if (sscanf(value.addr, "%u", &frame.bevelWidth) != 1)
      frame.bevelWidth = 4;
    else if (frame.bevelWidth == 0)
      frame.bevelWidth = 4;
  } else if (XrmGetResource(resource_db, BB_BEVELWIDTH,"BevelWidth", &value_type,
                            &value)) {
    if (sscanf(value.addr, "%u", &frame.bevelWidth) != 1)
      frame.bevelWidth = 4;
    else if (frame.bevelWidth == 0)
      frame.bevelWidth = 4;
  } else
    frame.bevelWidth = 4;
}


void Resource::SizeAndPosition() {
  XrmValue value;
  char *value_type;
  unsigned int w,h;
  char positionstring[11];

  if (!(bbtool->position)) {
    if (!(XrmGetResource(resource_db, "bbpager.position","Bbpager.Position",
                         &value_type, &value)))
      strncpy(positionstring, "-0-0", 5);
    else
      strncpy(positionstring, value.addr, strlen(value.addr)+1);
  } else
    strncpy(positionstring, bbtool->position, strlen(bbtool->position)+1);


  position.mask=XParseGeometry(positionstring, &position.x, &position.y, 
                              &w, &h);

  if (!(position.mask & XValue))
    position.x=0;
  if (!(position.mask & YValue))
    position.y=0;


  if (XrmGetResource(resource_db, "bbpager.columns",
                       "Bbpager.Columns",
                       &value_type, &value)) {
     if (sscanf(value.addr, "%u", &columns) != 1)
       columns=1;
     else {
       position.vertical=True;
       if (columns==0) columns=1;
     }
  }
  else
    columns=1;

  if (XrmGetResource(resource_db, "bbpager.rows","Bbpager.Rows",
                                         &value_type, &value)) {
    if (sscanf(value.addr, "%u", &rows) != 1)
      rows=1;
    else {
      position.horizontal=True;
      if (rows==0) rows=1;
    }
  }
  else
    rows=1;
  
  if (!position.horizontal && !position.vertical) {
    if (bbtool->withdrawn)
        position.vertical=True;
    else
      position.horizontal=True;
  }

  if (!(XrmGetResource(resource_db, "bbpager.desktop.width",
                       "Bbpager.Desktop.Width",
                       &value_type, &value))) {
    if (!bbtool->withdrawn)
      desktopSize.width=40;
    else
      desktopSize.width=64/columns - ((columns-1))*frame.bevelWidth;
  }
  else
    if (sscanf(value.addr, "%u", &desktopSize.width) != 1) {
      if (!bbtool->withdrawn)
        desktopSize.width=40;
      else
        desktopSize.width=64/columns - ((columns-1))*frame.bevelWidth;
 
    }

  if (!(XrmGetResource(resource_db, "bbpager.desktop.height",
                       "Bbpager.Desktop.Height",
                       &value_type, &value))) {
    if (!bbtool->withdrawn)
      desktopSize.height=30;
    else
      desktopSize.height=48/columns;
  } 
  else
    if (sscanf(value.addr, "%u", &desktopSize.height) != 1) {
      if (!bbtool->withdrawn)
        desktopSize.width=30;
      else
        desktopSize.height=48/columns;
    }  
}

void Resource::PagerWin() {
  XrmValue value;
  char *value_type;

  if (XrmGetResource(resource_db, "bbpager.desktop.focusStyle",
                     "Bbpager.Desktop.FocusStyle", &value_type, &value)) {
    if (! strncasecmp("texture", value.addr, value.size))
      desktop_focus_style = texture;
    else if (! strncasecmp("none", value.addr, value.size)) 
      desktop_focus_style = none;
    else 
      desktop_focus_style = border;
  } else
    desktop_focus_style = border;

  
  readTexture("bbpager.desktop", "Bbpager.Desktop",
              BB_LABEL,"Toolbar.Label",
              "slategrey","darkslategrey",
              "Sunken Gradient Diagonal Bevel1",&desktopwin.texture);

  if (desktop_focus_style==texture)
    readTexture("bbpager.desktop.focus", "Bbpager.Desktop.Focus",
                0,0,
                "darkslategrey","slategrey",
                "Sunken Gradient Diagonal Bevel1",&desktopwin.focusedTexture);

  if (XrmGetResource(resource_db, "bbpager.window.focusStyle",
                     "Bbpager.Window.FocusStyle", &value_type, &value)) {
    if (! strncasecmp("border", value.addr, value.size))
      focus_style = border;
    else if (! strncasecmp("none", value.addr, value.size)) 
      focus_style = none;
    else 
      focus_style = texture;
  } else
    focus_style = texture;

  readTexture("bbpager.window", "Bbpager.Window",
              BB_WINDOW_UNFOCUS,"Window.Unfocus",
              "rgb:c/9/6","rgb:8/6/4",
              "Raised Gradient Diagonal Bevel1",&pagerwin.texture);

  if (focus_style==texture)
    readTexture("bbpager.window.focus","Bbpager.Window.Focus",
                      BB_WINDOW_FOCUS,"Window.Focus", "rgb:c/9/6","rgb:8/6/4",
                      "Raised Vertical Gradient Bevel1",
                      &pagerwin.focusedTexture);
  
  readColor("bbpager.active.window.borderColor",
            "Bbpager.active.Window.BorderColor",
             0,0,"LightGrey",&pagerwin.activeColor);

  readColor("bbpager.inactive.window.borderColor",
            "Bbpager.inactive.Window.BorderColor",
            0,0,"black",&pagerwin.inactiveColor);

  readColor("bbpager.active.desktop.borderColor",
            "Bbpager.Active.desktop.BorderColor",
            0,0,"LightGrey",&desktopwin.activeColor);

}
