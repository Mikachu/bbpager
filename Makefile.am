CPPFLAGS =	@CPPFLAGS@  @INTERLACE@ \
		-DGLOBAL_BB=\"$(datadir)/bbtools/bbpager.bb\" \
		-DGLOBAL_NOBB=\"$(datadir)/bbtools/bbpager.nobb\"


SUBDIRS = data
bin_PROGRAMS = bbpager
bbpager_SOURCES = bbpager.cc Image.cc LinkedList.cc \
  		  bbpager.hh Image.hh LinkedList.hh \
		  Timer.hh Timer.cc \
		  main.cc main.hh \
		  resource.cc resource.hh \
		  Baseresource.cc Baseresource.hh \
		  Basewindow.cc Basewindow.hh \
		  BaseDisplay.cc BaseDisplay.hh \
		  wminterface.cc wminterface.hh \
		  NETInterface.cc NETInterface.hh \
		  blackboxstyle.hh
EXTRA_DIST = BUGS TODO
bbpager_LDADD = @X_LIBS@
