CPPFLAGS = @CPPFLAGS@ @SHAPE@ \
-DBBTOOL_GLOBAL=\"$(datadir)/bbtools/bbpager.rc\" \
-DBBTOOL_LOCAL=\"~/.bbtools/bbpager.rc\"
				

SUBDIRS = data scripts
man_MANS = bbpager.1
bin_PROGRAMS = bbpager
bbmail_SOURCES = bbpager.cc bbpager.hh \
		 main.cc main.hh \
		 Baseresource.cc Baseresource.hh \
		 resource.cc resource.hh \
		 wminterface.cc wminterface.hh \
		 NETInterface.cc NETInterface.hh \
		 blackboxstyle.hh
#		 pagermenu.cc pagermenu.hh \
EXTRA_DIST = $(man_MANS) BUGS TODO
bbmail_LDADD = @X_LIBS@

