// bbpager.cc for bbpager - a pager for Blackbox.
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

#include "bbpager.hh"
#include "version.h"
#include <stdio.h>

ToolWindow::ToolWindow(int argc,char **argv, struct CMDOPTIONS *options) :
  Basewindow(argc,argv,options) {
  XrmInitialize();

  iargc=argc;
  iargv=argv;

  resource = new Resource(this);
  wminterface = new WMInterface(this);
  windowList = new LinkedList<WindowList>;
  desktopList = new LinkedList<DesktopList>;

  desktop_nr=0;
  current_desktop_nr=-1;
  wm_init=False;
  number_of_desktops=0;
  row_last=column_last=0;
  MakeWindow(False);
  wminterface->moduleInit();
  eventLoop();
}

ToolWindow::~ToolWindow() {
  XUnmapWindow(getXDisplay(),framewin);

  /* destroy pixmaps */
  if (pixmap.frame) getImageControl()->removeImage(pixmap.frame);
  if (pixmap.desktop) getImageControl()->removeImage(pixmap.desktop);
  if (pixmap.window) getImageControl()->removeImage(pixmap.window);
  if (pixmap.focusedWindow) 
    getImageControl()->removeImage(pixmap.focusedWindow);
  if (pixmap.focusedDesktop) 
    getImageControl()->removeImage(pixmap.focusedDesktop);

  /* destroy windows */
  XDestroyWindow(getXDisplay(),framewin);
  /* destroy windows */
  delete windowList;
  delete desktopList;
}

void ToolWindow::moveWinToDesktop(Window win,DesktopList *desktop) {

  LinkedListIterator<WindowList> win_it(windowList);
  for (; win_it.current(); win_it++)
    if ((win_it.current()->win) == win)
      break;
  if (win_it.current()) {
    if ((desktop->desktop_nr!=win_it.current()->desktop_nr) 
                    & (!win_it.current()->sticky)) {

        XUnmapWindow(getXDisplay(),win_it.current()->pager_win);
        XReparentWindow(getXDisplay(),win_it.current()->pager_win,
                        desktop->win,win_it.current()->pager_x,
                        win_it.current()->pager_y);
        XMapWindow(getXDisplay(),win_it.current()->pager_win);
        win_it.current()->desktop_nr=desktop_nr;
    }
  }
}

int ToolWindow::getWindowGeometry(struct WindowList  *winlist) {
  unsigned int border_width,depth;
  Window root_return,child_return;
  int x_return,y_return;
  int status;

  status = XGetGeometry(getXDisplay(),winlist->win,&root_return,&x_return,
                        &y_return,&winlist->width,&winlist->height,
                        &border_width,&depth);
  if (status) {
    XTranslateCoordinates(getXDisplay(),winlist->win,root_return,x_return,
                          y_return,&winlist->x_position,&winlist->y_position,
                          &child_return);
    return 1;
  }
  return 0;
}


void ToolWindow::reconfigure(void) {
  /* destroy pixmaps */
  getImageControl()->removeImage(pixmap.frame);
  getImageControl()->removeImage(pixmap.desktop);
  getImageControl()->removeImage(pixmap.window);
  if (pixmap.focusedWindow) 
    getImageControl()->removeImage(pixmap.focusedWindow);
  if (pixmap.focusedDesktop) 
    getImageControl()->removeImage(pixmap.focusedDesktop);

  resource->Reload();

  MakeWindow(True);
  

  desktop_nr=0;
  {  
  LinkedListIterator<DesktopList> desktop_it(desktopList);
  for (; desktop_it.current(); desktop_it++)
    addDesktopWindow(desktop_it.current(),True);
  }
  LinkedListIterator<WindowList> win_it(windowList);
  for (; win_it.current(); win_it++)
    addFrameWindow(win_it.current(),0,True);

  XClearWindow(getXDisplay(), framewin);
}

void ToolWindow::raiseWindow(Window win) {
  LinkedListIterator<WindowList> win_it(windowList);
  for (; win_it.current(); win_it++)
    if ((win_it.current()->win) == win)
      break;

  if (win_it.current()) {
    WindowList *tmp = win_it.current();
    XRaiseWindow(getXDisplay(),tmp->pager_win);
  }
}

void ToolWindow::lowerWindow(Window win) {
  LinkedListIterator<WindowList> win_it(windowList);
  for (; win_it.current(); win_it++)
    if ((win_it.current()->win) == win)
      break;

  if (win_it.current()) {
    WindowList *tmp = win_it.current();
    XLowerWindow(getXDisplay(),tmp->pager_win);
  }
}

void ToolWindow::focusWindow(Window win) {
  WindowList *tmp;


  if (focuswin) {
    LinkedListIterator<WindowList> old_it(windowList);
    for (; old_it.current(); old_it++) {
      if (old_it.current()->win == focuswin) {
        if (resource->getFocusStyle()==border)
          XSetWindowBorder(getXDisplay(),old_it.current()->pager_win,
                           resource->pagerwin.inactiveColor.getPixel());
         else
          XSetWindowBackgroundPixmap(getXDisplay(), 
                                     old_it.current()->pager_win,
                                     pixmap.window);

         old_it.current()->focused=False;
      
         XClearWindow(getXDisplay(),old_it.current()->pager_win);
      }
    }
  }
 
  LinkedListIterator<WindowList> win_it(windowList);
  for (; win_it.current(); win_it++) {
    if ((win_it.current()->win) == win) {
      tmp = win_it.current();
  
      
      if (resource->getFocusStyle()==border)
        XSetWindowBorder(getXDisplay(),tmp->pager_win,resource->pagerwin.
                         activeColor.getPixel());
      else 
        XSetWindowBackgroundPixmap(getXDisplay(), tmp->pager_win,
                                   pixmap.focusedWindow);
    
      XClearWindow(getXDisplay(),tmp->pager_win);
      tmp->focused=True;
      focuswin=tmp->win;
    }
  }
}

void ToolWindow::desktopChange(int desktop_nr) {
  Window win;

  if (resource->getDesktopFocusStyle()!=none) {
    LinkedListIterator<DesktopList> win_it(desktopList);
    if (current_desktop_nr!=-1) {
      win_it.set(current_desktop_nr);
      win = win_it.current()->win;
      if (resource->getDesktopFocusStyle()==border)
        XSetWindowBorderWidth(getXDisplay(),win,0);
      else
        XSetWindowBackgroundPixmap(getXDisplay(), win, pixmap.desktop);
      XClearWindow(getXDisplay(),win);
    }
    win_it.set(desktop_nr);
    win = win_it.current()->win;
    if (resource->getDesktopFocusStyle()==border)
      XSetWindowBorderWidth(getXDisplay(),win,1);
    else
      XSetWindowBackgroundPixmap(getXDisplay(), win, pixmap.focusedDesktop);
     XClearWindow(getXDisplay(),win);
  }
    current_desktop_nr=desktop_nr;
}

int ToolWindow::winOnDesktop(Window win) {
  LinkedListIterator<WindowList> win_it(windowList);
  for (; win_it.current(); win_it++)
    if ((win_it.current()->pager_win) == win)
      break;

  if (win_it.current())
    return(win_it.current()->desktop_nr);

  return(0);
}

void ToolWindow::removeWindow(Window win) {
  WindowList *tmp=0;
  {
  LinkedListIterator<WindowList> win_it(windowList);
  for (; win_it.current(); win_it++)
    if ((win_it.current()->win) == win)
      break;

  if (win_it.current()) {
    tmp = win_it.current();
  
  
    if (tmp->sticky) wminterface->removeSticky(tmp->win,tmp->desktop_nr);
    windowList->remove(tmp);
    if (tmp->focused) focuswin=0;
    XDestroyWindow(getXDisplay(),tmp->pager_win);
    delete tmp;
  }
  }
}

void ToolWindow::removeDesktopWindow()
{
   /* delete last */
  DesktopList *tmp = new DesktopList;
  tmp= desktopList->last();
  LinkedListIterator<WindowList> win_it(windowList);
  for (; win_it.current(); win_it++)
    if ((win_it.current()->sticky && 
      win_it.current()->desktop_nr==tmp->desktop_nr))
      break;
              
  if (win_it.current()) {
    WindowList *sticky=win_it.current();
    windowList->remove(sticky);
    XDestroyWindow(getXDisplay(),sticky->pager_win);
    delete sticky;
  }
  
  desktopList->remove(tmp);
  XUnmapWindow(getXDisplay(),tmp->win);
  XDestroyWindow(getXDisplay(),tmp->win);
  delete tmp;
  if (resource->position.vertical) {
   if (number_of_desktops<resource->columns)
    frame.width=(unsigned int)(resource->desktopSize.width + 
                resource->frame.bevelWidth)*
                (number_of_desktops%resource->columns) + 
                resource->frame.bevelWidth;
   else
    frame.width=(unsigned int)(resource->desktopSize.width + 
                 resource->frame.bevelWidth)*
                 resource->columns + resource->frame.bevelWidth;

    frame.height=(unsigned int)(((number_of_desktops-1)/
                  resource->columns)+1)*
                  (resource->desktopSize.height + 
                   resource->frame.bevelWidth) + 
                   resource->frame.bevelWidth;
  } else {
    frame.width=(unsigned int)((number_of_desktops-1)/resource->rows+1) *
                (resource->desktopSize.width + 
                 resource->frame.bevelWidth) + 
                 resource->frame.bevelWidth;
   
    if (number_of_desktops<resource->rows)
      frame.height=(unsigned int)(resource->desktopSize.height+
                  resource->frame.bevelWidth)*
                  (number_of_desktops%resource->rows) + 
                  resource->frame.bevelWidth;
    else
      frame.height=(unsigned int)(resource->desktopSize.height+
                  resource->frame.bevelWidth)*
                  resource->rows + resource->frame.bevelWidth;
  }

  if (resource->position.mask & XNegative) {
    frame.x = getCurrentScreenInfo()->getWidth() +
              resource->position.x - frame.width;
  }
  if (resource->position.mask & YNegative) {
    frame.y = getCurrentScreenInfo()->getHeight() +
              resource->position.y - frame.height;
  }
              
  if (!withdrawn)
    XMoveResizeWindow(getXDisplay(), framewin,frame.x,frame.y,
                      frame.width,frame.height);
  else
    XResizeWindow(getXDisplay(),framewin,frame.width,frame.height);

  if (!shape)
    XSetWindowBackgroundPixmap(getXDisplay(), framewin, pixmap.frame);

  XClearWindow(getXDisplay(),framewin);
  desktop_nr--;
}

void ToolWindow::addDesktopWindow(struct DesktopList *tmp,bool reconfigure) {
  XSetWindowAttributes attrib;
  int row,column;
  unsigned long create_mask = CWBackPixmap|CWCursor|CWEventMask|CWBorderPixel;

  attrib.background_pixmap = ParentRelative;

  attrib.border_pixel=resource->desktopwin.activeColor.getPixel();
  attrib.cursor = getSessionCursor();
  attrib.event_mask = ButtonPressMask | ButtonReleaseMask | ExposureMask |
                      FocusChangeMask | StructureNotifyMask|
                      SubstructureRedirectMask | ButtonMotionMask;

  tmp->width=resource->desktopSize.width;
  tmp->height=resource->desktopSize.height;
  
  if (resource->position.vertical) {
    row=desktop_nr/resource->columns;
    column=desktop_nr%resource->columns +1;

    tmp->x=(resource->frame.bevelWidth + resource->desktopSize.width)* 
            (column-1) + resource->frame.bevelWidth;
    tmp->y = resource->frame.bevelWidth +
             ((row) * (resource->desktopSize.height +
              resource->frame.bevelWidth));

  if (number_of_desktops<resource->columns)
    frame.width=(unsigned int)(resource->desktopSize.width + 
                resource->frame.bevelWidth)*
                (number_of_desktops%resource->columns) + 
                resource->frame.bevelWidth;
   else
    frame.width=(unsigned int)(resource->desktopSize.width + 
                resource->frame.bevelWidth)*
                resource->columns + resource->frame.bevelWidth;

   frame.height=(unsigned int)(((number_of_desktops-1)/resource->columns+1)*
                   (resource->desktopSize.height + 
                   resource->frame.bevelWidth) + resource->frame.bevelWidth);
  }
  else {
    column=desktop_nr/resource->rows;
    row=desktop_nr%resource->rows +1;

    tmp->x=(resource->frame.bevelWidth + resource->desktopSize.width)* 
            column + resource->frame.bevelWidth;

    tmp->y = resource->frame.bevelWidth +
             ((row-1) * (resource->desktopSize.height +
              resource->frame.bevelWidth));

    frame.width=(unsigned int)((number_of_desktops-1)/resource->rows+1) *
                 (resource->desktopSize.width + 
                  resource->frame.bevelWidth) + resource->frame.bevelWidth;
   
    if (number_of_desktops<resource->rows)
       frame.height=(unsigned int)(resource->desktopSize.height+
                  resource->frame.bevelWidth)*
                  (number_of_desktops%resource->rows) + 
                  resource->frame.bevelWidth;
    else
      frame.height=(unsigned int)(resource->desktopSize.height+
                  resource->frame.bevelWidth)*
                  resource->rows + resource->frame.bevelWidth;
  }
  frame.x=resource->position.x;
  frame.y=resource->position.y;

  if (resource->position.mask & XNegative) {
    frame.x = getCurrentScreenInfo()->getWidth() + 
              resource->position.x - frame.width;
  }
  if (resource->position.mask & YNegative) {
    frame.y = getCurrentScreenInfo()->getHeight() + 
              resource->position.y - frame.height;
  }


  if (!withdrawn)
    XMoveResizeWindow(getXDisplay(), framewin,frame.x,
                    frame.y,frame.width,frame.height);
  else
    XResizeWindow(getXDisplay(),framewin,frame.width,frame.height);

  if (!reconfigure)
    tmp->win = XCreateWindow(getXDisplay(), framewin,tmp->x,tmp->y,tmp->width,
                           tmp->height,0, getCurrentScreenInfo()->getDepth(),
                           InputOutput, getCurrentScreenInfo()->getVisual(),
                           create_mask,&attrib);
  else
      XMoveResizeWindow(getXDisplay(),tmp->win,tmp->x,tmp->y,tmp->width,tmp->height);

  getImageControl()->removeImage(pixmap.frame);
  pixmap.frame = getImageControl()->renderImage(frame.width, frame.height,
                 &resource->frame.texture);

  if (!shape)
    XSetWindowBackgroundPixmap(getXDisplay(), framewin, pixmap.frame);

  XSetWindowBackgroundPixmap(getXDisplay(), tmp->win, pixmap.desktop);
  XMapSubwindows(getXDisplay(), framewin);
  XClearWindow(getXDisplay(),framewin);
  XClearWindow(getXDisplay(),tmp->win );
  tmp->desktop_nr=desktop_nr++;
}

void ToolWindow::addFrameWindow(struct WindowList *window,Window desktopWin,bool reconfigure) {
  XSetWindowAttributes attrib;
  unsigned long create_mask = CWBackPixmap|CWCursor|CWBorderPixel;
  double xdiv,ydiv;

  xdiv=10;
  ydiv=10;

  if (!desktopWin) {
    LinkedListIterator<DesktopList> desktop_it(desktopList);
    desktop_it.set(window->desktop_nr);
    desktopWin = desktop_it.current()->win;
  }

  attrib.background_pixmap = ParentRelative;
  attrib.border_pixel=resource->pagerwin.inactiveColor.getPixel();
  attrib.cursor = getSessionCursor();

  xdiv=(double)resource->desktopSize.width/
          getCurrentScreenInfo()->getWidth();
  ydiv=(double)resource->desktopSize.height/
          getCurrentScreenInfo()->getHeight();
  window->pager_x=(int)(window->x_position*xdiv);
  window->pager_y=(int)(window->y_position*ydiv);
  window->pager_width=(unsigned int)(window->width*xdiv);
  window->pager_height=(unsigned int)(window->height*ydiv);
  if (window->pager_width==0)   window->pager_width=1;
  if (window->pager_height==0) window->pager_height=1;

  
  if (!reconfigure) {
    window->pager_win = XCreateWindow(getXDisplay(), desktopWin,
                                    window->pager_x,window->pager_y,
                                    window->pager_width,window->pager_height,
                                    1, getCurrentScreenInfo()->getDepth(), 
                                    InputOutput,
                                    getCurrentScreenInfo()->getVisual(), 
                                    create_mask,&attrib);
    XSelectInput(getXDisplay(),window->win,
                 PropertyChangeMask|StructureNotifyMask);
  }
  else
    XMoveResizeWindow(getXDisplay(),window->pager_win,window->pager_x,
                      window->pager_y,window->pager_width,window->pager_height);
  if (!window->focused)
     XSetWindowBackgroundPixmap(getXDisplay(), window->pager_win,
                                pixmap.window);
  else
     XSetWindowBackgroundPixmap(getXDisplay(), window->pager_win,
                                pixmap.focusedWindow);
  if (!window->icon)
    XMapWindow(getXDisplay(),window->pager_win);
  XClearWindow(getXDisplay(),window->pager_win );
}

void ToolWindow::MakeWindow(bool reconfigure) {
  XSetWindowAttributes attrib;
  XWMHints wmhints;
  XClassHint classhints;
  XTextProperty windowname;

  unsigned long create_mask = CWBackPixmap|CWBorderPixel|
                              CWCursor|CWEventMask;

  if (resource->position.vertical) {

    if (number_of_desktops<resource->columns)
      frame.width=(unsigned int)(resource->desktopSize.width + 
                  resource->frame.bevelWidth)*
                  (number_of_desktops%resource->columns) + 
                  resource->frame.bevelWidth;
     else
      frame.width=(unsigned int)(resource->desktopSize.width + 
                  resource->frame.bevelWidth)*
                  resource->columns + resource->frame.bevelWidth;

     frame.height=(unsigned int)(((number_of_desktops-1)/resource->columns+1)*
                     (resource->desktopSize.height + 
                     resource->frame.bevelWidth) + resource->frame.bevelWidth);

  }
  else {
    frame.width=(unsigned int)((number_of_desktops-1)/resource->rows+1) *
                (resource->desktopSize.width + 
                 resource->frame.bevelWidth) + resource->frame.bevelWidth;
   
    if (number_of_desktops<resource->rows)
       frame.height=(unsigned int)(resource->desktopSize.height+
                  resource->frame.bevelWidth)*
                  (number_of_desktops%resource->rows) + 
                  resource->frame.bevelWidth;
    else
      frame.height=(unsigned int)(resource->desktopSize.height+
                  resource->frame.bevelWidth)*
                  resource->rows + resource->frame.bevelWidth;

  }
  frame.x=resource->position.x;
  frame.y=resource->position.y;
  if (resource->position.mask & XNegative) {
    frame.x = getCurrentScreenInfo()->getWidth() + 
              resource->position.x - frame.width;
  }

  if (resource->position.mask & YNegative) {
    frame.y = getCurrentScreenInfo()->getHeight() +
              resource->position.y - frame.height;

  }

  if (withdrawn) {
    wmhints.initial_state = WithdrawnState;
  } else {
	 	wmhints.initial_state = NormalState;
  }

  attrib.background_pixmap = ParentRelative;
  attrib.border_pixel=resource->desktopwin.activeColor.getPixel();


  pixmap.frame = getImageControl()->renderImage(frame.width, frame.height,
                 &resource->frame.texture);

  pixmap.window =	getImageControl()->renderImage(resource->desktopSize.width,
                  resource->desktopSize.height,
                  &resource->pagerwin.texture);

  if (resource->getFocusStyle()==texture)
    pixmap.focusedWindow =	
          getImageControl()->renderImage(resource->desktopSize.width,
                  resource->desktopSize.height,
                  &resource->pagerwin.focusedTexture);

  
  pixmap.desktop =	getImageControl()->renderImage(resource->desktopSize.width,
                   resource->desktopSize.height,
                   &resource->desktopwin.texture);

  if (resource->getDesktopFocusStyle()==texture)
    pixmap.focusedDesktop =	
          getImageControl()->renderImage(resource->desktopSize.width,
                  resource->desktopSize.height,
                  &resource->desktopwin.focusedTexture);

  
  attrib.cursor = getSessionCursor();
  attrib.event_mask = ButtonPressMask | ButtonReleaseMask | ExposureMask |
                      FocusChangeMask | StructureNotifyMask|
                      SubstructureRedirectMask;

  if (!reconfigure) {
    framewin = XCreateWindow(getXDisplay(), 
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
 
  classhints.res_name=BBTOOL;
  classhints.res_class="bbtools";
          
  sizehints.x=frame.x;//getResource()->position.x;
  sizehints.y=frame.y;//getResource()->position.y;

  sizehints.max_width=sizehints.min_width=frame.width;
  sizehints.max_height=sizehints.min_height=frame.height;
  sizehints.flags=USPosition|PMinSize|PMaxSize;

  XStringListToTextProperty(&name,1,&windowname);
  XSetWMProperties(getXDisplay(),framewin,&windowname,NULL,getArgv(),getArgc(),
                  &sizehints,&wmhints,&classhints);
  Atom wmproto[2];
  wmproto[0]=wm_delete_window;
  wmproto[1]=getBlackboxStructureMessagesAtom();
  XSetWMProtocols(getXDisplay(), framewin,wmproto, 2);

  if (!decorated&&!withdrawn) {
    BlackboxHints net_hints;
    net_hints.decoration=DecorNone;
    net_hints.attrib= AttribOmnipresent;
    net_hints.flags=AttribDecoration|AttribOmnipresent;
    XChangeProperty(getXDisplay(), framewin, getBlackboxHintsAtom(),
                getBlackboxHintsAtom(), 32, PropModeReplace,
                (unsigned char *) &net_hints,  PropBlackboxHintsElements);
  }
 

  
  if (!shape) {
    XSetWindowBackgroundPixmap(getXDisplay(), framewin, pixmap.frame);
  }

  if (!withdrawn && resource->report.auto_raise) {
    XRaiseWindow(getXDisplay(),framewin);
    lower=False;
  }
  else lower=True;

  XClearWindow(getXDisplay(), framewin);
  XMapWindow(getXDisplay(), framewin);
  XMapSubwindows(getXDisplay(), framewin);

}

void ToolWindow::CheckConfig()
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
}

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
