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

#include "bbpager.hh"
#include "version.h"
#include <stdio.h>

DesktopWindow::DesktopWindow(ToolWindow *toolwindow):
	bbtool(toolwindow)
{
	buildWindow(false);
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
		if ((*it).desktopId == id) {
		  	bbtool->pagerWindowList().remove(it);
			delete (*it);
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
	int row, column;
	unsigned long create_mask = CWBackPixmap|CWCursor|CWEventMask|CWBorderPixel;

	attrib.background_pixmap = ParentRelative;
	attrib.border_pixel = bbtool->getResource()->desktopwin.activeColor.getPixel();
	attrib.cursor =  XCreateFontCursor(display, XC_left_ptr); //getSessionCursor();
	attrib.event_mask = ButtonPressMask | ButtonReleaseMask | ExposureMask |
			    FocusChangeMask | StructureNotifyMask|
			    SubstructureRedirectMask | ButtonMotionMask;

	_width = bbtool->getResource()->desktopSize.width;
	_height = bbtool->getResource()->desktopSize.height;
  
	bbtool->frameWindow().resize();

	pixmap = bt::PixmapCache::find(bbtool->getCurrentScreen(), 
		 bbtool->getResource()->desktopwin.texture, _width, _height);

	if (resource->getDesktopFocusStyle() == texture)
		pixmap = bt::PixmapCache::find(bbtool->getCurrentScreen(), 
			 bbtool->getResource()->desktopwin.focusedTexture, _width, _height);

	if (!reconfigure)
		win = XCreateWindow(bbtool->XDisplay(), bbtool->frameWindow.window(), _x, _y, _width,
                           _height, 0, bbtool->getCurrentScreenInfo()->depth(),
                           InputOutput, bbtool->getCurrentScreenInfo()->visual(),
                           create_mask, &attrib);
	else
		XMoveResizeWindow(bbtool->XDisplay(), win, _x, _y, _width, _height);

	XSetWindowBackgroundPixmap(bbtool->XDisplay(), win, pixmap.desktop);
	XMapSubwindows(bbtool->XDisplay(), framewin);
	XClearWindow(bbtool->XDisplay(), framewin);
	XClearWindow(bbtool->XDisplay(), win);
	desktop_nr = bbtool->desktop_nr++;
}

void DesktopWindow::setFocus(void)
{
	if (bbtool->getResource()->getDesktopFocusStyle() == border)
		XSetWindowBorderWidth(bbtool->XDisplay(), win, 1);
	else
		XSetWindowBackgroundPixmap(bbtool->XDisplay(), win, pixmap_focused);

	XClearWindow(bbtool->XDisplay(),win);

}

void DesktopWindow::clearFocus(void)
{
	if (bbtool->getResource->getDesktopFocusStyle() == border)
		XSetWindowBorderWidth(bbtool->XDisplay(), win, 0);
	else
		XSetWindowBackgroundPixmap(bbtool->XDisplay(), win, pixmap);

	XClearWindow(bbtool->XDisplay(), win);
}



PagerWindow::PagerWindow(ToolWindow *toolwindow, Window _window):
	bbtool(toolwindow)
{
	win = _window;
	buildWindow(false);
}

PagerWindow::~PagerWindow(void)
{

    	XDestroyWindow(XDisplay(), pwin);
	if (pixmap) bt::PixmapCache::release(pixmap);
	if (pixmap_focused) bt::PixmapCache::release(pixmap_focused);
}

void PagerWindow::buildWindow(bool reconfigure)
{
	XSetWindowAttributes attrib;
	unsigned long create_mask = CWBackPixmap|CWCursor|CWBorderPixel;
	double xdiv,ydiv;

	xdiv = 10;
	ydiv = 10;

	bbtool->desktopWindow().find(desktop_nr);
	//if (!desktopWin) {
	//	LinkedListIterator<DesktopList> desktop_it(desktopList);
	//	desktop_it.set(window->desktop_nr);
	//	desktopWin = desktop_it.current()->win;
	//}

	attrib.background_pixmap = ParentRelative;
	attrib.border_pixel=resource->pagerwin.inactiveColor.getPixel();
	attrib.cursor = getSessionCursor();

	xdiv = (double)bbtool->getResource()->desktopSize.width / 
		bbtool->getCurrentScreenInfo()->width();
	ydiv = (double)resource->desktopSize.height / getCurrentScreenInfo()->getHeight();
	pager_x = (int)(x_position * xdiv);
	pager_y = (int)(y_position * ydiv);
	pager_width = (unsigned int)(window->width * xdiv);
	pager_height=(unsigned int)(window->height * ydiv);
	if (pager_width == 0)   
		window->pager_width = 1;
	if (window->pager_height == 0) 
	  	window->pager_height = 1;

  
	if (!reconfigure)
		pwin = XCreateWindow(bbtool->XDisplay(), desktop_window,
			     pager_x, pager_y, pager_width, pager_height,
			     1, bbtool->getCurrentScreenInfo()->depth(), 
			     InputOutput, bbtool->getCurrentScreenInfo()->visual(), 
			     create_mask,&attrib);
	else
		XMoveResizeWindow(getXDisplay(), pwin, pager_x, pager_y, pager_width, pager_height);

	XSelectInput(XDisplay(), win, PropertyChangeMask|StructureNotifyMask);

	pixmap.window =	getImageControl()->renderImage(resource->desktopSize.width,
                  resource->desktopSize.height,
                  &resource->pagerwin.texture);

	if (resource->getFocusStyle()==texture)
		pixmap.focusedWindow =	
			getImageControl()->renderImage(resource->desktopSize.width,
                  resource->desktopSize.height,
                  &resource->pagerwin.focusedTexture);

	
  	if (!window->focused)
		XSetWindowBackgroundPixmap(XDisplay(), pwin, pixmap.window);
	else
		XSetWindowBackgroundPixmap(getXDisplay(), pwin, pixmap.focusedWindow);
	
	if (!window->icon)
		XMapWindow(getXDisplay(),window->pager_win);

	XClearWindow(getXDisplay(),window->pager_win );

}

void PagerWindow::reconfigure(void)
{
	buildWindow(true);  
}

int PagerWindow::initWindowGeometry(void) 
{
	unsigned int border_width, depth;
	Window root_return, child_return;
	int x_return, y_return;
	int status;

	status = XGetGeometry(bbtool->XDisplay(), win, &root_return, &x_return,
                        &y_return, &_width, &_height,
                        &border_width, &depth);
	if (status) {
		XTranslateCoordinates(bbtool->XDisplay(), win, root_return, x_return,
                          y_return, &x_position, &y_position, &child_return);
		return 1;
	}
	return 0;
}

void PagerWindow::setFocus(void)
{
	if (bbtool->getResource()->getFocusStyle() == border)
		XSetWindowBorder(bbtool->XDisplay(), pwin,
				 bbtool->getResource()->pagerwin.activeColor.getPixel());
	else 
		XSetWindowBackgroundPixmap(bbtool->XDisplay(), pwin, pixmap_focused);
    
		XClearWindow(bbtool->XDisplay(), pwin);
		focused = true;
	
}

void PagerWindow::clearFocus(void)
{

	if (bbtool->getResource->getFocusStyle() == border)
		XSetWindowBorder(XDisplay(), pwin,
				bbtool->getResource()->pagerwin.inactiveColor.getPixel());
	else
		XSetWindowBackgroundPixmap(XDisplay(), pwin, pixmap);

      	focused = false;
      
      	XClearWindow(XDisplay(), pwin);
}


ToolWindow::ToolWindow(Configuration cml_options):
	bt::Application(cml_options.appName(), cml_options.displayName().c_str(), false),
	current_screen_info(display().screenInfo(DefaultScreen(XDisplay()))),
	_config(cml_options)

{
  resource = new Resource(this);
  wminterface = new WMInterface(this);

  desktop_nr = 0;
  current_desktop_nr = -1;
  wm_init = false;
  number_of_desktops = 0;
  row_last = column_last = 0;
//  MakeWindow(false);
  wminterface->moduleInit();
}

ToolWindow::~ToolWindow() {
	delete windowList;
	delete desktopList;
}

void ToolWindow::moveWinToDesktop(Window win, DesktopWindow *desktop) 
{

	PagerWindow pager_window = findPagerWindow(win);
  
//  LinkedListIterator<WindowList> win_it(windowList);
//  for (; win_it.current(); win_it++)
//    if ((win_it.current()->win) == win)
//      break;
	if (pager_window) {
		if ((desktop->desktop_nr != pager_window.desktopNr()) & (!pager_window.isSticky())) {

			XUnmapWindow(XDisplay(), pager_window.pagerWindow());
			XReparentWindow(XDisplay(), pager_window.pagerWindow(),
					desktop.window(), pager_window.x(), pager_window.y());
			XMapWindow(XDisplay(), pager_window.pagerWindow());
				   pager_window.setDesktopNr(desktop_nr);
		}
	}
}

PagerWindow &ToolWindow::findPagerWindow(Window win)
{
	std::list<PagerWindow>::iterator it = pager_window_list.begin();

	for (; it != pager_window_list.end(); it++) {
		if ((*it).window == win) {
			return (*it);
		}
	}
	return NULL;
}



void ToolWindow::reconfigure(void) 
{
	resource->Reload();

	MakeWindow(true);
  
	desktop_nr = 0;

	std::list<DesktopWindow>::iterator dit = desktop_window_list.begin();
	for (; dit != desktop_window_list.end(); dit++) {
		(*dit).reconfigure();
	}
  	std::list<PagerWindow>::iterator pit = desktop_window_list.begin();
	for (; pit != desktop_window_list.end(); pit++) {
		(*pit).reconfigure();
	}

  	frame_window.reconfigure();
}

void ToolWindow::raiseWindow(Window win) 
{
	PagerWindow pager_window = findPagerWindow(win); 	
	if (pager_window)
		XRaiseWindow(XDisplay(), pager_window.pagerWindow());
}

void ToolWindow::lowerWindow(Window win) 
{
	PagerWindow pager_window = findPagerWindow(win); 	
	if (pager_window)
    		XLowerWindow(XDisplay(), pager_window.pagerWindow());
}



void ToolWindow::focusWindow(Window win) 
{
	/* remove focus from previously focused window */
	if (focuswin) {
	  	focuswin.clearFocus();
	}

	PagerWindow pager_window = findPagerWindow(win);
	
     	if (pager_window) { 
	  	pager_window.setFocus();
		focuswin = pager_window;
	}
}

void ToolWindow::desktopChange(int desktop_nr) 
{
	Window win;

	if (resource->getDesktopFocusStyle() != none) {
		
		if (current_desktop_nr != -1) {
			DesktopWindow desktop_window = findDesktopWindow(current_desktop_nr);
			if (desktop_window)
				desktop_window.clearFocus();
		}
		
		DesktopWindow desktop_window = findDesktopWindow(desktop_nr);
		if (desktop_window)
			desktop_window.setFocus();	
	}
	current_desktop_nr = desktop_nr;
}

DesktopWindow &ToolWindow::findDesktopWindow(int desktop_nr)
{
	std::list<DesktopWindow>::iterator it = desktop_window_list.begin();

	for (; it != desktop_window_list.end(); it++) {
		if ((*it).desktopId() == desktop_nr) {
			return (*it);
		}
	}
	return NULL;
}


int ToolWindow::winOnDesktop(Window win) 
{
	PagerWindow pager_window = findPagerWindow(win);

	if (pager_window)
		return pager_window.desktopId();

	return(0);
}

void ToolWindow::removeWindow(Window win) {

  	PagerWindow pager_window = findPagerWindow(win);

	if (pager_window) {
	  	if (pager_window == focuswin)
			focuswin = NULL;
		pager_window_list.remove(pager_window);
		delete pager_window;
	}
	//if (tmp->sticky) wminterface->removeSticky(tmp->win,tmp->desktop_nr);
}

void ToolWindow::removeDesktopWindow()
{
	/* delete last */
  	DesktopWindow dekstop_window = desktop_window_list.back();
  	dekstop_window_list.pop_back();	
	delete desktop_window;
 
    	desktop_nr--;
	frame_window->resize();
}


//void ToolWindow::addFrameWindow(struct WindowList *window,Window desktopWin,bool reconfigure) 
//{
//}

FrameWindow::FrameWindow
{
}

FrameWindow::~FrameWindow()
{
	XUnmapWindow(XDisplay(), framewin);
	/* destroy pixmaps */
	if (pixmap) bt::PixmapCache::release(pixmap.frame);
	/* destroy windows */
	XDestroyWindow(XDisplay(),framewin);
}

FrameWindow::buildWindow(bool reconfigure)
{
	XSetWindowAttributes attrib;
	XWMHints wmhints;
	XClassHint classhints;
	XTextProperty windowname;

	unsigned long create_mask = CWBackPixmap | CWBorderPixel |
				    CWCursor | CWEventMask;

	if (withdrawn) {
		wmhints.initial_state = WithdrawnState;
	} else {
		wmhints.initial_state = NormalState;
	}

	attrib.background_pixmap = ParentRelative;
	attrib.border_pixel=resource->desktopwin.activeColor.getPixel();


	pixmap.frame = getImageControl()->renderImage(frame.width, frame.height,
                 &resource->frame.texture);

	attrib.cursor = getSessionCursor();
	attrib.event_mask = ButtonPressMask | ButtonReleaseMask | ExposureMask |
                      FocusChangeMask | StructureNotifyMask|
                      SubstructureRedirectMask;

	calcSize();
	
	if (!reconfigure) {
		win = XCreateWindow(getXDisplay(), 
                             getCurrentScreenInfo()->getRootWindow(), frame.x,
                             frame.y, frame.width,
                             frame.height, 0, 
                             getCurrentScreenInfo()->getDepth(),
                             InputOutput,
                             getCurrentScreenInfo()->getVisual(), 
                             create_mask, &attrib);
	} else if (!withdrawn) {
		XMoveResizeWindow(getXDisplay(), framewin, frame.x,frame.y,
                      frame.width,frame.height);

	} else {
		XResizeWindow(getXDisplay(),framewin,frame.width,frame.height);
	}

	char *name=BBTOOL;
	XSizeHints sizehints;

	wmhints.flags = StateHint | InputHint;
	wmhints.input = False;
 
	classhints.res_name = BBTOOL;
	classhints.res_class = "bbtools";
          
	sizehints.x = frame.x;//getResource()->position.x;
	sizehints.y = frame.y;//getResource()->position.y;

	sizehints.max_width = sizehints.min_width=frame.width;
	sizehints.max_height = sizehints.min_height=frame.height;
	sizehints.flags = USPosition | PMinSize | PMaxSize;

	XStringListToTextProperty(&name, 1, &windowname);
	XSetWMProperties(getXDisplay(),framewin,&windowname,NULL,getArgv(),getArgc(),
                  &sizehints,&wmhints,&classhints);
	Atom wmproto[1];
	wmproto[0] = wm_delete_window;
  	XSetWMProtocols(XDisplay(), framewin, wmproto, 1);

	if (!shape) {
		XSetWindowBackgroundPixmap(getXDisplay(), framewin, pixmap.frame);
	}

	if (!withdrawn && resource->report.auto_raise) {
		XRaiseWindow(getXDisplay(), framewin);
		lower = false;
	}
	else lower=True;

	XClearWindow(getXDisplay(), framewin);
	XMapWindow(getXDisplay(), framewin);
	XMapSubwindows(getXDisplay(), framewin);
}

FrameWindow::resize(void)
{
	size();
	if (!withdrawn)
		XMoveResizeWindow(bbtool->XDisplay(), bbtool->frameWindow.window(), bbtool->frameWindow.x(),
				  bbtool->frameWindow.y(), bbtool->frameWindow.width(), 
				  bbtool->frameWindow.height());
	else
		XResizeWindow(bbtool->XDisplay(), bbtool->frameWindow.window(), bbtool->frameWindow.width(),
			      bbtool->frameWindow.height());

}

FrameWindow::calcSize(bool reconfigure)
{

	if (resource->position.vertical) {

		if (number_of_desktops < resource->columns)
			_width = (unsigned int)(bbtool->getResource()->desktopSize.width + 
				bbtool->getResource->frame.bevelWidth) *
				(number_of_desktops % bbtool->getResource()->columns) + 
				bbtool->getResource()->frame.bevelWidth;
		else
			_width = (unsigned int)(bbtool->getResource()->desktopSize.width + 
				 bbtool->getResource()->frame.bevelWidth) *
				 bbtool->getResource()->columns + bbtool->getResource()->frame.bevelWidth;

			_height = (unsigned int)(((bbtool->numberOfDesktops() - 1) / bbtool->getResource()->columns + 1)*
				  (bbtool->getResource()->desktopSize.height + 
				   bbtool->getResource()->frame.bevelWidth) + bbtool->getResource()->frame.bevelWidth);

	} else {
		_width = (unsigned int)((bbtool->numberOfDesktops - 1) / bbtool->getResource()->rows + 1) *
		         (bbtool->getResource()->desktopSize.width + 
		         bbtool->getResource()->frame.bevelWidth) + bbtool->getResource()->frame.bevelWidth;
   
		if (bbtool->numberOfDesktops < bbtool->getResource()->rows)
			_height = (unsigned int)(bbtool->getResource()->desktopSize.height +
				  bbtool->getResource()->frame.bevelWidth)*
				  (bbtool->numberOfDesktops % bbtool->getResource()->rows) + 
				  bbtool->getResource()->frame.bevelWidth;
		else
			_height = (unsigned int)(bbtool->getResource()->desktopSize.height +
				  bbtool->getResource()->frame.bevelWidth) *
				  bbtool->getResource()->rows + resource->frame.bevelWidth;

	}

	_x = bbtool->getResource()->position.x;
	_y = bbtool->getResource()->position.y;

	if (bbtool->getResource()->position.mask & XNegative) {
		_x = bbtool->getCurrentScreenInfo()->width() + 
		     bbtool->getResource()->position.x - _width;
	}

	if (bbtool->getResource()->position.mask & YNegative) {
		_y = bbtool->getCurrentScreenInfo()->height() +
		     bbtool->getResource()->position.y - _height;

	}


}

void ToolWindow::MakeWindow(bool reconfigure) 
{
}

/*void ToolWindow::CheckConfig()
{
	struct stat file_status;

	if (stat(config_filename,&file_status)!=0)
	{
		fprintf(stderr,"Could not open config file %s\n",
        		resource->style.conf_filename);
	}
   	else if (file_status.st_mtime != resource->style.mtime)
	{
		resource->style.mtime=file_status.st_mtime;
		reconfigure();
	}
}*/

void ToolWindow::process_event(XEvent *Event) {
  static bool moved;
  static Window grabbedWindow;
  static Window realWindow;
  static Window pagerWindow;
  static WindowList *moveWindow;
  static int grabbed_x,grabbed_y,move_x,move_y;

  switch (Event->type) {
    case  PropertyNotify: {
      wminterface->windowAttributeChange(Event->xproperty.window);
    }
    break;
    case ClientMessage: {
      if ((unsigned)Event->xclient.data.l[0]==wm_delete_window) shutdown();
        wminterface->handleBlackboxEvents(*Event);
    }
    break;
    case ButtonPress: {
      if (Event->xbutton.button == LEFT_BUTTON) {
        if (Event->xbutton.window==framewin) {
          XRaiseWindow(getXDisplay(),framewin);
          lower=False;
        }
      }
      if (Event->xbutton.button == MIDDLE_BUTTON) {
        if (Event->xbutton.window==framewin) {
          XLowerWindow(getXDisplay(),framewin);
          lower=True;
        }
      }
      if (Event->xbutton.button == (unsigned)
                                resource->getDesktopChangeButton()) {
          LinkedListIterator<DesktopList> desktop_it(desktopList);
          for (; desktop_it.current(); desktop_it++) {
            if ((desktop_it.current()->win) == Event->xbutton.window) {
              wminterface->changeDesktop(desktop_it.current()->desktop_nr);
              break;
            }
          }
        }
      if (Event->xbutton.button == (unsigned)
                                 resource->getWindowRaiseButton()) {
        LinkedListIterator<WindowList> win_it(windowList);
        for (; win_it.current(); win_it++)
          if ((win_it.current()->pager_win) == Event->xbutton.subwindow) {
            XRaiseWindow(getXDisplay(),win_it.current()->win);
          } 
      }
      if (Event->xbutton.button == (unsigned)
                                 resource->getWindowFocusButton()) {
        LinkedListIterator<WindowList> win_it(windowList);
        for (; win_it.current(); win_it++) 
          if ((win_it.current()->pager_win) == Event->xbutton.subwindow) {
            wminterface->setWindowFocus(win_it.current()->win);
        }
      }
      if (Event->xbutton.button == (unsigned)
                                 resource->getWindowMoveButton()) {
          LinkedListIterator<WindowList> win_it(windowList);
          LinkedListIterator<DesktopList> desktop_it(desktopList);

          DesktopList *desktop = new DesktopList;
  
          for (; desktop_it.current(); desktop_it++) {
            if ((desktop_it.current()->win) == Event->xbutton.window)
              break;
          }

          if (desktop_it.current()) {
            desktop=desktop_it.current();
          } 

          for (; win_it.current(); win_it++)
            if ((win_it.current()->pager_win) == Event->xbutton.subwindow)
              break;

          if (win_it.current()) {
            XSetWindowAttributes attrib;
            unsigned long create_mask = CWBackPixmap|CWCursor|
                                        CWBorderPixel;
  
            attrib.background_pixmap = ParentRelative;
            attrib.border_pixel=
            resource->pagerwin.inactiveColor.getPixel();
            attrib.cursor = getSessionCursor();

            grabbedWindow = XCreateWindow(getXDisplay(), framewin,
                                        win_it.current()->pager_x+desktop->x,
                                        win_it.current()->pager_y+desktop->y,
                                        win_it.current()->pager_width,
                                        win_it.current()->pager_height,
                                        1, getCurrentScreenInfo()->getDepth(),
                                        InputOutput,
                                        getCurrentScreenInfo()->getVisual(),
                                        create_mask,&attrib);

            grabbed_x=desktop->x+win_it.current()->pager_x-Event->xbutton.x;
            grabbed_y=desktop->y+win_it.current()->pager_y-Event->xbutton.y;
            if (!win_it.current()->focused ||
                               resource->getFocusStyle()!=texture)
              XSetWindowBackgroundPixmap(getXDisplay(), grabbedWindow,
                                         pixmap.window);
            else
               XSetWindowBackgroundPixmap(getXDisplay(), grabbedWindow,
                                         pixmap.focusedWindow);
 
            moveWindow=win_it.current();
            realWindow=win_it.current()->win;
            pagerWindow=win_it.current()->pager_win;
            moved=False;
            XMapWindow(getXDisplay(),grabbedWindow);
          } else {
            grabbed_x=0;
            grabbed_y=0;
            grabbedWindow=0;
          }
        }
      }
    break;
    case ButtonRelease: {
      if (Event->xbutton.button == (unsigned) 
                                   resource->getWindowMoveButton())
        if (grabbedWindow!=0) {
          if (!moved) {
            XDestroyWindow(getXDisplay(),grabbedWindow);
            grabbedWindow=0; 
            break;
          }
          LinkedListIterator<DesktopList> desktop_it(desktopList);
          for (; desktop_it.current(); desktop_it++) {
            if (move_x>desktop_it.current()->x -
                      desktop_it.current()->width &&
                      move_x<=desktop_it.current()->x +
                      desktop_it.current()->width &&
                      move_y>desktop_it.current()->y -
                      desktop_it.current()->height&&
                      move_y<desktop_it.current()->y +
                      desktop_it.current()->height)
            break;
          } 
          if (desktop_it.current()) {
            if (!moveWindow->sticky) 
              wminterface->sendWindowToDesktop(realWindow,
                            desktop_it.current()->desktop_nr);
            double xdiv=(double)resource->desktopSize.width/
                         getCurrentScreenInfo()->getWidth();
            double ydiv=(double)resource->desktopSize.height/
                         getCurrentScreenInfo()->getHeight();
 
            int x=(int)((move_x-desktop_it.current()->x)/xdiv);
            int y=(int)((move_y-desktop_it.current()->y)/ydiv);
            XMoveWindow(getXDisplay(),realWindow,x,y);
            XUnmapWindow(getXDisplay(),grabbedWindow);
            XDestroyWindow(getXDisplay(),grabbedWindow);
            if (!moveWindow->sticky) 
              moveWinToDesktop(realWindow,desktop_it.current());
            grabbedWindow=0;
          }
          else {
            XDestroyWindow(getXDisplay(),grabbedWindow);
            grabbedWindow=0;
          }
        }
      }
    break;
    case MotionNotify: {
      if (grabbedWindow) {
        moved=True;
        move_x=Event->xmotion.x+grabbed_x;
        move_y=Event->xmotion.y+grabbed_y;
        XMoveWindow(getXDisplay(),grabbedWindow,Event->xmotion.x+grabbed_x,
                    Event->xmotion.y+grabbed_y);
      }
    }
    break;
    case ConfigureNotify: {
      if (Event->xconfigure.window==framewin && Event->xconfigure.send_event) {
      if (withdrawn)
        reconfigure();
      int parent_x,parent_y;
      Window parent_root;
      unsigned int parent_width;
      unsigned int parent_height;
      unsigned int parent_border_width;
      unsigned int parent_depth;
      frame.x=Event->xconfigure.x;
      frame.y=Event->xconfigure.y;
      if (withdrawn) {
        XGetGeometry(getXDisplay(),Event->xconfigure.above,&parent_root,
                     &parent_x,&parent_y,&parent_width,&parent_height,
                     &parent_border_width,&parent_depth);
                     frame.x=Event->xconfigure.x+parent_x;
        frame.x+=parent_x;
        frame.y+=parent_y;
      } else {
        if (position==NULL) position= new char [13];
          sprintf(position,"+%i+%i",frame.x,frame.y);
      }
    }
    else {
      LinkedListIterator<WindowList> win_it(windowList);
      WindowList *tmp;
      for (; win_it.current(); win_it++)
        if ((win_it.current()->win) == Event->xconfigure.window) {
          tmp=win_it.current();

          int status=getWindowGeometry(tmp);
          if (status==-1) break;
          double xdiv=(double)resource->desktopSize.width/
          getCurrentScreenInfo()->getWidth();
          double ydiv=(double)resource->desktopSize.height/
          getCurrentScreenInfo()->getHeight();
          tmp->pager_x=(int)(tmp->x_position*xdiv);
          tmp->pager_y=(int)(tmp->y_position*ydiv);
          tmp->pager_width=(unsigned int)(tmp->width*xdiv);
          if (!tmp->shaded)
            tmp->pager_height=(unsigned int)(tmp->height*ydiv);
          if (tmp->pager_width==0) tmp->pager_width=1;
          if (tmp->pager_height==0) tmp->pager_height=1;

          XMoveResizeWindow(getXDisplay(),win_it.current()->pager_win,
                            tmp->pager_x,tmp->pager_y,
                            tmp->pager_width,tmp->pager_height);

          if (!win_it.current()->focused || 
                                        resource->getFocusStyle()!=texture)
            XSetWindowBackgroundPixmap(getXDisplay(),
                                       win_it.current()->pager_win,
                                       getPixmap().window);
          else
            XSetWindowBackgroundPixmap(getXDisplay(),
                                       win_it.current()->pager_win,
                                       getPixmap().focusedWindow);
        }

      }
          
//    else if (Event.xconfigure.window==framewin) 
//      XResizeWindow(getXDisplay(),framewin,frame.width,frame.height);

    }
    break;
    case Expose: {
      if (!lower)
        XRaiseWindow(getXDisplay(),framewin);
    }
    break;
  }
}
