// desktop.cxx for bbpager - an pager tool for Blackbox.
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


#include "desktop.h"

extern "C" {
#include <X11/cursorfont.h>
}

#include <iostream>

using std::list;
using std::cout;
using std::endl;

DesktopWindow::DesktopWindow(ToolWindow *toolwindow, unsigned int _desktop_nr):
                 bt::EventHandler(), bbtool(toolwindow),
                 m_focused(false)
{
    screen = bbtool->getCurrentScreen();
    display = bbtool->XDisplay();
    resource = bbtool->getResource(); 
    desktop_nr = _desktop_nr;
    pixmap = pixmap_focused = 0;
    moved = false;
    buildWindow(false);
    bbtool->insertEventHandler(win, this);
    grabbedWindow = 0;
}


DesktopWindow::~DesktopWindow(void)
{
    bbtool->removeEventHandler(win);
    XDestroyWindow(bbtool->XDisplay(), win);
    if (pixmap) bt::PixmapCache::release(pixmap);
    if (pixmap_focused) bt::PixmapCache::release(pixmap_focused);
}

void DesktopWindow::reconfigure(void)
{
    buildWindow(true);
}

void DesktopWindow::buildWindow(bool reconfigure) 
{
    XSetWindowAttributes attrib;
    unsigned long create_mask = CWBackPixmap | CWEventMask | CWBorderPixel;

    attrib.background_pixmap = ParentRelative;
    attrib.border_pixel = resource->desktopwin.activeColor.pixel(screen);
    attrib.event_mask = ButtonPressMask | ButtonReleaseMask | ExposureMask |
                FocusChangeMask | StructureNotifyMask|
                SubstructureRedirectMask | ButtonMotionMask;

    _width = resource->desktopSize.width;
    _height = resource->desktopSize.height;
  
    calcPosition();
    bbtool->frameWindow()->resize();

    pixmap = bt::PixmapCache::find(bbtool->getCurrentScreen(), 
             resource->desktopwin.texture, _width, _height, pixmap);
    if (pixmap == 0 && 
        resource->desktopwin.texture.texture() != (bt::Texture::Flat | bt::Texture::Solid)) 
    {
        cout << "Error: cannot create desktop pixmap with texture: \"";
        cout << resource->desktopwin.texture.description() << "\"" << endl;
    }

    if (resource->getDesktopFocusStyle() == texture) {
        pixmap_focused = bt::PixmapCache::find(bbtool->getCurrentScreen(), 
                 resource->desktopwin.focusedTexture, _width, _height, pixmap_focused);
        if (pixmap_focused == 0 &&
          resource->desktopwin.focusedTexture.texture() != (bt::Texture::Flat | bt::Texture::Solid))
        {
            cout << "Error: cannot create focused desktop pixmap with texture: \"";
            cout << resource->desktopwin.focusedTexture.description() << "\"" << endl;
        }
    }
    if (!reconfigure)
        win = XCreateWindow(display, bbtool->frameWindow()->window(), _x, _y, _width,
                           _height, 0, bbtool->getCurrentScreenInfo()->depth(),
                           InputOutput, bbtool->getCurrentScreenInfo()->visual(),
                           create_mask, &attrib);
    else
        XMoveResizeWindow(bbtool->XDisplay(), win, _x, _y, _width, _height);

    bt::Rect u(0, 0, width(), height());
    bt::drawTexture(screen,
                    resource->desktopwin.texture,
                    win, 
                    u, u, pixmap);
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
    m_focused = true;
    if (resource->getDesktopFocusStyle() == border)
        XSetWindowBorderWidth(display, win, 1);
    else
    {
        bt::Rect u(0, 0, width(), height());
        bt::drawTexture(screen,
                        resource->desktopwin.focusedTexture,
                        win, 
                        u, u, pixmap_focused);
    }

}

void DesktopWindow::clearFocus(void)
{
    m_focused = false;
    if (resource->getDesktopFocusStyle() == border)
        XSetWindowBorderWidth(bbtool->XDisplay(), win, 0);
    else
    {
        bt::Rect u(0, 0, width(), height());
        bt::drawTexture(screen,
                        resource->desktopwin.texture,
                        win, 
                        u, u, pixmap_focused);
    }
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
            {
                bt::Rect u(0, 0, pager->width(), pager->height());
                bt::drawTexture(screen,
                        pager->getTexture(),
                        grabbedWindow, 
                        u, u, pager->getPixmap());
            }
            else
            {
                bt::Rect u(0, 0, pager->width(), pager->height());
                bt::drawTexture(screen,
                        pager->getFocusedTexture(),
                        grabbedWindow, 
                        u, u, pager->getFocusedPixmap());
            }
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
            list<DesktopWindow *>::iterator it = bbtool->desktopWindowList().begin();
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
        redraw();
    }
}

void DesktopWindow::redraw(void)
{
    bt::Rect u(0, 0, width(), height());
    if (resource->getDesktopFocusStyle() == border)
    {
        bt::drawTexture(screen,
                        resource->desktopwin.texture,
                         win, 
                         u, u, pixmap);
    }
    
    if (m_focused)
    {
        if (resource->getDesktopFocusStyle() == border)
            XSetWindowBorderWidth(display, win, 1);
        else
        {
            bt::Rect u(0, 0, width(), height());
            bt::drawTexture(screen,
                        resource->desktopwin.focusedTexture,
                        win, 
                        u, u, pixmap_focused);
        }
    }
    else
    {
        if (resource->getDesktopFocusStyle() == border)
            XSetWindowBorderWidth(display, win, 1);
    }
}

void DesktopWindow::exposeEvent(const XExposeEvent * const event)
{
    redraw();
}



