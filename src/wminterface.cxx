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

#include <stdio.h>

WMInterface::WMInterface(ToolWindow *toolwindow) :
    bt::EventHandler(), bbtool(toolwindow)
{
    root_window = bbtool->getCurrentScreenInfo()->rootWindow();
    XSelectInput(bbtool->XDisplay(), root_window, PropertyChangeMask);
    bbtool->insertEventHandler(root_window, this);
    ewmh = bbtool->ewmh();
}

WMInterface::~WMInterface()
{
}

void WMInterface::sendClientMessage(Window window, Atom atom, XID data, XID data1)
{
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

#if 0
void WMInterface::updateWindowList(void)
{
    bt::EWMH::WindowList window_vect;
    PagerWindow *pwindow;

    if (ewmh->readClientList(bbtool->getCurrentScreenInfo()->rootWindow(), window_vect)) {
        /* delete any windows not in list */
        std::list<PagerWindow *>::iterator pit = bbtool->pagerWindowList().begin();

        //bbtool->pagerWindowList().clear();
        for (; pit != bbtool->pagerWindowList().end(); ) {
            delete (*pit);
            pit = bbtool->pagerWindowList().erase(pit);
        }
        /* add any new window windows */
        bt::EWMH::WindowList::iterator it = window_vect.begin();
        for (; it != window_vect.end(); it++) {
            // skip myself
            if ((*it) == bbtool->frameWindow()->window())
            {
                continue;
            }
            PagerWindow *pager_window = new PagerWindow(bbtool, *it);
            bbtool->pagerWindowList().push_back(pager_window);
        }

    }
}
#else

void WMInterface::updateWindowList(void)
{
    bt::EWMH::WindowList window_vect;
    PagerWindow *pwindow;

    if (ewmh->readClientList(bbtool->getCurrentScreenInfo()->rootWindow(), window_vect)) {
        /* add any new window windows */
        bt::EWMH::WindowList::iterator it = window_vect.begin();
        bt::EWMH::WindowList::iterator it_end = window_vect.end();
        for (; it != it_end; it++) {
            // skip myself
            if ((*it) == bbtool->frameWindow()->window())
            {
                continue;
            }
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

        for (; pit != pit_end; ) {
            if (!(*pit)->isMarked()) {
                delete (*pit);
                pit = bbtool->pagerWindowList().erase(pit);
            } else {
                pit++;
            }
        }

    }
}
#endif

void WMInterface::updateWindowStack()
{
    bt::EWMH::WindowList window_vect;
    if (ewmh->readClientListStacking(bbtool->getCurrentScreenInfo()->rootWindow(), window_vect)) {
        // some check to see if we need update
        PagerWindow *pwindow;
        bt::EWMH::WindowList::iterator it = window_vect.begin();
        bt::EWMH::WindowList::iterator it_end = window_vect.end();
        for (; it != it_end; it++) {
            pwindow = bbtool->findPagerWindow((*it));
            if ( pwindow == NULL) continue;
            pwindow->raise();
        }
    }
}

void WMInterface::changeDesktop(int desk_number)
{

    sendClientMessage(root_window, ewmh->currentDesktop(), desk_number);
}

void WMInterface::sendWindowToDesktop(Window win,int desk_number)
{
    sendClientMessage(win, ewmh->wmDesktop(), desk_number);
}

void WMInterface::setWindowFocus(Window win, Time time)
{
    //ewmh->setActiveWindow(root_window, win);
    sendClientMessage(win, ewmh->activeWindow(), 2, time);
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
    int i;
    bbtool->setNumberOfDesktops(number_of_desktops);
    if (number_of_desktops > old_number_of_desktops) {
        for (i=old_number_of_desktops;i<number_of_desktops;i++)
            bbtool->addDesktopWindow(i);
    } else if (number_of_desktops<old_number_of_desktops) {
        for (i = number_of_desktops; i < old_number_of_desktops; i++)
            bbtool->removeDesktopWindow();
    }
}

bool WMInterface::readActiveWindow(Window target, Window *active)
{
    unsigned char* data = NULL;
    if (ewmh->getProperty(target, XA_WINDOW, ewmh->activeWindow(), &data)) {
        *active = * (reinterpret_cast<Window *>(data));
        XFree(data);
        return True;
    }
    return False;
}

//property notify events, send to root window.
void WMInterface::propertyNotifyEvent(const XPropertyEvent * const event)
{
    if (event->atom == ewmh->clientList()) {
        updateWindowList();
    } else if (event->atom == ewmh->clientListStacking()) {
        updateWindowStack();
    } else if (event->atom ==  ewmh->numberOfDesktops()) {
        unsigned int number;
        if (ewmh->readNumberOfDesktops(root_window, &number))
            changeNumberOfDesktops(number);
    } else if (event->atom == ewmh->desktopGeometry()) {

    } else if (event->atom == ewmh->currentDesktop()) {
        unsigned int current_desktop;
        if (!ewmh->readCurrentDesktop(root_window, &current_desktop))
            fprintf(stderr, "Error: Cannot read current desktop\n");
        else {
            bbtool->desktopChange(current_desktop);
        }
    } else if (event->atom == ewmh->desktopNames()) {

    } else if (event->atom == ewmh->activeWindow()) {
        Window active;
        if (!readActiveWindow(root_window, &active)) {
            fprintf(stderr, "Error: Cannot read active window\n");
        } else {
            bbtool->focusWindow(active);
        }
    } else if (event->atom == ewmh->workarea()) {

    } else {
        // ignore
    }



    //wminterface->windowAttributeChange(event->xproperty.window);
}


