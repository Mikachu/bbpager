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



DesktopWindow::DesktopWindow(ToolWindow *toolwindow):
                 bt::EventHandler(), bbtool(toolwindow)
{
    screen = bbtool->getCurrentScreen();
    display = bbtool->XDisplay();
    resource = bbtool->getResource();  
    moved = false;
    buildWindow(false);
    bbtool->insertEventHandler(win, this);
}


DesktopWindow::~DesktopWindow(void)
{
    XDestroyWindow(bbtool->XDisplay(), win);

    if (pixmap) bt::PixmapCache::release(pixmap);
    if (pixmap_focused) bt::PixmapCache::release(pixmap_focused);

    std::list<PagerWindow>::iterator it = bbtool->pagerWindowList().begin();

    for (; it != bbtool->pagerWindowList().end(); it++) {
        if ((*it).isSticky()) {
            /* not yet supported */
        }
        if ((*it).desktopId() == desktop_id) {
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
  
    bbtool->frameWindow()->resize();
    _x = bbtool->frameWindow()->desktopX();
    _y = bbtool->frameWindow()->desktopY();

    pixmap = bt::PixmapCache::find(bbtool->getCurrentScreen(), 
             resource->desktopwin.texture, _width, _height);

    if (resource->getDesktopFocusStyle() == texture)
        pixmap = bt::PixmapCache::find(bbtool->getCurrentScreen(), 
                 resource->desktopwin.focusedTexture, _width, _height);

    if (!reconfigure)
        win = XCreateWindow(display, bbtool->frameWindow()->window(), _x, _y, _width,
                           _height, 0, bbtool->getCurrentScreenInfo()->depth(),
                           InputOutput, bbtool->getCurrentScreenInfo()->visual(),
                           create_mask, &attrib);
    else
        XMoveResizeWindow(bbtool->XDisplay(), win, _x, _y, _width, _height);

    XSetWindowBackgroundPixmap(bbtool->XDisplay(), win, pixmap);
    XMapSubwindows(display, bbtool->frameWindow()->window());
    XClearWindow(display, bbtool->frameWindow()->window());
    desktop_id = bbtool->desktop_nr++;
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
//        DesktopWindow *desktop = findDesktopWindow(event->window);               
//        if (desktop) {
            bbtool->wminterface->changeDesktop(desktopId());
//        }
    }
    if (event->button == static_cast<unsigned>(resource->getWindowRaiseButton())) {
        PagerWindow *pager = bbtool->findPagerWindow(event->subwindow);
        if (pager) {
            XRaiseWindow(display, pager->realWindow());
        } 
    }
    if (event->button == static_cast<unsigned>(resource->getWindowFocusButton())) {
        PagerWindow *pager = bbtool->findPagerWindow(event->subwindow);
        if (pager) {
            bbtool->wminterface->setWindowFocus(pager->realWindow());
        }
    }
    if (event->button == static_cast<unsigned>(resource->getWindowMoveButton())) {
//        DesktopWindow *desktop = findDesktopWindow(event->window);
        PagerWindow *pager = bbtool->findPagerWindow(event->subwindow);

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
                pager->clearFocus();
            //              XSetWindowBackgroundPixmap(XDisplay(), grabbedWindow, pager->pixmap());
            else
                pager->setFocus();
            //               XSetWindowBackgroundPixmap(XDisplay(), grabbedWindow, pixmap.focusedWindow);

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
            std::list<DesktopWindow>::iterator desktop_it = bbtool->desktopWindowList().begin();
            for (; desktop_it != bbtool->desktopWindowList().end(); desktop_it++) {
                if (move_x > desktop_it->x() - desktop_it->width() &&
                          move_x <= desktop_it->x() + desktop_it->width() &&
                          move_y > desktop_it->y() - desktop_it->height() &&
                          move_y < desktop_it->y() + desktop_it->height())
                break;
            } 
            if (desktop_it != bbtool->desktopWindowList().end()) {
                if (!moveWindow->isSticky()) 
                    bbtool->wminterface->sendWindowToDesktop(realWindow, desktop_it->desktopId());
                double xdiv = static_cast<double>(resource->desktopSize.width) /
                             bbtool->getCurrentScreenInfo()->width();
                double ydiv = static_cast<double>(resource->desktopSize.height) /
                             bbtool->getCurrentScreenInfo()->height();

                int x = static_cast<int>((move_x - desktop_it->x()) / xdiv);
                int y = static_cast<int>((move_y - desktop_it->y()) / ydiv);
                XMoveWindow(display,realWindow, x, y);
                XUnmapWindow(display,grabbedWindow);
                XDestroyWindow(display,grabbedWindow);
                if (!moveWindow->isSticky()) 
                    bbtool->moveWinToDesktop(realWindow, &(*desktop_it));
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
    bbtool(toolwindow)
{
    win = _window;
    display = bbtool->XDisplay();
    resource = bbtool->getResource();
    screen = bbtool->getCurrentScreen();
    marked = true;
    buildWindow(false);
}

PagerWindow::~PagerWindow(void)
{

    XDestroyWindow(display, pwin);
    if (pixmap) bt::PixmapCache::release(pixmap);
    if (pixmap_focused) bt::PixmapCache::release(pixmap_focused);
}

void PagerWindow::buildWindow(bool reconfigure)
{
    XSetWindowAttributes attrib;
    Window desktop_window;
    unsigned long create_mask = CWBackPixmap|CWCursor|CWBorderPixel;
    double xdiv,ydiv;

    initWindowGeometry();
    xdiv = 10;
    ydiv = 10;

    desktop_window = bbtool->findDesktopWindow(desktop_nr)->window();

    attrib.background_pixmap = ParentRelative;
    attrib.border_pixel=resource->pagerwin.inactiveColor.pixel(screen);
    attrib.cursor = XCreateFontCursor(display, XC_left_ptr); //getSessionCursor();

    xdiv = (double)resource->desktopSize.width / 
        bbtool->getCurrentScreenInfo()->width();
    ydiv = (double)resource->desktopSize.height / bbtool->getCurrentScreenInfo()->height();
    pager_x = (int)(window_x * xdiv);
    pager_y = (int)(window_y * ydiv);
    pager_width = (unsigned int)(window_width * xdiv);
    pager_height=(unsigned int)(window_height * ydiv);
    if (pager_width == 0)   
        pager_width = 1;
    if (pager_height == 0) 
        pager_height = 1;

  
    if (!reconfigure)
        pwin = XCreateWindow(display, desktop_window,
                 pager_x, pager_y, pager_width, pager_height,
                 1, bbtool->getCurrentScreenInfo()->depth(), 
                 InputOutput, bbtool->getCurrentScreenInfo()->visual(), 
                 create_mask, &attrib);
    else
        XMoveResizeWindow(display, pwin, pager_x, pager_y, pager_width, pager_height);

    XSelectInput(display, win, PropertyChangeMask|StructureNotifyMask);

    pixmap = bt::PixmapCache::find(screen, resource->pagerwin.texture, 
             resource->desktopSize.width, resource->desktopSize.height);

    if (resource->getFocusStyle()==texture)
        pixmap_focused = bt::PixmapCache::find(screen, resource->pagerwin.focusedTexture,
            resource->desktopSize.width, resource->desktopSize.height);

    
    if (!focused)
        XSetWindowBackgroundPixmap(display, pwin, pixmap);
    else
        XSetWindowBackgroundPixmap(display, pwin, pixmap_focused);
    
    if (!icon)
        XMapWindow(display, pwin);

    XClearWindow(display, pwin);

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
    if (resource->getFocusStyle() == border)
        XSetWindowBorder(display, pwin,
                 resource->pagerwin.activeColor.pixel(screen));
    else 
        XSetWindowBackgroundPixmap(display, pwin, pixmap_focused);
    
        XClearWindow(display, pwin);
        focused = true;
    
}

void PagerWindow::clearFocus(void)
{

    if (resource->getFocusStyle() == border)
        XSetWindowBorder(display, pwin,
                resource->pagerwin.inactiveColor.pixel(screen));
    else
        XSetWindowBackgroundPixmap(display, pwin, pixmap);

        focused = false;
      
        XClearWindow(display, pwin);
}


ToolWindow::ToolWindow(Configuration cml_options):
    bt::Application(cml_options.appName(), cml_options.displayName().c_str(), false),
    current_screen_info(display().screenInfo(DefaultScreen(XDisplay()))),
    config(cml_options)

{
    resource = new Resource(this, config.rcFilename());
    wminterface = new WMInterface(this);

    wm_delete_window = XInternAtom (XDisplay(), "WM_DELETE_WINDOW", False);
    desktop_nr = 0;
    current_desktop_nr = -1;
    wm_init = false;
    number_of_desktops = 0;
    row_last = column_last = 0;
    current_screen = DefaultScreen(XDisplay());
  //frame_window = new FrameWindow();
//  MakeWindow(false);

    _netwm = new bt::Netwm(XDisplay());
    wminterface->moduleInit();
    
}

ToolWindow::~ToolWindow() 
{
}

void ToolWindow::moveWinToDesktop(Window win, DesktopWindow *desktop) 
{

    PagerWindow *pager_window = findPagerWindow(win);
  
    if (pager_window == 0) {
        if ((desktop->desktopId() != pager_window->desktopId()) & (!pager_window->isSticky())) {

            XUnmapWindow(XDisplay(), pager_window->window());
            XReparentWindow(XDisplay(), pager_window->window(),
                    desktop->window(), pager_window->x(), pager_window->y());
            XMapWindow(XDisplay(), pager_window->window());
                   pager_window->setDesktopId(desktop_nr);
        }
    }
}

PagerWindow *ToolWindow::findPagerWindow(Window win, std::list<PagerWindow>::iterator return_it )
{
    std::list<PagerWindow>::iterator it = pager_window_list.begin();

    for (; it != pager_window_list.end(); it++) {
        if ((*it).window() == win) {
            if (return_it != NULL)
                return_it = it;
            return &(*it);
        }
    }
    return NULL;
}



void ToolWindow::reconfigure(void) 
{
    //resource->Reload();

    MakeWindow(true);
  
    desktop_nr = 0;

    std::list<DesktopWindow>::iterator dit = desktop_window_list.begin();
    for (; dit != desktop_window_list.end(); dit++) {
        (*dit).reconfigure();
    }
    std::list<DesktopWindow>::iterator pit = desktop_window_list.begin();
    for (; pit != desktop_window_list.end(); pit++) {
        (*pit).reconfigure();
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
    /* remove focus from previously focused window */
    if (focuswin) {
        focuswin->clearFocus();
    }

    PagerWindow *pager_window = findPagerWindow(win);
    
    if (pager_window) { 
        pager_window->setFocus();
        focuswin = pager_window;
    }
}

void ToolWindow::desktopChange(int desktop_nr) 
{
    if (resource->getDesktopFocusStyle() != none) {
        
        if (current_desktop_nr != -1) {
            DesktopWindow *desktop_window = findDesktopWindow(current_desktop_nr);
            if (desktop_window)
                desktop_window->clearFocus();
        }
        
        DesktopWindow *desktop_window = findDesktopWindow(desktop_nr);
        if (desktop_window)
            desktop_window->setFocus(); 
    }
    current_desktop_nr = desktop_nr;
}

DesktopWindow *ToolWindow::findDesktopWindow(int desktop_nr)
{
    std::list<DesktopWindow>::iterator it = desktop_window_list.begin();

    for (; it != desktop_window_list.end(); it++) {
        if (it->desktopId() == desktop_nr) {
            return &(*it);
        }
    }
    return NULL;
}

DesktopWindow *ToolWindow::findDesktopWindow(Window win)
{
    std::list<DesktopWindow>::iterator it = desktop_window_list.begin();

    for (; it != desktop_window_list.end(); it++) {
        if (it->window() == win) {
            return &(*it);
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

void ToolWindow::removeWindow(Window win) 
{

    std::list<PagerWindow>::iterator it;

    PagerWindow *pager_window = findPagerWindow(win, it);

    if (pager_window) {
        if (pager_window == focuswin)
            focuswin = NULL;
        const PagerWindow pwin = *pager_window;
        pager_window_list.erase(it);
        delete pager_window;
    }
    //if (tmp->sticky) wminterface->removeSticky(tmp->win,tmp->desktop_nr);
}

void ToolWindow::addDesktopWindow(void)
{
    desktop_window_list.push_back(DesktopWindow(this));
}

void ToolWindow::removeDesktopWindow(void)
{
    /* delete last */
  //    DesktopWindow dekstop_window = desktop_window_list.back();
    desktop_window_list.pop_back(); 
//  delete desktop_window
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
    XSetWindowAttributes attrib;
    XWMHints wmhints;
    XClassHint classhints;
    XTextProperty windowname;

    unsigned long create_mask = CWBackPixmap | CWBorderPixel |
                    CWCursor | CWEventMask;

    if (bbtool->configuration().isWithdrawn()) {
        wmhints.initial_state = WithdrawnState;
    } else {
        wmhints.initial_state = NormalState;
    }

    attrib.background_pixmap = ParentRelative;
    attrib.border_pixel = bbtool->resource->desktopwin.activeColor.pixel(screen);

    pixmap = bt::PixmapCache::find(screen, 
             bbtool->resource->desktopwin.texture, fwidth, fheight);

    attrib.cursor = XCreateFontCursor(display, XC_left_ptr);
    attrib.event_mask = ButtonPressMask | ButtonReleaseMask | ExposureMask |
                      FocusChangeMask | StructureNotifyMask|
                      SubstructureRedirectMask;

    calcSize();
    
    if (!reconfigure) {
        win = XCreateWindow(display, bbtool->getCurrentScreenInfo()->rootWindow(), fx, fy, fwidth,
                             fheight, 0, bbtool->getCurrentScreenInfo()->depth(), InputOutput,
                             bbtool->getCurrentScreenInfo()->visual(), 
                             create_mask, &attrib);
    } else if (!bbtool->configuration().isWithdrawn()) {
        XMoveResizeWindow(display, win, fx, fy, fwidth,fheight);

    } else {
        XResizeWindow(display, win, fwidth, fheight);
    }

    char *name="bbpager"; //BBTOOL;
    XSizeHints sizehints;

    wmhints.flags = StateHint | InputHint;
    wmhints.input = False;
 
    classhints.res_name = "bbager"; // BBTOOL;
    classhints.res_class = "bbtools";
          
    sizehints.x = fx;//getResource()->position.x;
    sizehints.y = fy;//getResource()->position.y;

    sizehints.max_width = sizehints.min_width = fwidth;
    sizehints.max_height = sizehints.min_height= fheight;
    sizehints.flags = USPosition | PMinSize | PMaxSize;

    XStringListToTextProperty(&name, 1, &windowname);
    XSetWMProperties(display, win ,&windowname, NULL, bbtool->configuration().argv(), 
                     bbtool->configuration().argc(), &sizehints, &wmhints, &classhints);
    Atom wmproto[1];
    wmproto[0] = bbtool->wmDeleteWindow();
    XSetWMProtocols(display, win, wmproto, 1);

    if (!bbtool->configuration().isShaped()) {
        XSetWindowBackgroundPixmap(display, win, pixmap);
    }

/*  if (!bbtool->configuration().isWithdrawn() && bbtool->resource->report.auto_raise) {
        XRaiseWindow(display, win);
        lower = false;
    }
    else lower=true;
*/
    XClearWindow(display, win);
    XMapWindow(display, win);
    XMapSubwindows(display, win);
}

void FrameWindow::resize(void)
{
    calcSize();
    calcDesktopPosition();
    if (!bbtool->configuration().isWithdrawn())
        XMoveResizeWindow(display, win, fx, fy, fwidth, fheight);
    else
        XResizeWindow(display, win, fwidth, fheight);

}

void FrameWindow::calcDesktopPosition(void)
{
    int column, row;
    if (bbtool->getResource()->position.vertical) {
        column = bbtool->numberOfDesktops() / bbtool->getResource()->rows + 1;
        if (column > current_column) {
            ldx = (bbtool->getResource()->desktopSize.width + 
                bbtool->resource->frame.bevelWidth) *
                (column - 1) + bbtool->getResource()->frame.bevelWidth;
            ldy = bbtool->resource->frame.bevelWidth;
            current_column = column;
        } else {
             ldx = (bbtool->getResource()->desktopSize.width + 
                bbtool->resource->frame.bevelWidth) *
                (current_column - 1) + bbtool->getResource()->frame.bevelWidth;
             ldy = (bbtool->getResource()->desktopSize.height +
                  bbtool->getResource()->frame.bevelWidth)*
                  (bbtool->numberOfDesktops() % bbtool->getResource()->rows - 1) + 
                  bbtool->getResource()->frame.bevelWidth;
        }
    } else {
        row = bbtool->numberOfDesktops() / bbtool->getResource()->columns + 1;
        if (row > current_column) {
            ldx = bbtool->resource->frame.bevelWidth;
            ldy = (bbtool->getResource()->desktopSize.height + 
                bbtool->resource->frame.bevelWidth) *
                (row - 1) + bbtool->getResource()->frame.bevelWidth;
            current_row = row;
        } else {
            ldx = (bbtool->getResource()->desktopSize.width + 
                 bbtool->resource->frame.bevelWidth) *
                 (bbtool->numberOfDesktops() % bbtool->getResource()->columns - 1) + 
                 bbtool->getResource()->frame.bevelWidth;
            ldy = (bbtool->getResource()->desktopSize.height + 
                bbtool->resource->frame.bevelWidth) *
                (row - 1) + bbtool->getResource()->frame.bevelWidth;
 
        }
 
    }


}

void FrameWindow::calcSize(void)
{

    if (bbtool->resource->position.vertical) {
        if (bbtool->numberOfDesktops() < bbtool->resource->columns) {
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
        if (bbtool->numberOfDesktops() < bbtool->getResource()->rows)
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

void FrameWindow::addSticky(PagerWindow *tmp)
{
  tmp->setSticky(true);
        
/*
  for (; desktop_it.current(); desktop_it++) {
    i++;

    if (bbtool->getCurrentDesktopNr()!=
      desktop_it.current()->desktop_nr) {
      WindowList *copy = new WindowList;
      copy->win= tmp->win;
      copy->width=tmp->width;
      copy->height=tmp->height;
      copy->x_position=tmp->x_position;
      copy->y_position=tmp->y_position;
      copy->icon=tmp->icon;
      copy->sticky=tmp->sticky;
      copy->focused=tmp->focused;
      copy->desktop_nr=desktop_it.current()->desktop_nr;
      bbtool->addFrameWindow(copy,desktop_it.current()->win,False);
      bbtool->windowList->insert(copy);
    }
  }*/
}

void FrameWindow::removeSticky(Window win,int keep_on_desktop)
{
  //tmp->setSticky(false);

  /*for (i=0;i<bbtool->getNumberOfDesktops();i++) {
    LinkedListIterator<WindowList> win_it(bbtool->windowList);
    for (; win_it.current(); win_it++) {
      if (win_it.current()->win==win &&
          win_it.current()->desktop_nr!=keep_on_desktop) {
        WindowList *old = win_it.current();
        bbtool->windowList->remove(old);
        XDestroyWindow(bbtool->getXDisplay(),old->pager_win);
        delete old;
      }
    }
  }*/
}

/*void FrameWindow::changeIconState(Window win) {

  LinkedListIterator<WindowList> win_it(bbtool->windowList);
  for (; win_it.current(); win_it++)
    if ((win_it.current()->win) == win)
      break;

 if (win_it.current()) {
    WindowList *tmp = win_it.current();
    
    if (!tmp->icon) {
      if (!tmp->sticky) {
        XUnmapWindow(bbtool->getXDisplay(),tmp->pager_win);
        tmp->icon=True;
      } else {
        LinkedListIterator<WindowList> win_it(bbtool->windowList);
        for (; win_it.current(); win_it++) {
          if (win_it.current()->win==tmp->win) {
            XUnmapWindow(bbtool->getXDisplay(),win_it.current()->pager_win);
            win_it.current()->icon=True;
          }
        }
      }
    } else  {
      if (!tmp->sticky) {
        XMapWindow(bbtool->getXDisplay(),tmp->pager_win);
        tmp->icon=False;
      } else {
        LinkedListIterator<WindowList> win_it(bbtool->windowList);
        for (; win_it.current(); win_it++) {
          if (win_it.current()->win==tmp->win) {
            XMapWindow(bbtool->getXDisplay(),win_it.current()->pager_win);
              win_it.current()->icon=False;
          }
        }
      }
    }
  }
}
*/

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
   if ((unsigned)event->data.l[0] == bbtool->wmDeleteWindow()) bbtool->shutdown();
}

void ToolWindow::MakeWindow(bool reconfigure) 
{
}

void ToolWindow::shutdown(void)
{
    quit();
}

/* NOTE: either move to PagerWindow or wminterface
void ToolWindow::configureNotifyEvent(const XConfigureEvent * const event)
{
    PagerWindow *pager = findPagerWindow(Event->xconfigure.window);

    if (pager) {
        int status = getWindowGeometry(pager);
        if (status == -1) break;
        double xdiv = static_cast<double>(resource->desktopSize.width) / getCurrentScreenInfo()->width();
        double ydiv = static_cast<double>(resource->desktopSize.height) / getCurrentScreenInfo()->height();
        pager->x(static_cast<int>(pager->x() * xdiv));
        pager->y(static_cast<int>(pager->y() * ydiv));
        pager->width(static_cast<unsigned int>(pager->width() * xdiv));
        if (!pager->isShaded())
            pager->height(static_cast<unsigned int>(pager->height() * ydiv));
        if (pager->width() == 0) pager->width(1);
        if (pager->height() == 0) pager->height(1);

        // resize pager window
        XMoveResizeWindow(XDisplay(), pager->window(),
                        pager->x(), pager->y(),
                        pager->width(), pager->height());

        if (!pager->isFocused() || resource->getFocusStyle() != texture)
            pager->clearFocus();
        else
            pager->setFocus();
    }
}
*/

