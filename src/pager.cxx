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
    bt::EventHandler(), bbtool(toolwindow), ewmh(toolwindow->ewmh())
{
    pwin = 0;
    win = _window;
    ewmh->readWMDesktop(_window, desktop_nr);
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
    bt::EWMH::AtomList states;
    bt::EWMH::AtomList::const_iterator it;

    ewmh->readWMState(win, states);
    for (it = states.begin(); it != states.end(); it++) {
        if ((*it) == ewmh->wmStateShaded()) {
            shaded = true; // window resized by configureNotify ?
        } 
//      handled by wmDesktop !
//      else if ((*it) == ewmh->wmStateSticky()) {
//      }
        else if ((*it) == ewmh->wmStateSkipPager()) {
            skip = true;
        }
        else if ((*it) == ewmh->wmStateHidden()) {
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
    {
        bbtool->removeEventHandler(pwin[i]) ;
        XDestroyWindow(display, pwin[i]);  
    }
    delete [] pwin;
    if (pixmap) bt::PixmapCache::release(pixmap);
    if (pixmap_focused && resource->getFocusStyle() == texture) bt::PixmapCache::release(pixmap_focused);
}

Window PagerWindow::window(int nr)
{
    if (static_cast<unsigned int>(nr) >= number_of_desktops)
    {
        return 0;
    }
    return pwin[nr];
}
        
Window PagerWindow::window(void) 
{ 
    Window win;
    if (!sticky) {
        win = pwin[0];
    } else {
        int nr = bbtool->getCurrentDesktopNr();
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


void PagerWindow::calcGeometry()
{
  // Find the geometry of the original window that we're representing in the pager.
  initWindowGeometry();

  // Find the scaling factors necessary to scale the original window down to our pager window.
  double xdiv = static_cast<double>(bbtool->getResource()->desktopSize.width) / bbtool->getCurrentScreenInfo()->width();
  double ydiv = static_cast<double>(bbtool->getResource()->desktopSize.height) / bbtool->getCurrentScreenInfo()->height();

  // Find the position of the pager window by scaling the original window position.
  pager_x = (int)(window_x * xdiv);
  pager_y = (int)(window_y * ydiv);

  // Set the unadjusted size of the pager window by scaling the original window size.
  pager_unfocus_width = (unsigned int)(window_width * xdiv);
  pager_unfocus_height = (unsigned int)(window_height * ydiv);
  pager_focus_width = (unsigned int)(window_width * xdiv);
  pager_focus_height = (unsigned int)(window_height * ydiv);

  // Because of the following:
  //
  //   1) we support different border widths for focused and unfocused windows
  //   2) an X window's width property does not include its border width
  //   3) we don't want our pager window's total width or height to change as
  //      its focus changes
  //
  // We must adjust the width and height property of the pager window by subtracting
  // out the appropriate border width.
  //
  pager_unfocus_width -= 2 * resource->pagerwin.inactiveWidth;
  pager_unfocus_height -= 2 * resource->pagerwin.inactiveWidth;
  pager_focus_width -= 2 * resource->pagerwin.activeWidth;
  pager_focus_height -= 2 * resource->pagerwin.activeWidth;

  // Reduce the pager window's height if it's shaded.
  if (shaded) {
    pager_unfocus_height = static_cast<unsigned int>(pager_unfocus_height * ydiv);
    pager_focus_height = static_cast<unsigned int>(pager_focus_height * ydiv);
  }

  // Our adjustments may have created negative or zero width and height properties,
  // so replace these with saner values.
  if (pager_unfocus_width <= 0)
    pager_unfocus_width = 1;
  if (pager_unfocus_height <= 0)
    pager_unfocus_height = 1;
  if (pager_focus_width <= 0)
    pager_focus_width = 1;
  if (pager_focus_height <= 0)
    pager_focus_height = 1;
}

void PagerWindow::buildWindow(bool reconfigure)
{
//    if (skip) return;       // don't build window if state skipPagerWindow is set.
  calcGeometry();

    if (desktop_nr == static_cast<unsigned int>(-1)) {  // on all desktops
        sticky = true;
    }
    pixmap = bt::PixmapCache::find(screen, resource->pagerwin.texture, 
             pager_unfocus_width, pager_unfocus_height);
    if (resource->getFocusStyle()==texture) {
        pixmap_focused = bt::PixmapCache::find(screen, resource->pagerwin.focusedTexture,
            pager_focus_width, pager_focus_height);
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

    redraw();
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
  unsigned long create_mask = CWBackPixmap | CWEventMask ;
    XSetWindowAttributes attrib;

    attrib.background_pixmap = ParentRelative;
    attrib.event_mask = ExposureMask;
    
    m_pDesktop = bbtool->findDesktopWindow(nr);
    if (!sticky) nr = 0;
    if (m_pDesktop == NULL) { //not on existing window
        return;
    }

  unsigned int pager_width = ( focused ? pager_focus_width : pager_unfocus_width );
  unsigned int pager_height = ( focused ? pager_focus_height : pager_unfocus_height );

    if (!reconfigure) {
        pwin[nr] = XCreateWindow(display, m_pDesktop->window(),
                 pager_x, pager_y, pager_width, pager_height,
                 0, bbtool->getCurrentScreenInfo()->depth(), 
                 InputOutput, bbtool->getCurrentScreenInfo()->visual(), 
                 create_mask, &attrib);
        bbtool->insertEventHandler(pwin[nr], this);
    } else
        XMoveResizeWindow(display, pwin[nr], pager_x, pager_y, pager_width, pager_height);


    //    redraw(); 
    
    if (!hidden /*&& !iconic*/ && !skip)
        XMapWindow(display, pwin[nr]);

    XClearWindow(display, pwin[nr]);
}

void PagerWindow::propertyNotifyEvent(const XPropertyEvent * const event)
{
    if (event->atom == ewmh->wmDesktop()) {
      //unsigned int desktop_nr;
        ewmh->readWMDesktop(event->window, desktop_nr);
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
    else if (event->atom == ewmh->wmState()) {
        bt::EWMH::AtomList states;
        bt::EWMH::AtomList::const_iterator it;
        bool skip_state = false;
        bool shaded_state = false;
        bool hidden_state = false;
        ewmh->readWMState(event->window, states);
        for (it = states.begin(); it != states.end(); it++) {
            if ((*it) == ewmh->wmStateShaded()) {
                shaded_state = true;
            } 
//          handled by wmDesktop !
//          if ((*it) == ewmh->wmStateSticky()) {
//          }
            if ((*it) == ewmh->wmStateSkipPager()) {
                skip_state = true;
            }
            if ((*it) == ewmh->wmStateHidden()) {
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
      redraw();
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
   focused = true;
   redraw();
}

void PagerWindow::clearFocus(void)
{
    focused = false;
    redraw();
}

bt::Texture PagerWindow::getTexture(void) 
{ 
    return resource->pagerwin.texture; 
}

bt::Texture PagerWindow::getFocusedTexture(void) 
{ 
    return resource->pagerwin.focusedTexture; 
}

void PagerWindow::redraw(void)
{
  calcGeometry();

  bt::Rect u(0, 0, width(), height());

    unsigned int i;
    for (i = 0; i < number_of_desktops; i++)  // number_of_desktops=1 for non-sticky windows
    {
      if (sticky) {
        m_pDesktop = bbtool->findDesktopWindow(i);
      }
      else {
        m_pDesktop = bbtool->findDesktopWindow(desktop_nr);
      }

      unsigned int pager_width = ( focused ? pager_focus_width : pager_unfocus_width );
      unsigned int pager_height = ( focused ? pager_focus_height : pager_unfocus_height );

      XMoveResizeWindow(display, pwin[i], pager_x, pager_y, pager_width, pager_height);

      if (focused) {
        XSetWindowBorderWidth(display, pwin[i], resource->pagerwin.activeWidth);
        XSetWindowBorder(display, pwin[i], resource->pagerwin.activeColor.pixel(screen));
      }
      else {
        XSetWindowBorderWidth(display, pwin[i], resource->pagerwin.inactiveWidth);
        XSetWindowBorder(display, pwin[i], resource->pagerwin.inactiveColor.pixel(screen));
      }

        if (resource->getFocusStyle() == texture && focused)
        {
                if (pixmap_focused == ParentRelative)
                {
                    if (m_pDesktop->pixmap() == ParentRelative)
                    {
                        bt::Rect t(-(pager_x + m_pDesktop->desktopX()), 
                                   -(pager_y + m_pDesktop->desktopY()), 
                                   bbtool->frameWindow()->width(), bbtool->frameWindow()->height());
                        bt::drawTexture(screen,
                                    resource->frame.texture,
                                    pwin[i], 
                                    t, u, bbtool->frameWindow()->pixmap());
                    }
                    else
                    {                   
                        bt::Rect t(-pager_x, -pager_y, m_pDesktop->desktopWidth(), m_pDesktop->desktopHeight());
                        bt::drawTexture(screen,
                                    resource->getDesktopFocusStyle() == texture && m_pDesktop->focused() ? resource->desktopwin.focusedTexture : resource->desktopwin.texture,
                                    pwin[i], 
                                    t, u, m_pDesktop->pixmap());
                    }
                }
                else
                {
                    bt::drawTexture(screen,
                            getFocusedTexture(),
                            pwin[i], 
                            u, u, pixmap_focused);
                }
        }
        else
        {
                if (pixmap == ParentRelative)
                {
                    if (m_pDesktop->pixmap() == ParentRelative)
                    {
                        bt::Rect t(-(pager_x + m_pDesktop->desktopX()), 
                                   -(pager_y + m_pDesktop->desktopY()), 
                                   bbtool->frameWindow()->width(), bbtool->frameWindow()->height());
                        bt::drawTexture(screen,
                                    resource->frame.texture,
                                    pwin[i], 
                                    t, u, bbtool->frameWindow()->pixmap());
                    }
                    else
                    {
                        bt::Rect t(-pager_x, -pager_y, m_pDesktop->desktopWidth(), m_pDesktop->desktopHeight());
                        bt::drawTexture(screen,
                                    resource->getDesktopFocusStyle() == texture && m_pDesktop->focused() ? resource->desktopwin.focusedTexture : resource->desktopwin.texture,
                                    pwin[i], 
                                    t, u, m_pDesktop->pixmap());
                    }
                }
                else
                { 
                    bt::drawTexture(screen,
                                getTexture(),
                                pwin[i], 
                                u, u, pixmap);
                }
        } 
    }
}

void PagerWindow::exposeEvent(const XExposeEvent * const event)
{
    redraw();
}
