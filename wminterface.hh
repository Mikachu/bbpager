//  wminterface.hh for bbtools.
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
#ifndef __WMINTERFACE_HH
#define __WMINTERFACE_HH

#include "Image.hh"
#include "bbpager.hh"
#include "NETInterface.hh"

class BImageControl;
class Resource;
class ToolWindow;
class BaseDisplay;

struct WindowList;


class WMInterface : public BlackboxInterface {

public:
  WMInterface(ToolWindow *);
  ~WMInterface(void);

  void moduleInit(void);

  void removeSticky(Window,int);
  void addSticky(WindowList *);
  void sendWindowToDesktop(Window,int);
  void setWindowFocus(Window);
  int isIconicState(Window);
  int getAttributes(Window);
  void changeIconState(Window);
  void windowAttributeChange(Window);
  
  int getNumberOfDesktops();
  int getCurrentDesktop();
  void changeDesktop(int);

  void processClientEvents(XEvent Event);

/*  Atom getKWMModuleDesktopChange(void) { return kwm_module_desktop_change; }
  Atom getKWMModuleDesktopNumberChange(void) {
                return kwm_module_desktop_number_change; }
  Atom getKWMModuleInit(void) { return kwm_module_init; }
  Atom getKWMModuleWinAdd(void) { return kwm_module_win_add; }
  Atom getKWMModuleWinRemove(void) { return kwm_module_win_remove; }
  Atom getKWMModuleWinChange(void) { return kwm_module_win_change; }
  Atom getKWMModuleWinActivate(void) { return kwm_module_win_activate; }
  Atom getKWMModuleWinRaise(void) { return kwm_module_win_raise; }
  Atom getKWMModuleWinLower(void) { return kwm_module_win_lower; }*/
 protected:
    virtual void BlackboxNotifyStartup(void);
    virtual void BlackboxNotifyWindowAdd(Window,int);
    virtual void BlackboxNotifyDel(Window);
    virtual void BlackboxNotifyAttributes(Window);
    virtual void BlackboxNotifyFocus(Window);
    virtual void BlackboxNotifyCurrentWorkspace(int);
    virtual void BlackboxNotifyWorkspaceCount(int);
    virtual void BlackboxNotifyWindowRaise(Window);
    virtual void BlackboxNotifyWindowLower(Window);

private:
  void sendClientMessage(Atom, XID);

  ToolWindow *bbtool;

//  Atom kwm_module_init;
//  Atom kwm_module_win_add;
//  Atom kwm_module_win_remove;
//  Atom kwm_module_win_change;
//  Atom kwm_module_win_activate;
//  Atom kwm_module_win_iconfied;
//  Atom kwm_module_win_raise;
//  Atom kwm_module_win_lower;
//  Atom kwm_module_desktop_change;
//  Atom kwm_module_desktop_number_change;


};

#endif /* __WMINTERFACE_HH */
