dnl Process this file with autoconf to produce a configure script.
AC_INIT(src/bbpager.cxx)
AM_INIT_AUTOMAKE(bbpager, 0.4.7)

AC_PREFIX_DEFAULT(/usr/local)

AM_MAINTAINER_MODE

dnl get the debug values right
AC_DEFUN(AC_SET_DEBUG,
[
 test "$CXXFLAGS" = "" && CXXFLAGS="-g -Wall -DDEBUG"
 test "$LDFLAGS" = "" && LDFLAGS=""
])

AC_DEFUN(AC_SET_NODEBUG,
[
 test "$CXXFLAGS" = "" && CXXFLAGS="-O2 -Wall"
 test "$LDFLAGS" = "" && LDFLAGS="-s"
])

AC_ARG_ENABLE(debug,[  --enable-debug          create debugging code [default=no]],
[
if test $enableval = "no";
 then AC_SET_NODEBUG
 else AC_SET_DEBUG
fi
],
AC_SET_NODEBUG)

dnl Checks for programs.
dnl needed by configure itsself
AC_PROG_CC
AC_PROG_CXX
AC_PROG_CPP
AC_PROG_INSTALL
AC_HEADER_STDC
AC_CHECK_HEADERS(ctype.h fcntl.h libgen.h locale.h process.h signal.h stdio.h time.h unistd.h sys/param.h sys/select.h sys/stat.h sys/time.h sys/types.h sys/wait.h)
AC_HEADER_TIME

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_STRUCT_TM

dnl Checks for library functions.
AC_CHECK_FUNCS(select strstr)

AC_CHECK_FUNCS(setlocale sigaction strftime)

AC_PATH_X
AC_PATH_XTRA

dnl Checks for X libraries.
AC_CHECK_LIB([X11],
	[XOpenDisplay],
	[],
	[AC_MSG_ERROR(XOpenDisplay not found in -lX11)]
	)

dnl Checks for Xextension
PKG_CHECK_MODULES([XEXT], [xext],
	[
	AC_CHECK_LIB([Xext],
		[XMissingExtension],
		[],
	        [AC_MSG_ERROR(XMissingExtension not found in -lXext)]
		)
	],
	[AC_MSG_ERROR(xext not found)]
	)

dnl Checks for Xinerama extension (optional, enables -head)
PKG_CHECK_MODULES([XINERAMA], [xinerama],
	[
	AC_DEFINE([HAVE_XINERAMA], [1], [Define if the Xinerama extension is available])
	have_xinerama=yes
	],
	[have_xinerama=no]
	)

dnl pkgconfig checks for blackbox library
PKG_CHECK_MODULES(LIBBT, libbt)

LIBS="$LIBS $LIBBT_LIBS $X_LIBS $X_PRE_LIBS $XEXT_LIBS $XINERAMA_LIBS"

dnl generate the config header
AM_CONFIG_HEADER(config.h)

dnl Print results
AC_MSG_RESULT([])
AC_MSG_RESULT([	$PACKAGE version $VERSION configured successfully.])
AC_MSG_RESULT([])
AC_MSG_RESULT([Using '$prefix' for installation.])
AC_MSG_RESULT([Using '$CXX' for C++ compiler.])
AC_MSG_RESULT([Building with '$CXXFLAGS' for C++ compiler flags.])
AC_MSG_RESULT([Building with '$LIBS' for linker flags.])
AC_MSG_RESULT([])

AC_OUTPUT(
Makefile
src/Makefile
data/Makefile
)
