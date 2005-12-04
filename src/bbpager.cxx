// bbpager.cc for bbpager - a pager for Blackbox.
//
//  Copyright (c) 1998-2003 by John Kennis, jkennis@chello.nl
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

#include <string>
#include "bbpager.h"
#include "config.h"
#include <stdio.h>

extern "C" {
#include <X11/cursorfont.h>
}

#include <iostream>

using std::cout;
using std::endl;
using std::list;

ToolWindow::ToolWindow(Configuration cml_options):
    bt::Application(cml_options.appName(), cml_options.displayName().c_str(), false),
    current_screen_info(display().screenInfo(DefaultScreen(XDisplay()))),
    config(cml_options)

{

    current_screen = DefaultScreen(XDisplay());
    root_window = current_screen_info.rootWindow();
    number_of_desktops = 0;
    desktop_nr = 0;
    current_desktop_nr = 0;
    wm_init = false;
    row_last = column_last = 0;

    xa_wm_delete_window = XInternAtom(XDisplay(), "WM_DELETE_WINDOW", False);


    resource = new Resource(this, config.rcFilename());

    _ewmh = new bt::EWMH(display());
    if (!_ewmh->readNumberOfDesktops(current_screen_info.rootWindow(), &number_of_desktops))
    {
        delete _ewmh;
        delete resource;
        // temporary fix: 1: we could add a timer here an wait
        //                2: we should check for supported atoms, and not use
        //                number of desktops.
        std::string error = "Please start bbpager after an EWHM complaint Window manager";
        error += "\nIf you start bbpager from .xinitrc or .xsession consider delaying bbpager startup using \"sleep 5 && bbpager &\"";
        throw error;
    }

    frame_window = new FrameWindow(this);

    wminterface = new WMInterface(this);

    xa_wm_state =  XInternAtom(XDisplay(),"WM_STATE", False);

    unsigned int i;
    for (i = 0; i < number_of_desktops; i++) {
        addDesktopWindow(i);
    }
    _ewmh->readCurrentDesktop(current_screen_info.rootWindow(), &current_desktop_nr);
    desktopChange(current_desktop_nr);
    wminterface->updateWindowList();
    Window active;
    if (!wminterface->readActiveWindow(root_window, &active)) {
            cout << "Error: cannot get active window from Window Manager" << endl;
    }
    focusWindow(active);
}

ToolWindow::~ToolWindow() 
{
    list<DesktopWindow *>::iterator it;
    for (it = desktop_window_list.begin(); it != desktop_window_list.end(); it++) {
        delete (*it);
    }

    list<PagerWindow *>::iterator pit = pagerWindowList().begin();

    for (; pit != pagerWindowList().end(); pit++) {
        delete (*pit);
    }

}

void ToolWindow::moveWinToDesktop(PagerWindow *pager_window, unsigned int desktop_nr) 
{
    DesktopWindow *desktop = findDesktopWindow(desktop_nr);
    if (desktop) { 
        if ((desktop->desktopId() != pager_window->desktopId() && !pager_window->isSticky())) {

            XUnmapWindow(XDisplay(), pager_window->window());
            XReparentWindow(XDisplay(), pager_window->window(),
                    desktop->window(), pager_window->x(), pager_window->y());
            XMapWindow(XDisplay(), pager_window->window());
                   pager_window->setDesktopId(desktop_nr);
        }
    }
}

void ToolWindow::moveWinToDesktop(Window win, DesktopWindow *desktop) 
{

    PagerWindow *pager_window = findPagerWindow(win);
  
    if (pager_window != 0) {
        if ((desktop->desktopId() != pager_window->desktopId()) & (!pager_window->isSticky())) {

            XUnmapWindow(XDisplay(), pager_window->window());
            XReparentWindow(XDisplay(), pager_window->window(),
                    desktop->window(), pager_window->x(), pager_window->y());
            XMapWindow(XDisplay(), pager_window->window());
                   pager_window->setDesktopId(desktop_nr);
        }
    }
}

PagerWindow *ToolWindow::findPagerWindow(Window win)
{
    list<PagerWindow *>::iterator it = pager_window_list.begin();

    for (; it != pager_window_list.end(); it++) {
        if ((*it)->realWindow() == win) {
            return *it;
        }
    }
    return NULL;
}

PagerWindow *ToolWindow::findPPagerWindow(Window win)
{
    list<PagerWindow *>::iterator it = pager_window_list.begin();

    
    for (; it != pager_window_list.end(); it++) {
        int i = 0;
        while ((*it)->window(i) != 0)
        {
            if ((*it)->window(i) == win) {
                return *it;
            }
            i++;
        }
    }
    return NULL;
}

PagerWindow *ToolWindow::findFocusedPagerWindow()
{
    list<PagerWindow *>::iterator it = pager_window_list.begin();

    
    for (; it != pager_window_list.end(); it++) {
        if ((*it)->isFocused()) {
            return *it;
        }
    }
    return NULL;
}


void ToolWindow::reconfigure(void) 
{
    //resource->Reload();
    delete resource;
    resource = new Resource(this, config.rcFilename());
    MakeWindow(true);
  
    desktop_nr = 0;

    list<DesktopWindow *>::iterator dit = desktop_window_list.begin();
    for (; dit != desktop_window_list.end(); dit++) {
        (*dit)->reconfigure();
    }
    list<PagerWindow *>::iterator pit = pager_window_list.begin();
    for (; pit != pager_window_list.end(); pit++) {
        (*pit)->reconfigure();
    }
    //frame_window->reconfigure();
}

// raise window in pager
void ToolWindow::raiseWindow(Window win) 
{
    PagerWindow *pager_window = findPagerWindow(win);   
    if (pager_window)
        XRaiseWindow(XDisplay(), pager_window->window());
}

// lower window in pager
void ToolWindow::lowerWindow(Window win) 
{
    PagerWindow *pager_window = findPagerWindow(win);   
    if (pager_window)
            XLowerWindow(XDisplay(), pager_window->window());
}


// focus window in pager
void ToolWindow::focusWindow(Window win) 
{
    if (resource->getFocusStyle() != none) {
        PagerWindow *focus_window = findFocusedPagerWindow();

        /* remove focus from previously focused window */
        if (focus_window) {
            focus_window->clearFocus();
        }
    
        PagerWindow *pager_window = findPagerWindow(win);
            
        if (pager_window) { 
            pager_window->setFocus();
        }
    }
}

void ToolWindow::desktopChange(unsigned int desktop_nr) 
{
    if (resource->getDesktopFocusStyle() != none) {
        DesktopWindow *desktop_window;
        
        desktop_window = findDesktopWindow(current_desktop_nr);
        if (desktop_window)
            desktop_window->clearFocus();
        
        desktop_window = findDesktopWindow(desktop_nr);
        if (desktop_window)
            desktop_window->setFocus(); 
    }
    current_desktop_nr = desktop_nr;
    wminterface->updateWindowStack();
}

DesktopWindow *ToolWindow::findDesktopWindow(unsigned int desktop_nr)
{
    list<DesktopWindow *>::iterator it = desktop_window_list.begin();

    for (; it != desktop_window_list.end(); it++) {
        if ((*it)->desktopId() == desktop_nr) {
            return (*it);
        }
    }
    return NULL;
}

DesktopWindow *ToolWindow::findDesktopWindow(Window win)
{
    list<DesktopWindow *>::iterator it = desktop_window_list.begin();

    for (; it != desktop_window_list.end(); it++) {
        if ((*it)->window() == win) {
            return (*it);
        }
    }
    return NULL;
}



int ToolWindow::winOnDesktop(Window win) 
{
    PagerWindow *pager_window = findPagerWindow(win);

    if (pager_window)
        return pager_window->desktopId();

    return(0);
}

void ToolWindow::addDesktopWindow(unsigned int nr)
{
    desktop_window_list.push_back(new DesktopWindow(this, nr));
}

void ToolWindow::removeDesktopWindow(void)
{
    /* delete last */
    DesktopWindow *desktop_window = desktop_window_list.back();
    desktop_window_list.pop_back(); 
    delete desktop_window;
    desktop_nr--;
    frame_window->resize();
}


FrameWindow::FrameWindow(ToolWindow *toolwindow) :
    EventHandler(), bbtool(toolwindow)
{
    screen = bbtool->getCurrentScreen();
    display = bbtool->XDisplay();
    m_pixmap = 0;
    buildWindow(false);

    bbtool->insertEventHandler(win, this);
}

FrameWindow::~FrameWindow()
{
    bbtool->removeEventHandler(win);
    XUnmapWindow(display, win);
    /* destroy pixmaps */
    if (m_pixmap) bt::PixmapCache::release(m_pixmap);
    /* destroy windows */
    XDestroyWindow(display, win);
}

void FrameWindow::buildWindow(bool reconfigure)
{

    calcSize();

    m_pixmap = bt::PixmapCache::find(screen, 
             bbtool->resource->frame.texture, fwidth, fheight);

    if (!reconfigure) {
        XSetWindowAttributes attrib;
        XWMHints wmhints;
        XClassHint classhints;
        XTextProperty windowname;

        unsigned long create_mask = CWBackPixmap | CWBorderPixel | CWEventMask;


        
        attrib.background_pixmap = ParentRelative;
        attrib.border_pixel = bbtool->resource->desktopwin.activeColor.pixel(screen);
        attrib.event_mask = ButtonPressMask | ButtonReleaseMask | ExposureMask |
                      FocusChangeMask | StructureNotifyMask | 
                      SubstructureRedirectMask;

        win = XCreateWindow(display, bbtool->getCurrentScreenInfo()->rootWindow(), fx, fy, fwidth,
                             fheight, 0, bbtool->getCurrentScreenInfo()->depth(), InputOutput,
                             bbtool->getCurrentScreenInfo()->visual(), 
                             create_mask, &attrib);

        char *name="bbpager"; //BBTOOL;
     
        if (bbtool->configuration().isWithdrawn()) {
            wmhints.initial_state = WithdrawnState;
        } else {
            wmhints.initial_state = NormalState;
        }

        wmhints.flags = StateHint | InputHint;
        wmhints.input = False;
     
        classhints.res_name = "bbager"; // BBTOOL;
        classhints.res_class = "bbtools";
              
        XStringListToTextProperty(&name, 1, &windowname);
        XSetWMProperties(display, win ,&windowname, NULL, bbtool->configuration().argv(), 
                         bbtool->configuration().argc(), NULL, &wmhints, &classhints);
        XFree(windowname.value);
        Atom wmproto[1];
        wmproto[0] = bbtool->wmDeleteWindowAtom();
        XSetWMProtocols(display, win, wmproto, 1);


        if (!bbtool->configuration().isDecorated() && !bbtool->configuration().isWithdrawn()) {
            bt::EWMH::AtomList window_type_atom;
            window_type_atom.push_back(bbtool->ewmh()->wmWindowTypeDock());

            XChangeProperty(display, win, bbtool->ewmh()->wmWindowType(), XA_ATOM,
                            32, PropModeReplace,
                            reinterpret_cast<unsigned char*>(&(window_type_atom[0])), window_type_atom.size());
  
                 
       }
       if (!bbtool->configuration().isWithdrawn()) {
            unsigned int dekstop_nr = static_cast<unsigned int>(-1);
            XChangeProperty(display, win, bbtool->ewmh()->wmDesktop(), XA_CARDINAL,
                            32, PropModeReplace,
                            reinterpret_cast<unsigned char*>(&dekstop_nr), 1);

             bt::EWMH::AtomList window_state_atom;
            window_state_atom.push_back(bbtool->ewmh()->wmStateSticky());
            window_state_atom.push_back(bbtool->ewmh()->wmStateSkipTaskbar());
            window_state_atom.push_back(bbtool->ewmh()->wmStateSkipPager());

            XChangeProperty(display, win, bbtool->ewmh()->wmState(), XA_ATOM,
                          32, PropModeReplace,
                            reinterpret_cast<unsigned char*>(&(window_state_atom[0])), window_state_atom.size());
       } 
    } else if (!bbtool->configuration().isWithdrawn()) {
        XMoveResizeWindow(display, win, fx, fy, fwidth, fheight);
    } else {
        XResizeWindow(display, win, fwidth, fheight);
    }

//    disable shape, until we can get it working again
//    if (!bbtool->configuration().isShaped()) {
      bt::Rect u(0, 0, fwidth, fheight);
      bt::drawTexture(screen,
                       bbtool->resource->frame.texture,
                        win, 
                        u, u, m_pixmap);
//    }
    XMapWindow(display, win);
}

void FrameWindow::resize(void)
{
    buildWindow(true); // reconfigure
}

void FrameWindow::calcSize(void)
{

    if (bbtool->resource->position.horizontal) {
        if (bbtool->numberOfDesktops() < static_cast<unsigned int>(bbtool->resource->columns)) {
            fwidth = (unsigned int)(bbtool->getResource()->desktopSize.width + 
                     bbtool->resource->frame.desktopMargin) *
                     (bbtool->numberOfDesktops() % bbtool->getResource()->columns) + 
                     2 * bbtool->getResource()->frame.bevelWidth - bbtool->resource->frame.desktopMargin;
        } else {
            fwidth = (unsigned int)(bbtool->getResource()->desktopSize.width + 
                 bbtool->getResource()->frame.desktopMargin) *
                 bbtool->getResource()->columns + 2 * bbtool->getResource()->frame.bevelWidth - bbtool->resource->frame.desktopMargin;
        }
        fheight = (unsigned int)(((bbtool->numberOfDesktops() - 1) / bbtool->getResource()->columns + 1)*
                  (bbtool->getResource()->desktopSize.height + 
                   bbtool->getResource()->frame.desktopMargin) + 2 * bbtool->getResource()->frame.bevelWidth  - bbtool->resource->frame.desktopMargin);

    } else {
        fwidth = (unsigned int)((bbtool->numberOfDesktops() - 1) / bbtool->getResource()->rows + 1) *
                 (bbtool->getResource()->desktopSize.width + 
                 bbtool->getResource()->frame.desktopMargin) + 2 * bbtool->getResource()->frame.bevelWidth - bbtool->resource->frame.desktopMargin;
        if (bbtool->numberOfDesktops() < static_cast<unsigned int>(bbtool->getResource()->rows))
            fheight = (unsigned int)(bbtool->getResource()->desktopSize.height +
                  bbtool->getResource()->frame.desktopMargin) *
                  (bbtool->numberOfDesktops() % bbtool->getResource()->rows) + 
                  2 * bbtool->getResource()->frame.bevelWidth - bbtool->resource->frame.desktopMargin;
        else
            fheight = (unsigned int)(bbtool->getResource()->desktopSize.height +
                  bbtool->getResource()->frame.desktopMargin) *
                  bbtool->getResource()->rows + 2 * bbtool->getResource()->frame.bevelWidth - bbtool->resource->frame.desktopMargin;

    }

    fx = bbtool->getResource()->position.x;
    fy = bbtool->getResource()->position.y;

    if (bbtool->getResource()->position.mask & XNegative) {
        fx = bbtool->getCurrentScreenInfo()->width() + 
             bbtool->getResource()->position.x - fwidth;
    }

    if (bbtool->getResource()->position.mask & YNegative) {
        fy = bbtool->getCurrentScreenInfo()->height() +
             bbtool->getResource()->position.y - fheight;

    }
}

void FrameWindow::buttonPressEvent(const XButtonEvent * const event)
{
    if (event->button == LEFT_BUTTON) {
        XRaiseWindow(display, win);
        lower = false;
    }
    if (event->button == MIDDLE_BUTTON) {
        XLowerWindow(display, win);
        lower = true;
    }
}


void FrameWindow::configureNotifyEvent(const XConfigureEvent * const event)
{
    if (event->send_event) {
        if (bbtool->configuration().isWithdrawn())
            bbtool->reconfigure();
        int parent_x,parent_y;
        Window parent_root;
        unsigned int parent_width;
        unsigned int parent_height;
        unsigned int parent_border_width;
        unsigned int parent_depth;
        setXY( event->x, event->y);
        if (bbtool->configuration().isWithdrawn()) {
            XGetGeometry(display, event->above, &parent_root,
                         &parent_x, &parent_y, &parent_width, &parent_height,
                         &parent_border_width, &parent_depth);
                         setXY(event->x + parent_x + parent_x, 
                                               y() + parent_y);
        } else {
            if (bbtool->configuration().geometry().empty()) {
                char position[13];
                sprintf(position,"+%i+%i",x(),y());
                bbtool->configuration().setGeometry(position);
            }
        }
    }
}

void FrameWindow::clientMessageEvent(const XClientMessageEvent * const event)
{
   if ((unsigned)event->data.l[0] == bbtool->wmDeleteWindowAtom()) {
        bbtool->shutdown();
   }
}

void FrameWindow::exposeEvent(const XExposeEvent * const event)
{
    bt::Rect u(0, 0, fwidth, fheight);
    bt::drawTexture(screen,
                     bbtool->resource->frame.texture,
                     win, 
                     u, u, m_pixmap);
}

void ToolWindow::MakeWindow(bool reconfigure) 
{
}

void ToolWindow::shutdown(void)
{
    quit();
}


