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
                 m_focused(false),
                 m_pixmap(0),
                 m_pixmapFocused(0)
{
    screen = bbtool->getCurrentScreen();
    display = bbtool->XDisplay();
    desktop_nr = _desktop_nr;
    moved = false;
    buildWindow(false);
    bbtool->insertEventHandler(win, this);
    grabbedWindow = 0;
}


DesktopWindow::~DesktopWindow(void)
{
    bbtool->removeEventHandler(win);
    XDestroyWindow(bbtool->XDisplay(), win);
    if (m_pixmap) bt::PixmapCache::release(m_pixmap);
    if (m_pixmapFocused &&
        bbtool->getResource()->getDesktopFocusStyle() == texture)
    {
        bt::PixmapCache::release(m_pixmapFocused);
    }
}

void DesktopWindow::reconfigure(void)
{
    buildWindow(true);
}

void DesktopWindow::buildWindow(bool reconfigure)
{
    XSetWindowAttributes attrib;
    unsigned long create_mask = CWBackPixmap | CWEventMask;

    attrib.background_pixmap = ParentRelative;
    attrib.event_mask = ButtonPressMask | ButtonReleaseMask | ExposureMask |
        FocusChangeMask | StructureNotifyMask|
        SubstructureRedirectMask | ButtonMotionMask;

    calcGeometry();
    bbtool->frameWindow()->resize();
    m_pixmap = bt::PixmapCache::find(bbtool->getCurrentScreen(),
                                     bbtool->getResource()->desktopwin.texture,
                                     _desktop_width, _desktop_height, m_pixmap);
    if (bbtool->getResource()->getDesktopFocusStyle() == texture)
    {
        m_pixmapFocused = bt::PixmapCache::find(bbtool->getCurrentScreen(),
                                                bbtool->getResource()->desktopwin.focusedTexture,
                                                _desktop_width, _desktop_height, m_pixmapFocused);
    }
    if (!reconfigure)
    {
        win = XCreateWindow(display,
                            bbtool->frameWindow()->window(),
                            _window_x, _window_y,
                            _desktop_width,
                            _desktop_height, 0,
                            bbtool->getCurrentScreenInfo()->depth(),
                            InputOutput,
                            bbtool->getCurrentScreenInfo()->visual(),
                            create_mask,
                            &attrib);
    }
    redraw();
    XClearWindow(display, bbtool->frameWindow()->window());
    XMapWindow(display, bbtool->frameWindow()->window());
    XMapSubwindows(display, bbtool->frameWindow()->window());
    desktop_id = desktop_nr;
}


void DesktopWindow::calcGeometry(void)
{
    // We will define the geometry of 3 rectangles:
    //
    //   1) desktop - the area inside which all the pager windows will reside. This
    //                is also the area that will be drawn using the desktop texture.
    //                We do not want the size or position of this area to change as
    //                the user moves from one desktop to another, that is, as the
    //                desktop switches between its active (aka focused) and inactive
    //                (aka unfocused) states.
    //   2) window - the area encompassing both the desktop area as well as the
    //               desktop border, if any. Because we support different border
    //               widths for active and inactive desktops, and because we want
    //               our desktop geometry to remain constant, the window geometry
    //               may have to change as the desktop switches between its active
    //               and inactive states.
    //   3) cell - the area encompassing the larger of (a) the window area's size
    //             when the desktop is active and (b) the window area's size when
    //             the desktop is inactive. The geometry of this area will remain
    //             constant as the desktop switches between its active and inactive
    //             states.
    //
    // These rectangles will differ in size only when the user has specified
    // different border widths for the active and inactive desktops.


    // ----- define sizes -----

    _desktop_width = bbtool->getResource()->desktopSize.width; //bbtool->getResource()->desktopSize.width;
    _desktop_height = bbtool->getResource()->desktopSize.height;

    if (m_focused)
    {
        _window_width = _desktop_width +
                        2 * bbtool->getResource()->desktopwin.activeWidth;
        _window_height = _desktop_height +
                        2 * bbtool->getResource()->desktopwin.activeWidth;
    }
    else
    {
        _window_width = _desktop_width +
                        2 * bbtool->getResource()->desktopwin.inactiveWidth;
        _window_height = _desktop_height +
                        2 * bbtool->getResource()->desktopwin.inactiveWidth;
    }

    _cell_width = _desktop_width +
                  2 * std::max(bbtool->getResource()->desktopwin.inactiveWidth,
                  bbtool->getResource()->desktopwin.activeWidth);
    _cell_height = _desktop_height +
                  2 * std::max(bbtool->getResource()->desktopwin.inactiveWidth,
                  bbtool->getResource()->desktopwin.activeWidth);


  // ----- define positions -----

    unsigned int desktop_margin = bbtool->getResource()->frame.desktopMargin;
    unsigned int frame_margin = bbtool->getResource()->frame.bevelWidth;
    unsigned int frame_border = bbtool->getResource()->frame.texture.borderWidth();

    // figure out what row and column we occupy
    int column, row;
    if (bbtool->getResource()->position.horizontal)
    {
        // horizontal.
        row = (desktop_nr) / bbtool->getResource()->columns;
        column = (desktop_nr) % bbtool->getResource()->columns;
    }
    else
    {
        // vertical
        row = (desktop_nr) % bbtool->getResource()->rows;
        column = (desktop_nr) / bbtool->getResource()->rows;
    }

    // position the cell (relative to the frame origin) based on the current row/column
    _cell_x = frame_border + frame_margin +
              column * (desktop_margin + _cell_width);
    _cell_y = frame_border + frame_margin +
              row * (desktop_margin + _cell_height);

    // position the window so that it is centered within the cell
    _window_x = _cell_x + (_cell_width - _window_width) / 2;
    _window_y = _cell_y + (_cell_height - _window_height) / 2;

    // position the dektop so that it is centered within the window
    _desktop_x = _window_x + (_window_width - _desktop_width) / 2;
    _desktop_y = _window_y + (_window_height - _desktop_height) / 2;
}

void DesktopWindow::setFocus(void)
{
    m_focused = true;
    redraw();
}

void DesktopWindow::clearFocus(void)
{
    m_focused = false;
    redraw();
}

void DesktopWindow::buttonPressEvent(const XButtonEvent * const event)
{
    if (event->button ==
        static_cast<unsigned>(bbtool->getResource()->getDesktopChangeButton()))
    {
        bbtool->wminterface->changeDesktop(desktopId());
    }
    if (event->button ==
        static_cast<unsigned>(bbtool->getResource()->getWindowRaiseButton()))
    {
        PagerWindow *pager = bbtool->findPPagerWindow(event->subwindow);
        if (pager)
            XRaiseWindow(display, pager->realWindow());
    }
    if (event->button ==
        static_cast<unsigned>(bbtool->getResource()->getWindowFocusButton()))
    {
        PagerWindow *pager = bbtool->findPPagerWindow(event->subwindow);
        if (pager)
            bbtool->wminterface->setWindowFocus(pager->realWindow(), event->time);
    }
    if (event->button ==
        static_cast<unsigned>(bbtool->getResource()->getWindowMoveButton()))
    {
//        DesktopWindow *desktop = findDesktopWindow(event->window);
        PagerWindow *pager = bbtool->findPPagerWindow(event->subwindow);

        if (pager)
        {
            XSetWindowAttributes attrib;
            unsigned long create_mask = CWBackPixmap|CWCursor;

            attrib.background_pixmap = ParentRelative;
            attrib.cursor = XCreateFontCursor(display, XC_left_ptr);

            grabbedWindow = XCreateWindow(display, bbtool->frameWindow()->window(),
                                          pager->x() + desktopX(),
                                          pager->y() + desktopY(),
                                          pager->width(), pager->height(),
                                          0, bbtool->getCurrentScreenInfo()->depth(),
                                          InputOutput,
                                          bbtool->getCurrentScreenInfo()->visual(),
                                          create_mask,&attrib);

            grabbed_x = desktopX() + pager->x() - event->x;
            grabbed_y = desktopY() + pager->y() - event->y;
            XMapWindow(display,grabbedWindow);

            if (pager->isFocused())
            {
                XSetWindowBorderWidth(display,
                                      grabbedWindow,
                                      bbtool->getResource()->pagerwin.activeWidth);
                XSetWindowBorder(display,
                                 grabbedWindow,
                                 bbtool->getResource()->pagerwin.activeColor.pixel(screen));
            }
            else
            {
                XSetWindowBorderWidth(display,
                                      grabbedWindow,
                                      bbtool->getResource()->pagerwin.inactiveWidth);
                XSetWindowBorder(display,
                                 grabbedWindow,
                                 bbtool->getResource()->pagerwin.inactiveColor.pixel(screen));
            }

            if (bbtool->getResource()->getFocusStyle() == texture && pager->isFocused())
            {
                bt::Rect u(0, 0, pager->width(), pager->height());
                if (pager->getPixmap() == ParentRelative)
                {
                    if (m_pixmap == ParentRelative)
                    {
                        bt::Rect t(-(pager->x() + desktopX()),
                                   -(pager->y() + desktopY()),
                                   bbtool->frameWindow()->width(),
                                   bbtool->frameWindow()->height());
                        bt::drawTexture(screen,
                                        bbtool->getResource()->frame.texture,
                                        grabbedWindow,
                                        t, u, bbtool->frameWindow()->pixmap());
                    }
                    else
                    {
                        bt::Rect t(-pager->x(), -pager->y(), desktopWidth(), desktopHeight());
                        bt::drawTexture(screen,
                                        (bbtool->getResource()->getDesktopFocusStyle() ==
                                         texture && m_focused ?
                                         bbtool->getResource()->desktopwin.focusedTexture :
                                         bbtool->getResource()->desktopwin.texture),
                                        grabbedWindow, t, u,
                                        pixmap());
                    }
                }
                else
                {
                    bt::drawTexture(screen,
                                    pager->getFocusedTexture(),
                                    grabbedWindow, u, u,
                                    pager->getFocusedPixmap());
                }
            }
            else
            {
                bt::Rect u(0, 0, pager->width(), pager->height());
                if (pager->getPixmap() == ParentRelative)
                {
                    if (m_pixmap == ParentRelative)
                    {
                        bt::Rect t(-(pager->x() + desktopX()),
                                   -(pager->y() + desktopY()),
                                   bbtool->frameWindow()->width(),
                                   bbtool->frameWindow()->height());
                        bt::drawTexture(screen,
                                        bbtool->getResource()->frame.texture,
                                        grabbedWindow, t, u,
                                        bbtool->frameWindow()->pixmap());
                    }
                    else
                    {
                        bt::Rect t(-pager->x(), -pager->y(), desktopWidth(), desktopHeight());
                        bt::drawTexture(screen,
                                        (bbtool->getResource()->getDesktopFocusStyle() ==
                                         texture && m_focused ?
                                         bbtool->getResource()->desktopwin.focusedTexture :
                                         bbtool->getResource()->desktopwin.texture),
                                        grabbedWindow, t, u,
                                        pixmap());
                    }
                }
                else
                {
                    bt::drawTexture(screen,
                                    pager->getTexture(),
                                    grabbedWindow, u, u,
                                    pager->getPixmap());
                }
            }

            moveWindow = pager;
            realWindow = pager->realWindow();
            pagerWindow = pager->window();
            moved = false;
//           XMapWindow(display,grabbedWindow);
        } else {
            grabbed_x = 0;
            grabbed_y = 0;
            grabbedWindow = 0;
        }
    }

}

void DesktopWindow::buttonReleaseEvent(const XButtonEvent * const event)
{
    if (event->button ==
        static_cast<unsigned>(bbtool->getResource()->getWindowMoveButton())) {
        if (grabbedWindow != 0)
        {
            if (!moved)
            {
                XDestroyWindow(display, grabbedWindow);
                grabbedWindow = 0;
                return;
            }
            list<DesktopWindow *>::iterator it =
                bbtool->desktopWindowList().begin();
            for (; it != bbtool->desktopWindowList().end(); it++)
            {
                if (move_x > (*it)->windowX() - (*it)->windowWidth() &&
                    move_x <= (*it)->windowX() + (*it)->windowWidth() &&
                    move_y > (*it)->windowY() - (*it)->windowHeight() &&
                    move_y < (*it)->windowY() + (*it)->windowHeight())
                    break;
            }
            if (it != bbtool->desktopWindowList().end())
            {
                const bt::Rect &head = bbtool->getCurrentHead();

                if (!moveWindow->isSticky())
                    bbtool->wminterface->sendWindowToDesktop(realWindow,
                                                             (*it)->desktopId());
                double xdiv =
                    static_cast<double>(bbtool->getResource()->desktopSize.width) /
                    head.width();
                double ydiv =
                    static_cast<double>(bbtool->getResource()->desktopSize.height) /
                    head.height();

                int x = static_cast<int>((move_x - (*it)->desktopX()) / xdiv);
                int y = static_cast<int>((move_y - (*it)->desktopY()) / ydiv);
                x += head.x();
                y += head.y();
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
        XMoveWindow(display, grabbedWindow, move_x, move_y);
        redraw();
    }
}

void DesktopWindow::redraw(void)
{
    calcGeometry();
    m_pixmap = bt::PixmapCache::find(bbtool->getCurrentScreen(),
                                     bbtool->getResource()->desktopwin.texture,
                                     _desktop_width, _desktop_height, m_pixmap);
    if (bbtool->getResource()->getDesktopFocusStyle() == texture)
    {
        m_pixmapFocused = bt::PixmapCache::find(bbtool->getCurrentScreen(),
                                                bbtool->getResource()->desktopwin.focusedTexture,
                                                _desktop_width, _desktop_height, m_pixmapFocused);
    }

    XMoveResizeWindow(bbtool->XDisplay(), win,
                      _window_x, _window_y,
                      _desktop_width, _desktop_height);

    if (m_focused)
    {
        XSetWindowBorderWidth(display, win,
                              bbtool->getResource()->desktopwin.activeWidth);
        XSetWindowBorder(display, win,
                         bbtool->getResource()->desktopwin.activeColor.pixel(screen));
    }
    else
    {
        XSetWindowBorderWidth(display, win,
                              bbtool->getResource()->desktopwin.inactiveWidth);
        XSetWindowBorder(display, win,
                         bbtool->getResource()->desktopwin.inactiveColor.pixel(screen));
    }

    bt::Rect u(0, 0, desktopWidth(), desktopHeight());

    if (bbtool->getResource()->getDesktopFocusStyle() == texture && m_focused)
    {
        if (m_pixmapFocused == ParentRelative)
        {
            bt::Rect t(-desktopX(), -desktopY(),
                       bbtool->frameWindow()->width(),
                       bbtool->frameWindow()->height());
            bt::drawTexture(screen,
                            bbtool->getResource()->frame.texture,
                            win, t, u, bbtool->frameWindow()->pixmap());
        }
        else
        {
            bt::drawTexture(screen,
                            bbtool->getResource()->desktopwin.focusedTexture,
                            win, u, u, m_pixmapFocused);
        }
    }
    else
    {
        if (m_pixmap == ParentRelative)
        {
            bt::Rect t(-desktopX(),
                       -desktopY(),
                       bbtool->frameWindow()->width(),
                       bbtool->frameWindow()->height());
            bt::drawTexture(screen,
                            bbtool->getResource()->frame.texture,
                            win, t, u, bbtool->frameWindow()->pixmap());
        }
        else
        {
            bt::drawTexture(screen,
                            bbtool->getResource()->desktopwin.texture,
                            win, u, u, m_pixmap);
        }
    }
}

void DesktopWindow::exposeEvent(const XExposeEvent * const event)
{
    redraw();
}
