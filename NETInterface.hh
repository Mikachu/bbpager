//  BlackboxInterface.hh for bbtools.
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

#ifndef __BlackboxINTERFACE_HH
#define __BlackboxINTERFACE_HH

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/Xresource.h>

#include "Timer.hh"
#include "Basewindow.hh"

class Basewindow;

class BlackboxInterface : public TimeoutHandler {
  public:
    BlackboxInterface(Basewindow *);
    virtual ~BlackboxInterface(void) {};
    void handleBlackboxEvents(XEvent);

  protected:
    virtual void BlackboxNotifyStartup(void) {net_init=True;}
    virtual void BlackboxNotifyWindowAdd(Window,int)=0;
    virtual void BlackboxNotifyDel(Window)=0;
    virtual void BlackboxNotifyAttributes(Window)=0;
    virtual void BlackboxNotifyFocus(Window)=0;
    virtual void BlackboxNotifyCurrentWorkspace(int)=0;
    virtual void BlackboxNotifyWorkspaceCount(int)=0;
    virtual void BlackboxNotifyWindowRaise(Window)=0;
    virtual void BlackboxNotifyWindowLower(Window)=0;
    virtual void timeout(void);

  private:
    Basewindow *base;
    bool net_init;
    BTimer *timer;
};


#endif // __BlackboxINTERFACE_HH
