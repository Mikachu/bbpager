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



DesktopWindow::DesktopWindow(ToolWindow *toolwindow, unsigned int _desktop_nr):
                 bt::EventHandler(), bbtool(toolwindow)
{
    screen = bbtool->getCurrentScreen();
    display = bbtool->XDisplay();
    resource = bbtool->getResource(); 
    desktop_nr = _desktop_nr;
    pixmap = pixmap_focused = 0;
    moved = false;
    buildWindow(false);
    bbtool->insertEventHandler(win, this);
}


DesktopWindow::~DesktopWindow(void)
{
    XDestroyWindow(bbtool->XDisplay(), win);

    if (pixmap) bt::PixmapCache::release(pixmap);
    if (pixmap_focused) bt::PixmapCache::release(pixmap_focused);

    std::list<PagerWindow *>::iterator it = bbtool->pagerWindowList().begin();

    for (; it != bbtool->pagerWindowList().end(); it++) {
        if ((*it)->isSticky()) {
            /* not yet supported */
        }
        if ((*it)->desktopId() == desktop_id) {
            bbtool->pagerWindowList().erase(it);
        }
    }
}

void DesktopWindow::reconfigure(void)
{
    buildWindow(true);
}

void DesktopWindow::buildWindow(bool reconfigure) 
{
    XSetWindowAttributes attrib;
    //int row, column;
    unsigned long create_mask = CWBackPixmap|CWCursor|CWEventMask|CWBorderPixel;

    attrib.background_pixmap = ParentRelative;
    attrib.border_pixel = resource->desktopwin.activeColor.pixel(screen);
    attrib.cursor =  XCreateFontCursor(display, XC_left_ptr); //getSessionCursor();
    attrib.event_mask = ButtonPressMask | ButtonReleaseMask | ExposureMask |
                FocusChangeMask | StructureNotifyMask|
                SubstructureRedirectMask | ButtonMotionMask;

    _width = resource->desktopSize.width;
    _height = resource->desktopSize.height;
  
    calcPosition();
    bbtool->frameWindow()->resize();

    pixmap = bt::PixmapCache::find(bbtool->getCurrentScreen(), 
             resource->desktopwin.texture, _width, _height, pixmap);

    if (resource->getDesktopFocusStyle() == texture) {
        pixmap_focused = bt::PixmapCache::find(bbtool->getCurrentScreen(), 
                 resource->desktopwin.focusedTexture, _width, _height, pixmap_focused);
    }
    if (!reconfigure)
        win = XCreateWindow(display, bbtool->frameWindow()->window(), _x, _y, _width,
                           _height, 0, bbtool->getCurrentScreenInfo()->depth(),
                           InputOutput, bbtool->getCurrentScreenInfo()->visual(),
                           create_mask, &attrib);
    else
        XMoveResizeWindow(bbtool->XDisplay(), win, _x, _y, _width, _height);

    XSetWindowBackgroundPixmap(bbtool->XDisplay(), win, pixmap);
    XClearWindow(display, bbtool->frameWindow()->window());
    XMapWindow(display, bbtool->frameWindow()->window());
    XMapSubwindows(display, bbtool->frameWindow()->window());
    desktop_id = bbtool->desktop_nr++;
}


void DesktopWindow::calcPosition(void)
{
    int column, row;
    unsigned int bw = bbtool->getResource()->frame.bevelWidth;
    if (bbtool->getResource()->position.horizontal) {
        // horizontal.
        row = (desktop_nr) / bbtool->getResource()->columns;
        column = (desktop_nr) % bbtool->getResource()->columns;
    } else {
        // vertical
        row = (desktop_nr) % bbtool->getResource()->rows;
        column = (desktop_nr) / bbtool->getResource()->rows;
    }
    _x = column * (bw + bbtool->getResource()->desktopSize.width) + bw;
    _y = row * (bw +  bbtool->getResource()->desktopSize.height) + bw;
}

void DesktopWindow::setFocus(void)
{
    if (resource->getDesktopFocusStyle() == border)
        XSetWindowBorderWidth(display, win, 1);
    else
        XSetWindowBackgroundPixmap(display, win, pixmap_focused);

    XClearWindow(display,win);

}

void DesktopWindow::clearFocus(void)
{
    if (resource->getDesktopFocusStyle() == border)
        XSetWindowBorderWidth(bbtool->XDisplay(), win, 0);
    else
        XSetWindowBackgroundPixmap(bbtool->XDisplay(), win, pixmap);

    XClearWindow(display, win);
}

void DesktopWindow::buttonPressEvent(const XButtonEvent * const event)
{
    if (event->button == static_cast<unsigned>(resource->getDesktopChangeButton())) {
        bbtool->wminterface->changeDesktop(desktopId());
    }
    if (event->button == static_cast<unsigned>(resource->getWindowRaiseButton())) {
        PagerWindow *pager = bbtool->findPPagerWindow(event->subwindow);
        if (pager) {
            XRaiseWindow(display, pager->realWindow());
        } 
    }
    if (event->button == static_cast<unsigned>(resource->getWindowFocusButton())) {
        PagerWindow *pager = bbtool->findPPagerWindow(event->subwindow);
        if (pager) {
            bbtool->wminterface->setWindowFocus(pager->realWindow());
        }
    }
    if (event->button == static_cast<unsigned>(resource->getWindowMoveButton())) {
//        DesktopWindow *desktop = findDesktopWindow(event->window);
        PagerWindow *pager = bbtool->findPPagerWindow(event->subwindow);

        if (pager) {
            XSetWindowAttributes attrib;
            unsigned long create_mask = CWBackPixmap|CWCursor|
                                        CWBorderPixel;

            attrib.background_pixmap = ParentRelative;
            attrib.border_pixel=
            resource->pagerwin.inactiveColor.pixel(screen);
            attrib.cursor = XCreateFontCursor(display, XC_left_ptr);

            grabbedWindow = XCreateWindow(display, bbtool->frameWindow()->window(),
                                        pager->x() + x(),
                                        pager->y() + y(),
                                        pager->width(), pager->height(),
                                        1, bbtool->getCurrentScreenInfo()->depth(),
                                        InputOutput,
                                        bbtool->getCurrentScreenInfo()->visual(),
                                        create_mask,&attrib);

            grabbed_x = x() + pager->x() - event->x;
            grabbed_y = y() + pager->y() - event->y;
            if (!pager->isFocused() || resource->getFocusStyle() != texture)
                XSetWindowBackgroundPixmap(display, grabbedWindow, pager->getPixmap());
            else
                XSetWindowBackgroundPixmap(display, grabbedWindow, pager->getFocusedPixmap());

            moveWindow = pager;
            realWindow = pager->realWindow();
            pagerWindow = pager->window();
            moved = false;
            XMapWindow(display,grabbedWindow);
        } else {
            grabbed_x = 0;
            grabbed_y = 0;
            grabbedWindow = 0;
        }
    }

}

void DesktopWindow::buttonReleaseEvent(const XButtonEvent * const event)
{
    if (event->button == static_cast<unsigned>(resource->getWindowMoveButton())) {
        if (grabbedWindow != 0) {
            if (!moved) {
                XDestroyWindow(display, grabbedWindow);
                grabbedWindow = 0;
                return;
            }
            std::list<DesktopWindow *>::iterator it = bbtool->desktopWindowList().begin();
            for (; it != bbtool->desktopWindowList().end(); it++) {
                if (move_x > (*it)->x() - (*it)->width() &&
                          move_x <= (*it)->x() + (*it)->width() &&
                          move_y > (*it)->y() - (*it)->height() &&
                          move_y < (*it)->y() + (*it)->height())
                break;
            } 
            if (it != bbtool->desktopWindowList().end()) {
                if (!moveWindow->isSticky()) 
                    bbtool->wminterface->sendWindowToDesktop(realWindow, (*it)->desktopId());
                double xdiv = static_cast<double>(resource->desktopSize.width) /
                             bbtool->getCurrentScreenInfo()->width();
                double ydiv = static_cast<double>(resource->desktopSize.height) /
                             bbtool->getCurrentScreenInfo()->height();

                int x = static_cast<int>((move_x - (*it)->x()) / xdiv);
                int y = static_cast<int>((move_y - (*it)->y()) / ydiv);
                XMoveWindow(display,realWindow, x, y);
                XUnmapWindow(display,grabbedWindow);
                XDestroyWindow(display,grabbedWindow);
//                if (!moveWindow->isSticky()) 
//                    bbtool->moveWinToDesktop(realWindow, *it);
                grabbedWindow = 0;
            } else {
                XDestroyWindow(display,grabbedWindow);
                grabbedWindow = 0;
            }
        }
    }
}

void DesktopWindow::motionNotifyEvent(const XMotionEvent * const event)
{
    if (grabbedWindow) {
        moved = true;
        move_x = event->x + grabbed_x;
        move_y = event->y + grabbed_y;
        XMoveWindow(display, grabbedWindow, event->x + grabbed_x,
                    event->y + grabbed_y);
    }
}




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

    if (skip) return;       // don't build window if state skipPagerWindow is set.
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

    if (resource->getFocusStyle()==texture)
        pixmap_focused = bt::PixmapCache::find(screen, resource->pagerwin.focusedTexture,
            resource->desktopSize.width, resource->desktopSize.height);

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
    unsigned long create_mask = CWBackPixmap|CWCursor|CWBorderPixel;
    XSetWindowAttributes attrib;

    attrib.background_pixmap = ParentRelative;
    attrib.border_pixel=resource->pagerwin.inactiveColor.pixel(screen);
    attrib.cursor = XCreateFontCursor(display, XC_left_ptr); //getSessionCursor();

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
    
    if (!hidden && !iconic)
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
                destroyWindow();                
            }
        } else {
            if (skip) {
                skip = false;
                buildWindow(false);                
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
                    hideWindow();
                 }
            } else {
                if (iconic) {
                    iconic = false;
                    showWindow();
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
 
    resource = new Resource(this, config.rcFilename());
    _netwm = new bt::Netwm(XDisplay());
    _netwm->readNumberOfDesktops(current_screen_info.rootWindow(), &number_of_desktops);
    frame_window = new FrameWindow(this);
    wminterface = new WMInterface(this);
    char *named_atoms[] = {
        "WM_DELETE_WINDOW",
        "WM_STATE"
    };
    Atom atoms[2];
    
    XInternAtoms(XDisplay(), named_atoms, 2, False, atoms);
    xa_wm_delete_window = atoms[0];
    xa_wm_state = atoms[1];
    unsigned int i;
    for (i = 0; i < number_of_desktops; i++) {
        addDesktopWindow(i);
    }
    _netwm->readCurrentDesktop(current_screen_info.rootWindow(), &current_desktop_nr);
    desktopChange(current_desktop_nr);
    wminterface->updateWindowList();
    Window active;
    if (!wminterface->readActiveWindow(root_window, &active)) {
            printf("error cannot read active window\n");
    }
    focusWindow(active);
}

ToolWindow::~ToolWindow() 
{
    std::list<DesktopWindow *>::iterator it;
    for (it = desktop_window_list.begin(); it != desktop_window_list.end(); it++) {
        delete (*it);
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
    std::list<PagerWindow *>::iterator it = pager_window_list.begin();

    for (; it != pager_window_list.end(); it++) {
        if ((*it)->realWindow() == win) {
            return *it;
        }
    }
    return NULL;
}

PagerWindow *ToolWindow::findPPagerWindow(Window win)
{
    std::list<PagerWindow *>::iterator it = pager_window_list.begin();

    
    for (; it != pager_window_list.end(); it++) {
        if ((*it)->window() == win) {
            return *it;
        }
    }
    return NULL;
}

PagerWindow *ToolWindow::findFocusedPagerWindow()
{
    std::list<PagerWindow *>::iterator it = pager_window_list.begin();

    
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

    MakeWindow(true);
  
    desktop_nr = 0;

    std::list<DesktopWindow *>::iterator dit = desktop_window_list.begin();
    for (; dit != desktop_window_list.end(); dit++) {
        (*dit)->reconfigure();
    }
    std::list<PagerWindow *>::iterator pit = pager_window_list.begin();
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
    std::list<DesktopWindow *>::iterator it = desktop_window_list.begin();

    for (; it != desktop_window_list.end(); it++) {
        if ((*it)->desktopId() == desktop_nr) {
            return (*it);
        }
    }
    return NULL;
}

DesktopWindow *ToolWindow::findDesktopWindow(Window win)
{
    std::list<DesktopWindow *>::iterator it = desktop_window_list.begin();

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


//void ToolWindow::addFrameWindow(struct WindowList *window,Window desktopWin,bool reconfigure) 
//{
//}

FrameWindow::FrameWindow(ToolWindow *toolwindow) :
    EventHandler(), bbtool(toolwindow)
{
    screen = bbtool->getCurrentScreen();
    display = bbtool->XDisplay();
    buildWindow(false);
    bbtool->insertEventHandler(win, this);
}

FrameWindow::~FrameWindow()
{
    XUnmapWindow(display, win);
    /* destroy pixmaps */
    if (pixmap) bt::PixmapCache::release(pixmap);
    /* destroy windows */
    XDestroyWindow(display, win);
}

void FrameWindow::buildWindow(bool reconfigure)
{

    calcSize();

    pixmap = bt::PixmapCache::find(screen, 
             bbtool->resource->frame.texture, fwidth, fheight);


    if (!reconfigure) {
        XSetWindowAttributes attrib;
        XWMHints wmhints;
        XClassHint classhints;
        XTextProperty windowname;

        unsigned long create_mask = CWBackPixmap | CWBorderPixel |
                        CWCursor | CWEventMask;


        
        attrib.background_pixmap = ParentRelative;
        attrib.border_pixel = bbtool->resource->desktopwin.activeColor.pixel(screen);
        attrib.cursor = XCreateFontCursor(display, XC_left_ptr);
        attrib.event_mask = ButtonPressMask | ButtonReleaseMask | ExposureMask |
                      FocusChangeMask | StructureNotifyMask | 
                      SubstructureRedirectMask;

        win = XCreateWindow(display, bbtool->getCurrentScreenInfo()->rootWindow(), fx, fy, fwidth,
                             fheight, 0, bbtool->getCurrentScreenInfo()->depth(), InputOutput,
                             bbtool->getCurrentScreenInfo()->visual(), 
                             create_mask, &attrib);

        char *name="bbpager"; //BBTOOL;
        XSizeHints sizehints;
     
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
                         bbtool->configuration().argc(), &sizehints, &wmhints, &classhints);
        Atom wmproto[1];
        wmproto[0] = bbtool->wmDeleteWindowAtom();
        XSetWMProtocols(display, win, wmproto, 1);

        bt::Netwm::AtomList window_type_atom;
        window_type_atom.push_back(bbtool->netwm()->wmWindowTypeDock());

        if (!bbtool->configuration().isDecorated() && !bbtool->configuration().isWithdrawn()) {
            XChangeProperty(display, win, bbtool->netwm()->wmWindowType(), XA_ATOM,
                            32, PropModeReplace,
                            reinterpret_cast<unsigned char*>(&(window_type_atom[0])), window_type_atom.size());
        } 
                             
    } else if (!bbtool->configuration().isWithdrawn()) {
        XMoveResizeWindow(display, win, fx, fy, fwidth, fheight);
    } else {
        XResizeWindow(display, win, fwidth, fheight);
    }

    if (!bbtool->configuration().isShaped()) {
        XSetWindowBackgroundPixmap(display, win, pixmap);
    }

//    XClearWindow(display, win);
    XMapWindow(display, win);
    XMapSubwindows(display, win);
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
                     bbtool->resource->frame.bevelWidth) *
                     (bbtool->numberOfDesktops() % bbtool->getResource()->columns) + 
                     bbtool->getResource()->frame.bevelWidth;
        } else {
            fwidth = (unsigned int)(bbtool->getResource()->desktopSize.width + 
                 bbtool->getResource()->frame.bevelWidth) *
                 bbtool->getResource()->columns + bbtool->getResource()->frame.bevelWidth;
        }
        fheight = (unsigned int)(((bbtool->numberOfDesktops() - 1) / bbtool->getResource()->columns + 1)*
                  (bbtool->getResource()->desktopSize.height + 
                   bbtool->getResource()->frame.bevelWidth) + bbtool->getResource()->frame.bevelWidth);

    } else {
        fwidth = (unsigned int)((bbtool->numberOfDesktops() - 1) / bbtool->getResource()->rows + 1) *
                 (bbtool->getResource()->desktopSize.width + 
                 bbtool->getResource()->frame.bevelWidth) + bbtool->getResource()->frame.bevelWidth;
        if (bbtool->numberOfDesktops() < static_cast<unsigned int>(bbtool->getResource()->rows))
            fheight = (unsigned int)(bbtool->getResource()->desktopSize.height +
                  bbtool->getResource()->frame.bevelWidth)*
                  (bbtool->numberOfDesktops() % bbtool->getResource()->rows) + 
                  bbtool->getResource()->frame.bevelWidth;
        else
            fheight = (unsigned int)(bbtool->getResource()->desktopSize.height +
                  bbtool->getResource()->frame.bevelWidth) *
                  bbtool->getResource()->rows + bbtool->getResource()->frame.bevelWidth;

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
   if ((unsigned)event->data.l[0] == bbtool->wmDeleteWindowAtom()) bbtool->shutdown();
}

void ToolWindow::MakeWindow(bool reconfigure) 
{
}

void ToolWindow::shutdown(void)
{
    quit();
}


