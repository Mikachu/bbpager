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

#include "bbpager.h"
//#include "EventHandler.hh"
#include "Netwm.hh"

class Resource;
class ToolWindow;
//class EventHandler;
struct WindowList;


class WMInterface : public bt::EventHandler 
{
    public:
        WMInterface(ToolWindow *);
        ~WMInterface(void);

        void removeSticky(Window,int);
        void addSticky(WindowList *);
        void sendWindowToDesktop(Window,int);
        void setWindowFocus(Window);
        int isIconicState(Window);
        void changeIconState(Window);
        void windowAttributeChange(Window);

        int getNumberOfDesktops(void) { unsigned int number; netwm->readNumberOfDesktops(root_window, &number); return(number); }
        int getCurrentDesktop(void);
        void changeDesktop(int);
        void updateWindowList(void);
        void updateWindowStack(void);
        
        void changeNumberOfDesktops(int number_of_desktops);
        bool readActiveWindow(Window target, Window *active);
        void processClientEvents(XEvent Event);

        virtual void propertyNotifyEvent(const XPropertyEvent * const event);

    private:
        ToolWindow *bbtool;

        Window root_window;
        bt::Netwm *netwm;

        void sendClientMessage(Window window, Atom atom, XID data);
};

#endif /* __WMINTERFACE_HH */
