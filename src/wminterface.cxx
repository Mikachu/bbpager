//  wminterface.cc for bbtools.
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

#include "wminterface.h"
#include "resource.h"

WMInterface::WMInterface(ToolWindow *toolwindow) : 
    bt::EventHandler(), bbtool(toolwindow)
{
    root_window = bbtool->getCurrentScreenInfo()->rootWindow();
    XSelectInput(bbtool->XDisplay(), root_window, PropertyChangeMask); 
    bbtool->insertEventHandler(root_window, this);
    netwm = bbtool->netwm();
}

WMInterface::~WMInterface() 
{
}

void WMInterface::sendClientMessage(Window window, Atom atom, XID data, XID data1) {
  XEvent e;
  unsigned long mask;

  e.xclient.type = ClientMessage;
  e.xclient.window = window; 
  e.xclient.message_type = atom;
  e.xclient.format = 32;
  e.xclient.data.l[0] = (unsigned long) data;
  e.xclient.data.l[1] = (unsigned long) data1;
  mask =  SubstructureRedirectMask;
  XSendEvent(bbtool->XDisplay(), 
             root_window,
             False, mask, &e);
}


void WMInterface::updateWindowList(void)
{
    bt::Netwm::WindowList window_vect;
    PagerWindow *pwindow;
  
    if (netwm->readClientList(bbtool->getCurrentScreenInfo()->rootWindow(), window_vect)) {
        /* add any new window windows */
        bt::Netwm::WindowList::iterator it = window_vect.begin();
        bt::Netwm::WindowList::iterator it_end = window_vect.end();
        for (; it != it_end; it++) {
            pwindow = bbtool->findPagerWindow((*it));
            if ( pwindow == NULL) { 
                PagerWindow *pager_window = new PagerWindow(bbtool, *it);
                bbtool->pagerWindowList().push_back(pager_window);
            } else {
                pwindow->mark(true);
            }
        }
        /* delete any windows not in list */
        std::list<PagerWindow *>::iterator pit = bbtool->pagerWindowList().begin();
        std::list<PagerWindow *>::iterator pit_end = bbtool->pagerWindowList().end();
   
        for (; pit != pit_end; pit++) {
            if (!(*pit)->isMarked()) {
                delete (*pit);
                bbtool->pagerWindowList().erase(pit);
                pit--;
            } else {
            }
        }

    }
}

void WMInterface::updateWindowStack() 
{
    bt::Netwm::WindowList window_vect;
    if (netwm->readClientListStacking(bbtool->getCurrentScreenInfo()->rootWindow(), window_vect)) {
        // some check to see if we need update
        PagerWindow *pwindow;
        bt::Netwm::WindowList::reverse_iterator it = window_vect.rbegin();
        bt::Netwm::WindowList::reverse_iterator it_end = window_vect.rend();
        for (; it != it_end; it++) {
            pwindow = bbtool->findPagerWindow((*it));
            if ( pwindow == NULL) continue;
            pwindow->raise();
        }    
    }
}


void WMInterface::changeDesktop(int desk_number) 
{

    sendClientMessage(root_window, netwm->currentDesktop(), desk_number);
}


void WMInterface::sendWindowToDesktop(Window win,int desk_number) 
{
    sendClientMessage(win, netwm->wmDesktop(), desk_number);
}

void WMInterface::setWindowFocus(Window win) 
{
    //netwm->setActiveWindow(root_window, win);
    sendClientMessage(win, netwm->activeWindow(), win);

}

int WMInterface::isIconicState(Window win) 
{
    Atom real_type;
    int format;
    unsigned long n, extra;
    int status;
    long *p=0;
    int result = -1;

    status = XGetWindowProperty(bbtool->XDisplay(), win, 
                              bbtool->wmStateAtom(), 0L, 1L,
                              False, bbtool->wmStateAtom(), &real_type,
                              &format, &n, &extra,  (unsigned char**)&p);
    if (!status) {
        if (p) {
            result = (p[0]==IconicState) ? 1 : 0;
            XFree(p);
        } else 
            result = 0;
    }

    return(result);
}

void WMInterface::changeNumberOfDesktops(int number_of_desktops) 
{
  int old_number_of_desktops = bbtool->getNumberOfDesktops();
  bbtool->setNumberOfDesktops(number_of_desktops);
  if (number_of_desktops > old_number_of_desktops) {
  int i;
  for (i=old_number_of_desktops;i<number_of_desktops;i++) {
    bbtool->addDesktopWindow(i);
    }
  } else if (number_of_desktops<old_number_of_desktops)
    bbtool->removeDesktopWindow();
}


bool WMInterface::readActiveWindow(Window target, Window *active)
{
  unsigned char* data = NULL;
  if (netwm->getProperty(target, XA_WINDOW, netwm->activeWindow(), &data)) {
    *active = * (reinterpret_cast<Window *>(data));
    XFree(data);
    return True;
  }
  return False;
}

//property notify events, dens to root window.
void WMInterface::propertyNotifyEvent(const XPropertyEvent * const event)
{
     if (event->atom == netwm->clientList()) {
        updateWindowList();
    } else if (event->atom == netwm->clientListStacking()) {
        updateWindowStack();
    } else if (event->atom ==  netwm->numberOfDesktops()) {
        unsigned int number;
        netwm->readNumberOfDesktops(root_window, &number);
        changeNumberOfDesktops(number);
    } else if (event->atom == netwm->desktopGeometry()) {

    } else if (event->atom == netwm->currentDesktop()) {
        unsigned int current_desktop;
        netwm->readCurrentDesktop(root_window, &current_desktop);
        bbtool->desktopChange(current_desktop);
    } else if (event->atom == netwm->desktopNames()) {

    } else if (event->atom == netwm->activeWindow()) {
        Window active;
        if (!readActiveWindow(root_window, &active)) {
            printf("error cannot read active window\n");
        }
        bbtool->focusWindow(active);
    } else if (event->atom == netwm->workarea()) {

    } else {
        // ignore
    }

   
    
    //wminterface->windowAttributeChange(event->xproperty.window);
}


