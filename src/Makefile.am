AM_CPPFLAGS = $(CPPFLAGS) -DSHAPE \
-I$(top_srcdir)/lib \
-DBBTOOL_GLOBAL=\"$(datadir)/bbtools/bbpager.rc\" \
-DBBTOOL_LOCAL=\"~/.bbtools/bbpager.rc\"
AM_CXXFLAGS = $(X_CFLAGS) $(XEXT_CFLAGS)
LDADD = ../lib/libbt.a
				
bin_PROGRAMS = bbpager
bbpager_SOURCES = bbpager.cxx bbpager.h \
		 main.cxx main.h \
		 Baseresource.cxx Baseresource.h \
		 resource.cxx resource.h \
		 wminterface.cxx wminterface.h \
		 blackboxstyle.h \
         pager.cxx pager.h \
         desktop.cxx desktop.h
#		 pagermenu.cc pagermenu.hh \
#


