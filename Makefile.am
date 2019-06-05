bin_PROGRAMS    = hdslog
hdslog_SOURCES   =\
	 main.c main.h configfile.c configfile.h gui.c gui.h gtkut.c gtkut.h tree.c tree.h http-client.c
INCLUDES        = @CFITSIO_CFLAGS@ @GTK_CFLAGS@
LDADD           = @CFITSIO_LIBS@ @GTK_LIBS@ 

# for ANA
# INCLUDES        = -I/opt/share/pgplot/g77_gcc @GTK_CFLAGS@
# LDADD           = -L/opt/sahre/pgplot/g77_gcc  -lcpgplot -lpgplot -lg2c @GTK_LIBS@ 

CLEANFILES      = *~
DISTCLEANFILES  = .deps/*.P
AM_CPPFLAGS = -DLOCALEDIR=\""$(localedir)"\" -DCOMMON_DIR=\""$(commondir)"\"

EXTRA_DIST = autogen.sh

GZIP_ENV =
BZIP2_ENV =
TAR=tar

ACLOCAL_AMFLAGS = -l m4

release: distdir
	-chmod -R a+r $(distdir)
	GZIP=$(GZIP_ENV) $(TAR) chozf $(distdir).tar.gz $(distdir)
	-rm -rf $(distdir)

