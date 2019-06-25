bin_PROGRAMS    = hdslog
SUBDIRS=icons
hdslog_SOURCES   =\
	 main.c main.h configfile.c configfile.h gui.c gui.h gtkut.c gtkut.h tree.c tree.h http-client.c mltree.c mltree.h resources.c resources.h mail.h
INCLUDES        = @CFITSIO_CFLAGS@ @GTK_CFLAGS@ $(GTK3_CFLAGS)
LDADD           = @CFITSIO_LIBS@ @GTK_LIBS@ $(GTK3_LIBS)

# for ANA
# INCLUDES        = -I/opt/share/pgplot/g77_gcc @GTK_CFLAGS@
# LDADD           = -L/opt/sahre/pgplot/g77_gcc  -lcpgplot -lpgplot -lg2c @GTK_LIBS@ 

CLEANFILES      = *~ icons/*~
DISTCLEANFILES  = .deps/*.P
AM_CPPFLAGS = -DLOCALEDIR=\""$(localedir)"\" -DCOMMON_DIR=\""$(commondir)"\"

EXTRA_DIST = autogen.sh\
	     gresource.xml

BUILT_SOURCES=resources.c resources.h
CLEANFILES += $(BUILT_SOURCES)

resource_files=$(shell $(GLIB_COMPILE_RESOURCES) --sourcedir=$(srcdir) --generate-dependencies $(srcdir)/gresource.xml)

resources.h:
	$(AM_V_GEN) $(GLIB_COMPILE_RESOURCES) --target=$@ --sourcedir=$(srcdir) --generate-header $(srcdir)/gresource.xml

resources.c: gresource.xml $(resource_files) resources.h
	$(AM_V_GEN) $(GLIB_COMPILE_RESOURCES) --target=$@ --sourcedir=$(srcdir) --generate-source $(srcdir)/gresource.xml

GZIP_ENV =
BZIP2_ENV =
TAR=tar

ACLOCAL_AMFLAGS = -l m4

release: distdir
	-chmod -R a+r $(distdir)
	GZIP=$(GZIP_ENV) $(TAR) chozf $(distdir).tar.gz $(distdir)
	-rm -rf $(distdir)

