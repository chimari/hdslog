dnl Process this file with autoconf to produce a configure script.
AC_INIT(main.c)
PACKAGE=hdslog

dnl version number
MAJOR_VERSION=1
<<<<<<< HEAD
MINOR_VERSION=2
MICRO_VERSION=0
EXTRA_VERSION=-20191226
=======
MINOR_VERSION=1
MICRO_VERSION=2
EXTRA_VERSION=-20190624
>>>>>>> f14475d9fae8ac398924a63a2320b86cd4f3844c
VERSION=$MAJOR_VERSION.$MINOR_VERSION.$MICRO_VERSION$EXTRA_VERSION

dnl set $target
AC_CANONICAL_SYSTEM

AM_INIT_AUTOMAKE($PACKAGE, $VERSION, no-define)
dnl AC_DEFINE_UNQUOTED(PACKAGE, "$PACKAGE")
dnl AC_DEFINE_UNQUOTED(VERSION, "$VERSION")
AC_SUBST(PACKAGE)
AC_SUBST(VERSION)


AM_CONFIG_HEADER(config.h)
dnl AC_ARG_PROGRAM
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET

dnl if test x"${use_gtk1}" = "xyes"; then
dnl AM_PATH_GLIB(1.2.6,,
dnl        AC_MSG_ERROR(Test for GLIB failed. See the file 'INSTALL' for help.),
dnl         gthread)
dnl AM_PATH_GTK(1.2.0, ,
dnl             AC_MSG_ERROR(Cannot find GTK: Is gtk-config in path?))
dnl else
AM_PATH_GLIB_2_0(2.32.0,, AC_MSG_ERROR([glib-2.x is required]), gthread gobject)
dnl fi

dnl GTK 3 or 2 
AC_ARG_WITH(gtk3,
             AC_HELP_STRING([--with-gtk3],
                        [use Gtk+3 instead of Gtk+2]),
                        [use_gtk3=yes])
if test x"${use_gtk3}" = "xyes"; then
GTK3_CFLAGS="`pkg-config gtk+-3.0 --cflags`"
GTK3_LIBS="`pkg-config gtk+-3.0 --libs`"
dnl    AM_PATH_GTK_3_0(3.22.0,, AC_MSG_ERROR([gtk+-2.x is required]), gthread gobject)
AC_DEFINE([USE_GTK3],[],[Use Gtk+3.x instead of Gtk+2])
else
AM_PATH_GTK_2_0(2.24.0,, AC_MSG_ERROR([gtk+-2.x is required]), gthread gobject)
AC_DEFINE([USE_GTK2],[],[Use Gtk+2.x instead of Gtk+3])
GTK3_CFLAGS=""
GTK3_LIBS=""
fi
AC_SUBST(GTK3_LIBS)
AC_SUBST(GTK3_CFLAGS)


dnl Check for OpenSSL
	PKG_CHECK_MODULES(OPENSSL, openssl, [:], [:])
	if test "x$OPENSSL_LIBS" != "x"; then
		CFLAGS="$CFLAGS $OPENSSL_CFLAGS"
		LIBS="$LIBS $OPENSSL_LIBS"
		AC_MSG_CHECKING([if openssl is available])
		AC_MSG_RESULT(yes)
		AC_DEFINE(USE_SSL, 1, Define if you use OpenSSL to support SSL.)
	else
                AC_MSG_ERROR([OpenSSL is required.])
	fi

dnl cfitsio
AC_ARG_WITH(cfitsio,
		 AC_HELP_STRING([--without-cfitsio],
		            [make without plot function]),
			    [without_cfitsio=yes])

AC_ARG_WITH(cfitsio-libraries,[  --with-cfitsio-libraries=DIR   cfitsio lib. dir. (def. /opt/share/cfitsio)], cfitsio_libraries="$withval", cfitsio_libraries="")
AC_ARG_WITH(cfitsio-includes,[  --with-cfitsio-includes=DIR   cfitsio include dir. (def. /opt/share/cfitsio)], cfitsio_includes="$withval", cfitsio_includes="")

if test x"${without_cfitsio}" = "xyes"; then
AC_DEFINE([WITHOUT_CFITSIO],[],[Without CFITSIO function])
CFITSIO_CFLAGS=""
CFITSIO_LIBS=""
else
 	if test "x$cfitsio_libraries" != "x" ; then
  		CFITSIO_LIBS="-L$cfitsio_libraries"
  	else
    		CFITSIO_LIBS="-L/usr/lib64"
  	fi

 	CFITSIO_LIBS="$CFITSIO_LIBS -lcfitsio -lnsl"

  	if test "x$cfitsio_includes" != "x" ; then
  		CFITSIO_CFLAGS="-I$cfitsio_includes"
  	else
    		CFITSIO_CFLAGS="-I/usr/include/cfitsio"
  	fi
fi
AC_SUBST(CFITSIO_LIBS)
AC_SUBST(CFITSIO_CFLAGS)

AC_CHECK_HEADERS([pwd.h sys/utsname.h])

AC_OUTPUT([
Makefile
icons/Makefile
version.h
])


dnl Output the configuration summary
echo ""
echo "####################"
echo " $PACKAGE $VERSION"
echo "####################"
echo ""
echo ""
echo "Configure finished, type 'make' to build."
