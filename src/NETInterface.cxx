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


BlackboxInterface::BlackboxInterface(Basewindow *basewindow) 
{
        base=basewindow;
        net_init=False;
}

/*
bool KWinModulePrivate::x11Event( XEvent * ev ) //from kdelibs/kdecore/kwinmodule.cpp
{
    if ( ev->xany.window == qt_xrootwin() ) {
	int m = NETRootInfo::event( ev );

	if ( m & CurrentDesktop )
	    for ( module = modules.first(); module; module = modules.next() )
		emit module->currentDesktopChanged( currentDesktop() );
	if ( m & ActiveWindow )
	    for ( module = modules.first(); module; module = modules.next() )
		emit module->activeWindowChanged( activeWindow() );
	if ( m & DesktopNames )
	    for ( module = modules.first(); module; module = modules.next() )
		emit module->desktopNamesChanged();
	if ( m & NumberOfDesktops )
	    for ( module = modules.first(); module; module = modules.next() )
		emit module->numberOfDesktopsChanged( numberOfDesktops() );
	if ( m & WorkArea )
	    for ( module = modules.first(); module; module = modules.next() )
		emit module->workAreaChanged();
	if ( m & ClientListStacking ) {
	    updateStackingOrder();
	    for ( module = modules.first(); module; module = modules.next() )
		emit module->stackingOrderChanged();
	}
    } else  if ( windows.contains( ev->xany.window ) ){
	NETWinInfo ni( qt_xdisplay(), ev->xany.window, qt_xrootwin(), 0 );
	unsigned int dirty = ni.event( ev );
	if ( !dirty && ev->type ==PropertyNotify && ev->xproperty.atom == XA_WM_HINTS )
	    dirty |= NET::WMIcon; // support for old icons
	if ( (dirty & NET::WMStrut) != 0 ) {
	    if ( !strutWindows.contains( ev->xany.window )  )
		strutWindows.append( ev->xany.window );
	}
	if ( dirty ) {
	    for ( module = modules.first(); module; module = modules.next() ) {
		emit module->windowChanged( ev->xany.window );
		emit module->windowChanged( ev->xany.window, dirty );
		if ( (dirty & NET::WMStrut) != 0 )
		    emit module->strutChanged();
	    }
	}
    }

    return FALSE;
}

// from kdelibs/kdecore/netwm.cpp 
// unsigned long NETRootInfo::event(XEvent *event) { 
//    unsigned long dirty = 0;
<snip>

    if (event->type == PropertyNotify) {

#ifdef    NETWMDEBUG
	fprintf(stderr, "NETRootInfo::event: handling PropertyNotify event\n");
#endif

	XEvent pe = *event;

	Bool done = False;
	Bool compaction = False;
	while (! done) {

#ifdef   NETWMDEBUG
	    fprintf(stderr, "NETRootInfo::event: loop fire\n");
#endif

	    if (pe.xproperty.atom == net_client_list)
		dirty |= ClientList;
	    else if (pe.xproperty.atom == net_client_list_stacking)
		dirty |= ClientListStacking;
	    else if (pe.xproperty.atom == kde_net_system_tray_windows)
		dirty |= KDESystemTrayWindows;
	    else if (pe.xproperty.atom == net_desktop_names)
		dirty |= DesktopNames;
	    else if (pe.xproperty.atom == net_workarea)
		dirty |= WorkArea;
	    else if (pe.xproperty.atom == net_number_of_desktops)
		dirty |= NumberOfDesktops;
	    else if (pe.xproperty.atom == net_desktop_geometry)
		dirty |= DesktopGeometry;
	    else if (pe.xproperty.atom == net_desktop_viewport)
		dirty |= DesktopViewport;
	    else if (pe.xproperty.atom == net_current_desktop)
		dirty |= CurrentDesktop;
	    else if (pe.xproperty.atom == net_active_window)
		dirty |= ActiveWindow;
	    else {

#ifdef    NETWMDEBUG
		fprintf(stderr, "NETRootInfo::event: putting back event and breaking\n");
#endif

		if ( compaction )
		    XPutBackEvent(p->display, &pe);
		break;
	    }

	    if (XCheckTypedWindowEvent(p->display, p->root, PropertyNotify, &pe) )
		compaction = True;
	    else
		break;
	}

	update(dirty & p->protocols);
    }

#ifdef   NETWMDEBUG
    fprintf(stderr, "NETRootInfo::event: handled events, returning dirty = 0x%lx\n",
	    dirty & p->protocols);
#endif

    return dirty & p->protocols;
}

//from kdebase/kpager/kpgaer.cpp

    connect( m_winmodule, SIGNAL( activeWindowChanged(WId)),
             SLOT(slotActiveWindowChanged(WId)));
    connect( m_winmodule, SIGNAL( windowAdded(WId) ),
             SLOT( slotWindowAdded(WId) ) );
    connect( m_winmodule, SIGNAL( windowRemoved(WId) ),
             SLOT( slotWindowRemoved(WId) ) );
    connect( m_winmodule, SIGNAL( windowChanged(WId,unsigned int) ),
             SLOT( slotWindowChanged(WId,unsigned int) ) );
    connect( m_winmodule, SIGNAL( stackingOrderChanged() ),
             SLOT( slotStackingOrderChanged() ) );
    connect( m_winmodule, SIGNAL( desktopNamesChanged() ),
             SLOT( slotDesktopNamesChanged() ) );
    connect( m_winmodule, SIGNAL( numberOfDesktopsChanged(int) ),
             SLOT( slotNumberOfDesktopsChanged(int) ) );
    connect( m_winmodule, SIGNAL( currentDesktopChanged(int)),
             SLOT( slotCurrentDesktopChanged(int) ) );
    connect(kapp, SIGNAL(backgroundChanged(int)),
            SLOT(slotBackgroundChanged(int)));

    QFont defFont("Helvetica", 10, QFont::Bold);
    defFont = cfg->readFontEntry("Font", &defFont);
    setFont(defFont);

    m_prefs_action = KStdAction::preferences(this, SLOT(configureDialog()), parent->actionCollection());
    m_quit_action = KStdAction::quit(kapp, SLOT(quit()), parent->actionCollection());

    updateLayout();
}
*/


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
