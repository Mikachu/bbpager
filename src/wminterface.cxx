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
    bbtool->insertEventHandler(bbtool->getCurrentScreenInfo()->rootWindow(), this);
    root_window = bbtool->getCurrentScreenInfo()->rootWindow();
    netwm = bbtool->netwm();
}

WMInterface::~WMInterface() 
{
}

void WMInterface::moduleInit() 
{
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
                PagerWindow pager_window(bbtool, *it);
                bbtool->pagerWindowList().push_back(pager_window);
            } else {
                pwindow->mark(true);
            }
        }

        /* delete any windows not in list */
        std::list<PagerWindow>::iterator pit = bbtool->pagerWindowList().begin();
        std::list<PagerWindow>::iterator pit_end = bbtool->pagerWindowList().end();
    
        for (; pit != pit_end; ) {
            if (!pit->isMarked()) {
//                delete *pit;
                bbtool->pagerWindowList().erase(pit);
            } else
                pit++;
        }
    }
}

void WMInterface::changeDesktop(int desk_number) 
{
    netwm->setCurrentDesktop(root_window, desk_number);
}


void WMInterface::sendWindowToDesktop(Window win,int desk_number) 
{
    netwm->setWMDesktop(root_window, desk_number);
}

void WMInterface::setWindowFocus(Window win) 
{
    netwm->setActiveWindow(root_window, win);
}

int WMInterface::isIconicState(Window win) 
{
    bt::Netwm::AtomList states;
    bt::Netwm::AtomList::iterator it;

    if (netwm->readWMState(win, states)) {
        for (it = states.begin(); it != states.end(); it++) {
            if ((*it) == netwm->wmStateHidden() || (*it) == netwm->wmStateSkipPager()) {
                return(1);
            }
        }
        return(0);
    }
    return(-1);
}


void WMInterface::focus(Window win) 
{
    if (bbtool->getResource()->getFocusStyle()!=none)
        bbtool->focusWindow(win);
}

/*void WMInterface::BlackboxNotifyWindowRaise(Window win) {
  bbtool->raiseWindow(win);
}

void WMInterface::BlackboxNotifyWindowLower(Window win) {
  bbtool->lowerWindow(win);
}*/

void WMInterface::changeNumberOfDesktops(int number_of_desktops) 
{
  int old_number_of_desktops = bbtool->getNumberOfDesktops();
  bbtool->setNumberOfDesktops(number_of_desktops);
  if (number_of_desktops > old_number_of_desktops) {
  int i;
  for (i=old_number_of_desktops;i<number_of_desktops;i++) {
    bbtool->addDesktopWindow();
/*    LinkedListIterator<WindowList> win_it(bbtool->windowList);
    for (; win_it.current(); win_it++)
      if ((win_it.current()->sticky &&win_it.current()->desktop_nr==1))
      break;
      if (win_it.current()) {
        WindowList *sticky= win_it.current();
        WindowList *copy = new WindowList;
        copy->win= sticky->win;
        copy->width=sticky->width;
        copy->height=sticky->height;
        copy->x_position=sticky->x_position;
        copy->y_position=sticky->y_position;
        copy->icon=sticky->icon;
        copy->sticky=sticky->sticky;
        copy->shaded=sticky->shaded;
        copy->desktop_nr=tmp->desktop_nr;
        bbtool->addFrameWindow(copy,0,False);
        bbtool->windowList->insert(copy);
      } */
    }
  } else if (number_of_desktops<old_number_of_desktops)
    bbtool->removeDesktopWindow();
}


bool WMInterface::readActiveWindow(Window target, Window *active)
{
  unsigned char* data = NULL;
  if (netwm->getProperty(target, XA_CARDINAL, netwm->activeWindow(), &data)) {
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
        //updateWindowStack();
    } else if (event->atom ==  netwm->numberOfDesktops()) {
        unsigned int number;
        if (!netwm->readNumberOfDesktops(root_window, &number)) {
            //error
        }
        changeNumberOfDesktops(number);
    } else if (event->atom == netwm->desktopGeometry()) {

    } else if (event->atom == netwm->currentDesktop()) {
        unsigned int current_desktop;
        netwm->readCurrentDesktop(root_window, &current_desktop);
        bbtool->desktopChange(current_desktop);
    } else if (event->atom == netwm->desktopNames()) {

    } else if (event->atom == netwm->activeWindow()) {
        Window active;
        readActiveWindow(root_window, &active);
        focus(active);
    } else if (event->atom == netwm->workarea()) {

    } else {
        // ignore
    }

   
    
    //wminterface->windowAttributeChange(event->xproperty.window);
}


