//  BlackboxInterface.cc for bbtools.
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

#include "NETInterface.hh"

BlackboxInterface::BlackboxInterface(Basewindow *basewindow) {

  timer=new BTimer(basewindow->getCurrentScreenInfo()->getBaseDisplay(),this) ;
  base=basewindow;
  net_init=False;
  timer->setTimeout(10000);
  timer->fireOnce(True);
  timer->start();
}

void BlackboxInterface::handleBlackboxEvents(XEvent Event) {
  if (Event.xclient.message_type==base->getBlackboxStructureMessagesAtom()) {
    if ((unsigned)Event.xclient.data.l[0]==base->getBlackboxNotifyStartupAtom()) {
      BlackboxNotifyStartup();
      net_init=True;
    }
    else if ((unsigned)Event.xclient.data.l[0]==
                                      base->getBlackboxNotifyWindowRaiseAtom()) {
      BlackboxNotifyWindowRaise(Event.xclient.data.l[1]);
    }
    else if ((unsigned)Event.xclient.data.l[0]==
                                      base->getBlackboxNotifyWindowLowerAtom()) {
      BlackboxNotifyWindowLower(Event.xclient.data.l[1]);
    }
    else if ((unsigned)Event.xclient.data.l[0]==
                                      base->getBlackboxNotifyWindowAddAtom()) {
      BlackboxNotifyWindowAdd(Event.xclient.data.l[1],Event.xclient.data.l[2]);
    }
    else if ((unsigned)Event.xclient.data.l[0]==
                                      base->getBlackboxNotifyWindowDelAtom()) {
      BlackboxNotifyDel(Event.xclient.data.l[1]);
    }
    else if ((unsigned)Event.xclient.data.l[0]==
                                      base->getBlackboxAttributesAtom()) {
      BlackboxNotifyAttributes(Event.xclient.data.l[1]);
    }
    else if ((unsigned)Event.xclient.data.l[0]==
                                      base->getBlackboxNotifyWindowFocusAtom()){
      BlackboxNotifyFocus(Event.xclient.data.l[1]);
    }
    if ((unsigned)Event.xclient.data.l[0]==
                                   base->getBlackboxNotifyCurrentWorkspaceAtom()){
      BlackboxNotifyCurrentWorkspace(Event.xclient.data.l[1]);
    }
    else if ((unsigned)Event.xclient.data.l[0]==
                                    base->getBlackboxNotifyWorkspaceCountAtom()) {
      BlackboxNotifyWorkspaceCount(Event.xclient.data.l[1]);
    }
  }
}

void BlackboxInterface::timeout() {
  if (!net_init) {
    fprintf(stderr,"Cannot connect to window manager\n");
    base->shutdown();
  }
}
