//  pager.cxx.h for bbpager - an pager tool for Blackbox.
//
//  Copyright (c) 1998-2004 by John Kennis, jkennis@chello.nl
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

#include "pager.h"

extern "C" {
#include <X11/cursorfont.h>
}

#include <iostream>

using std::cout;
using std::endl;

PagerWindow::PagerWindow(ToolWindow *toolwindow, Window _window):
    bt::EventHandler(), bbtool(toolwindow), netwm(toolwindow->netwm())
{
    pwin = 0;
    win = _window;
    netwm->readWMDesktop(_window, desktop_nr);
    display = bbtool->XDisplay();
    resource = bbtool->getResource();
    screen = bbtool->getCurrentScreen();
    marked = true;
    focused = false;
    hidden = false;
    iconic = false;
    sticky = false;
    skip = false;
    shaded = false;
    pwin = NULL;
    number_of_desktops = 0;
    XSelectInput(display, win, PropertyChangeMask|StructureNotifyMask);
    bbtool->insertEventHandler(win, this);

    //get state of window
    bt::Netwm::AtomList states;
    bt::Netwm::AtomList::const_iterator it;

    netwm->readWMState(win, states);
    for (it = states.begin(); it != states.end(); it++) {
        if ((*it) == netwm->wmStateShaded()) {
            shaded = true; // window resized by configureNotify ?
        } 
//      handled by wmDesktop !
//      else if ((*it) == netwm->wmStateSticky()) {
//      }
        else if ((*it) == netwm->wmStateSkipPager()) {
            skip = true;
        }
        else if ((*it) == netwm->wmStateHidden()) {
            hidden = true;
        }
    }
    if (bbtool->wminterface->isIconicState(win)) {
        iconic = true;
    }     
    buildWindow(false);
}

PagerWindow::~PagerWindow(void)
{
    unsigned int i;
    bbtool->removeEventHandler(win);
    for (i = 0; i < number_of_desktops; i++)
        XDestroyWindow(display, pwin[i]);  
    delete [] pwin;
    if (pixmap) bt::PixmapCache::release(pixmap);
    if (pixmap_focused) bt::PixmapCache::release(pixmap_focused);
}

Window PagerWindow::window(void) 
{ 
    Window win;
    if (!sticky) {
        win = pwin[0];
    } else {
        int nr = bbtool->getCurrentDesktopNr();
        //assert(nr >= number_of_desktops);
        win = pwin[nr];
    }
    return win; 
}

void PagerWindow::raise(void)
{
    unsigned int i;
    for (i = 0; i < number_of_desktops; i++) {
        XRaiseWindow(display, pwin[i]);
    }
}

void PagerWindow::lower(void)
{
    unsigned int i;
    for (i = 0; i < number_of_desktops; i++) {
        XLowerWindow(display, pwin[i]);
    }
}


void PagerWindow::buildWindow(bool reconfigure)
{
    double xdiv,ydiv;

//    if (skip) return;       // don't build window if state skipPagerWindow is set.
    initWindowGeometry();
    xdiv = 10;
    ydiv = 10;
    if (desktop_nr == static_cast<unsigned int>(-1)) {  // on all desktops
        sticky = true;
    }
    xdiv = (double)resource->desktopSize.width / 
        bbtool->getCurrentScreenInfo()->width();
    ydiv = (double)resource->desktopSize.height / bbtool->getCurrentScreenInfo()->height();
    pager_x = (int)(window_x * xdiv);
    pager_y = (int)(window_y * ydiv);
    pager_width = (unsigned int)(window_width * xdiv);
    pager_height=(unsigned int)(window_height * ydiv);

    if (shaded)
        pager_height = static_cast<unsigned int>(pager_height * ydiv);
    
    if (pager_width == 0)   
        pager_width = 1;
    if (pager_height == 0) 
        pager_height = 1;

    pixmap = bt::PixmapCache::find(screen, resource->pagerwin.texture, 
             pager_width, pager_height);
    if (pixmap == 0) {
        cout << "Error: cannot create pager window pixmap with texture: \"";
        cout << resource->pagerwin.texture.description() << "\"" << endl;
    }
    if (resource->getFocusStyle()==texture) {
        pixmap_focused = bt::PixmapCache::find(screen, resource->pagerwin.focusedTexture,
            resource->desktopSize.width, resource->desktopSize.height);
        if (pixmap == 0) {
            cout << "Error: cannot create focused pager window pixmap with texture: \"";
            cout << resource->pagerwin.focusedTexture.description() << "\"" << endl;
        }
    }
    unsigned int i;
    if (!sticky) {
        if (number_of_desktops != 1) {
            //Don't destroy windows on desktops > actual desktops,these windows are/will be detroyed by destroying desktop window.
            for (i = 0; i < number_of_desktops && i < static_cast<unsigned int>(bbtool->getNumberOfDesktops()); i++) {
                XDestroyWindow(display, pwin[i]);
            }
            number_of_desktops = 1;
            delete [] pwin;
            pwin = new Window[number_of_desktops];
            reconfigure = false;    // rebuild windows
        } 
            buildPagerWindow(reconfigure, desktop_nr);
    } else {
        if (number_of_desktops != static_cast<unsigned int>(bbtool->getNumberOfDesktops())) {
            //Don't destroy windows on desktops > actual desktops,these windows are/will be detroyed by destroying desktop window.
            for (i = 0; i < number_of_desktops && i < static_cast<unsigned int>(bbtool->getNumberOfDesktops()); i++) {  
                XDestroyWindow(display, pwin[i]);
            }
            number_of_desktops = bbtool->getNumberOfDesktops();
            delete [] pwin;
            pwin = new Window[number_of_desktops];
            reconfigure = false;
        }
        for (i = 0; i < number_of_desktops; i++) {
            buildPagerWindow(reconfigure, i);
        }
    }
}

void PagerWindow::showWindow()
{
    unsigned int i;
    for (i = 0; i < number_of_desktops; i++) {
        XMapWindow(display, pwin[i]);
    }
}

void PagerWindow::hideWindow()
{
    unsigned int i;
    for (i = 0; i < number_of_desktops; i++) {
        XUnmapWindow(display, pwin[i]);
    }
}

void PagerWindow::destroyWindow()
{
    unsigned int i;
    for (i = 0; i < number_of_desktops; i++) {
        XDestroyWindow(display, pwin[i]);
    }
}

void PagerWindow::buildPagerWindow(bool reconfigure, unsigned int nr)
{
    unsigned long create_mask = CWBackPixmap|CWBorderPixel;
    XSetWindowAttributes attrib;

    attrib.background_pixmap = ParentRelative;
    attrib.border_pixel=resource->pagerwin.inactiveColor.pixel(screen);

    DesktopWindow *desktop = bbtool->findDesktopWindow(nr);
    if (!sticky) nr = 0;
    if (desktop == NULL) { //not on existing window
        return;
    }
    if (!reconfigure) {
        pwin[nr] = XCreateWindow(display, desktop->window(),
                 pager_x, pager_y, pager_width, pager_height,
                 1, bbtool->getCurrentScreenInfo()->depth(), 
                 InputOutput, bbtool->getCurrentScreenInfo()->visual(), 
                 create_mask, &attrib);
    } else
        XMoveResizeWindow(display, pwin[nr], pager_x, pager_y, pager_width, pager_height);


    
    if (!focused)
        XSetWindowBackgroundPixmap(display, pwin[nr], pixmap);
    else
        XSetWindowBackgroundPixmap(display, pwin[nr], pixmap_focused);
    
    if (!hidden /*&& !iconic*/ && !skip)
        XMapWindow(display, pwin[nr]);

    XClearWindow(display, pwin[nr]);
}

void PagerWindow::propertyNotifyEvent(const XPropertyEvent * const event)
{
    if (event->atom == netwm->wmDesktop()) {
        unsigned int desktop_nr;
        netwm->readWMDesktop(event->window, desktop_nr);
        if (desktop_nr == static_cast<unsigned int>(-1)) {
            if (!sticky) {
                sticky = true;
                buildWindow(false); // rebuild windows, to appear on all desktops        
            }
            //else ignore, cannot move to desktop we are already on.
        } else {
            if (sticky) {
                sticky = false;
                buildWindow(false); // rebuild windows, to appear on only one desktop 
            } else
                bbtool->moveWinToDesktop(this, desktop_nr);
        }
    } 
    else if (event->atom == netwm->wmState()) {
        bt::Netwm::AtomList states;
        bt::Netwm::AtomList::const_iterator it;
        bool skip_state = false;
        bool shaded_state = false;
        bool hidden_state = false;
        netwm->readWMState(event->window, states);
        for (it = states.begin(); it != states.end(); it++) {
            if ((*it) == netwm->wmStateShaded()) {
                shaded_state = true;
            } 
//          handled by wmDesktop !
//          if ((*it) == netwm->wmStateSticky()) {
//          }
            if ((*it) == netwm->wmStateSkipPager()) {
                skip_state = true;
            }
            if ((*it) == netwm->wmStateHidden()) {
                hidden_state = true;
            }
        }
        if (shaded_state) {
            if (!shaded) {
                shaded = true;
                buildWindow(true);
            }
        } else {
            if (shaded) {
                shaded = false;
                buildWindow(true);
            }
        }
        if (skip_state) {
            if (!skip) {
                skip = true;
                hideWindow();                
            }
        } else {
            if (skip) {
                skip = false;
                showWindow();                
            }
        }
        if (hidden_state) {
            if (!hidden) {
                hidden = true;
                hideWindow();
            }
        } else {
            if (hidden) {
                hidden = false;
                showWindow();
            }
        }
        // check out state.
    } else {
        if (event->atom == bbtool->wmStateAtom()) {
            if (bbtool->wminterface->isIconicState(event->window)) {
                if (!iconic) {
                    iconic = true;
//                    hideWindow();
                 }
            } else {
                if (iconic) {
                    iconic = false;
//                    showWindow();
                }
            }
        }  
    }
}

void PagerWindow::configureNotifyEvent(const XConfigureEvent * const event)
{
    if (pwin) {
        initWindowGeometry();
        double xdiv = static_cast<double>(bbtool->getResource()->desktopSize.width) / bbtool->getCurrentScreenInfo()->width();
        double ydiv = static_cast<double>(bbtool->getResource()->desktopSize.height) / bbtool->getCurrentScreenInfo()->height();
        pager_x = (int)(window_x * xdiv);
        pager_y = (int)(window_y * ydiv);
        pager_width = (unsigned int)(window_width * xdiv);
        pager_height=(unsigned int)(window_height * ydiv);

        if (shaded)
            pager_height = static_cast<unsigned int>(pager_height * ydiv);

        if (pager_width == 0)   
            pager_width = 1;
        if (pager_height == 0) 
            pager_height = 1;

        // resize pager window, i = 0 for none sticky windows
        unsigned int i;
        for (i = 0; i < number_of_desktops; i++) {
            XMoveResizeWindow(display, pwin[i], pager_x, pager_y, pager_width, pager_height);
            if (!focused)
                XSetWindowBackgroundPixmap(display, pwin[i], pixmap);
            else
                XSetWindowBackgroundPixmap(display, pwin[i], pixmap_focused);
        }
 
    }
}

void PagerWindow::reconfigure(void)
{
    buildWindow(true);  
}

int PagerWindow::initWindowGeometry(void) 
{
    unsigned int border_width, depth;
    Window root_return, child_return;
    int status;

    status = XGetGeometry(display, win, &root_return, &window_x,
                        &window_y, &window_width, &window_height,
                        &border_width, &depth);
    if (status) {
        XTranslateCoordinates(display, win, root_return, window_x,
                          window_y, &window_x, &window_y, &child_return);
        return 1;
    }
    return 0;
}

void PagerWindow::setFocus(void)
{
    unsigned int i;
    for (i = 0; i < number_of_desktops; i++) { // i=1 for none sticky windows
        if (resource->getFocusStyle() == border)
            XSetWindowBorder(display, pwin[i],
                     resource->pagerwin.activeColor.pixel(screen));
        else 
            XSetWindowBackgroundPixmap(display, pwin[i], pixmap_focused);
        
        XClearWindow(display, pwin[i]);
   }
   focused = true;
}

void PagerWindow::clearFocus(void)
{
    unsigned int i;
    for (i = 0; i < number_of_desktops; i++) { // i=1 for none sticky windows
        if (resource->getFocusStyle() == border)
            XSetWindowBorder(display, pwin[i],
                    resource->pagerwin.inactiveColor.pixel(screen));
        else
            XSetWindowBackgroundPixmap(display, pwin[i], pixmap);

        XClearWindow(display, pwin[i]);
    }
    focused = false;
}

