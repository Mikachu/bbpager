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

#include "wminterface.hh"
#include "resource.hh"
#include "BaseDisplay.hh"

WMInterface::WMInterface(ToolWindow *toolwindow) : 
	BlackboxInterface(toolwindow), bbtool(toolwindow);
{
}

WMInterface::~WMInterface() 
{
}

void WMInterface::moduleInit() 
{
}

void WMInterface::updateWindowList(void)
{
	WindowList window_vect;
	PWindow *pwindow;
  
	if (readClientList(bbtool->getCurrentScreenInfo()->rootWindow(), window_vect)) {
		/* add any new window windows */
		bt::Netwm::WindowList::iterator it = window_vect.begin();
		bt::Netwm::WindowList::iterator it_end = window_vect.end();

		for (; it != it_end; it++) {
			pwindow = bbtool->findWindow((*it))
			if ( pwindow == NULL) {
				bbtool->windowList().insert(new PWindow((bbtool, *it)));
			} else {
				pwindow.setMarked();
			}
		}

	  	/* delete any windows not in list */
		PWindowList::iterator pit = bbtool->windowList().begin();
		PWindowList::iterator pit_end = bbtool->windowList().end();
	
		for (; pit != pit_end; pit++) {
			if (!(*pit)->isMarked()) {
				delete *pit;
				bbtool->windowList().erase(pit);
			}
		}
	}
}

void WMInterface::sendClientMessage(Atom atom, XID data) {
  XEvent e;
  unsigned long mask;

  e.xclient.type = ClientMessage;
  e.xclient.window =  bbtool->getCurrentScreenInfo()->getRootWindow(),
//bbtool->framewin;

  e.xclient.message_type = atom;
  e.xclient.format = 32;
  e.xclient.data.l[0] = (unsigned long) data;
  e.xclient.data.l[1] = 0;

  mask =  SubstructureRedirectMask;
  XSendEvent(bbtool->getXDisplay(), 
             bbtool->getCurrentScreenInfo()->getRootWindow(),
             False, mask, &e);
}

void WMInterface::changeDesktop(int desk_number) {
  unsigned long int data=desk_number;

  sendClientMessage(bbtool->getBlackboxChangeWorkspaceAtom(),data);
}


void WMInterface::sendWindowToDesktop(Window win,int desk_number) {
  XEvent e;
  unsigned long mask;

  e.xclient.type = ClientMessage;
  e.xclient.window = win;
  e.xclient.message_type = bbtool->getBlackboxChangeAttributesAtom();
  e.xclient.format = 32;
  e.xclient.data.l[0] = AttribWorkspace;
  e.xclient.data.l[1] = 0;
  e.xclient.data.l[2] = desk_number;
  e.xclient.data.l[3] = 0; 
  e.xclient.data.l[4] = 0;
  e.xclient.data.l[5] = 0;

  mask =  SubstructureRedirectMask;
  XSendEvent(bbtool->getXDisplay(), 
             bbtool->getCurrentScreenInfo()->getRootWindow(),
             False, mask, &e);

}

void WMInterface::setWindowFocus(Window win) {
  XEvent e;
  unsigned long mask;

  e.xclient.type = ClientMessage;
  e.xclient.window =  win;

  e.xclient.message_type = bbtool->getBlackboxChangeWindowFocusAtom();
  e.xclient.format = 32;
  e.xclient.data.l[0] = 0;
  e.xclient.data.l[1] = 0;

  mask =  SubstructureRedirectMask;
  XSendEvent(bbtool->getXDisplay(), 
             bbtool->getCurrentScreenInfo()->getRootWindow(),
             False, mask, &e);
}

int WMInterface::isIconicState(Window win) {
  Atom real_type;
  int format;
  unsigned long n, extra;
  int status;
  long *p=0;

  status = XGetWindowProperty(bbtool->getXDisplay(), win, 
                              bbtool->getWMStateAtom(), 0L, 1L,
                              False, bbtool->getWMStateAtom(), &real_type,
                              &format, &n, &extra,  (unsigned char**)&p);
  if (!status) {
    if (p) return (p[0]==IconicState) ? 1 : 0;
    else return 0;
  }

  return -1;
}

int WMInterface::getAttributes(Window win) {
  Atom real_type;
  int format;
  unsigned long n, extra;
  int status;
  long *p=0;

  
  status = XGetWindowProperty(bbtool->getXDisplay(), win, 
                              bbtool->getBlackboxAttributesAtom(), 0L, 1L,
                              False, bbtool->getBlackboxAttributesAtom(), 
                              &real_type, &format,&n, &extra,  
                              (unsigned char**)&p);
  if (!status) {
    if (p) return p[0];
    else return 0;
  }

  return -1;
}


void WMInterface::addSticky(WindowList *tmp)
{
  int i=0;
        
  tmp->sticky=True;
        
  LinkedListIterator<DesktopList> desktop_it(bbtool->desktopList);

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
  }
}

void WMInterface::removeSticky(Window win,int keep_on_desktop)
{
  int i;

  for (i=0;i<bbtool->getNumberOfDesktops();i++) {
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
  }
}

void WMInterface::changeIconState(Window win) {

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

void WMInterface::windowAttributeChange(Window win) {
  Atom real_type;
  int format;
  double ydiv;
  unsigned long n, extra;
  WindowList *tmp=0;

  BlackboxHints *net_hint;

  {
    LinkedListIterator<WindowList> win_it(bbtool->windowList);
    for (; win_it.current(); win_it++)
      if ((win_it.current()->win) == win) {
        tmp = win_it.current();
    }
  }
  if (tmp) {

    if (XGetWindowProperty(bbtool->getXDisplay(), win, 
                          bbtool->getBlackboxAttributesAtom(), 0L, 
                          PropBlackboxHintsElements, False, 
                          bbtool->getBlackboxAttributesAtom(), 
                          &real_type, &format,&n, &extra,  
                          (unsigned char**)&net_hint) == Success && net_hint) {

  
      if (n == PropBlackboxHintsElements) {

        if (net_hint->flags & AttribShaded) {
          if (net_hint->attrib & AttribShaded) {
            tmp->pager_height=2;
            XResizeWindow(bbtool->getXDisplay(),tmp->pager_win,tmp->pager_width,
                        tmp->pager_height);
            tmp->shaded=True;
          }
        } else if (tmp->shaded) {
          ydiv=(double)bbtool->resource->desktopSize.height/
          bbtool->getCurrentScreenInfo()->getHeight();
          tmp->pager_height=(unsigned int)(tmp->height*ydiv);
          XResizeWindow(bbtool->getXDisplay(),tmp->pager_win,tmp->pager_width,
                        tmp->pager_height);
          tmp->shaded=False;
        } 

        if (isIconicState(win)!=tmp->icon&&!tmp->shaded)
          changeIconState(win);
        
          if (net_hint->flags & AttribOmnipresent) {
            if (net_hint->attrib & AttribOmnipresent) {
              addSticky(tmp);
              tmp->sticky=True;
            }
          } else if (tmp->sticky) { 
            removeSticky(tmp->win,bbtool->getCurrentDesktopNr());
            tmp->sticky=False;
          }

   
//        if (net_hint->flags & AttribWorkspace)
//          tmp->setDesktopNr(net_hint->workspace);

//        if (net_hint->flags & AttribDecoration) {
//          tmp->setDecoration(net_hint->decoration);
//          fprintf(stderr,"%ul\n",net_hint->decoration);
//        }
      }
    }
  }
}


void WMInterface::BlackboxNotifyStartup() {
}

void WMInterface::BlackboxNotifyWindowAdd(Window win,int desktop_nr) {
  WindowList *tmp = new WindowList;
  tmp->win= win;
  int status=bbtool->getWindowGeometry(tmp);
  if (status) {
    if (isIconicState(tmp->win)) tmp->icon=True;
    else tmp->icon=False;
    tmp->sticky=False;
    tmp->focused=False;
    tmp->shaded=False;
    tmp->desktop_nr=desktop_nr;

    if (tmp->desktop_nr>=0) {
      bbtool->addFrameWindow(tmp,0,False);
      bbtool->windowList->insert(tmp);
      if (getAttributes(tmp->win)&AttribOmnipresent)
        addSticky(tmp);
    }
    else delete tmp;
  } else delete tmp;
}

void WMInterface::BlackboxNotifyDel(Window win) {
  bbtool->removeWindow(win);
}

void WMInterface::BlackboxNotifyAttributes(Window win) {
  windowAttributeChange(win);
}

void WMInterface::BlackboxNotifyFocus(Window win) {
  if (bbtool->getResource()->getFocusStyle()!=none)
   bbtool->focusWindow(win);
}

void WMInterface::BlackboxNotifyCurrentWorkspace(int current_desktop) {
  bbtool->desktopChange(current_desktop);
//      bbtool->redraw=True;
}

void WMInterface::BlackboxNotifyWindowRaise(Window win) {
  bbtool->raiseWindow(win);
}

void WMInterface::BlackboxNotifyWindowLower(Window win) {
  bbtool->lowerWindow(win);
}

void WMInterface::BlackboxNotifyWorkspaceCount(int number_of_desktops) {
  int old_number_of_desktops = bbtool->getNumberOfDesktops();
  bbtool->setNumberOfDesktops(number_of_desktops);
  if (number_of_desktops>old_number_of_desktops) {
  int i;
  for (i=old_number_of_desktops;i<number_of_desktops;i++) {
    DesktopList *tmp = new DesktopList;
    bbtool->addDesktopWindow(tmp,False);
    bbtool->desktopList->insert(tmp);
    LinkedListIterator<WindowList> win_it(bbtool->windowList);
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
      }
    }
  } else if (number_of_desktops<old_number_of_desktops)
    bbtool->removeDesktopWindow();
}
