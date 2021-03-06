# Automake file for external dynamic modules for NEST
#
# Hans Ekkehard Plesser, April 2008
# Automake file for the Developer Module
#
# libmymodule is built as a normal, installable library.
# It will be installed to $prefix/lib by make install.
#
# Headers from this directory are not to be installed upon
# make install. They are therefore included in _SOURCES.

libdir= @libdir@/nest

# We need to install the module header for static linking on BlueGene
include_HEADERS = stdpmodule.h

# All other source files
source_files=	stdpnames.h stdpmodule.cpp \
				stdp_triplet_connection.h \
				stdp_triplet_node.h stdp_triplet_node.cpp \
				stdp_longterm_node.h stdp_longterm_node.cpp

if BUILD_DYNAMIC_USER_MODULES
  lib_LTLIBRARIES= libstdpmodule.la stdpmodule.la
  stdpmodule_la_CXXFLAGS= @AM_CXXFLAGS@
  stdpmodule_la_SOURCES=  $(source_files)
  stdpmodule_la_LDFLAGS=  -module
else
  lib_LTLIBRARIES= stdpmodule.la
endif

libstdpmodule_la_CXXFLAGS= @AM_CXXFLAGS@ -DLINKED_MODULE
libstdpmodule_la_SOURCES=  $(source_files)

MAKEFLAGS= @MAKE_FLAGS@

AM_CPPFLAGS= @NEST_CPPFLAGS@ \
             @INCLTDL@

.PHONY: install-slidoc

pkgdatadir=@datadir@/nest

nobase_pkgdata_DATA=\
	sli/stdpmodule-init.sli

install-slidoc:
	NESTRCFILENAME=/dev/null $(DESTDIR)$(NEST_PREFIX)/bin/sli --userargs="@HELPDIRS@" $(NEST_PREFIX)/share/nest/sli/install-help.sli

install-data-hook: install-exec install-slidoc install-py

EXTRA_DIST= sli

install-py:
	find $(NEST_PREFIX)/lib -type d -name 'site-packages' -exec cp $(top_srcdir)/pynest/* "{}/nest/lib/" \;
