# Generated automatically from Makefile.in by configure.
#
#    This source code is free software; you can redistribute it
#    and/or modify it in source code form under the terms of the GNU
#    Library General Public License as published by the Free Software
#    Foundation; either version 2 of the License, or (at your option)
#    any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU Library General Public License for more details.
#
#    You should have received a copy of the GNU Library General Public
#    License along with this program; if not, write to the Free
#    Software Foundation, Inc.,
#    59 Temple Place - Suite 330
#    Boston, MA 02111-1307, USA
#
#

SHELL = /bin/sh

VERSION = 0.0

prefix      := /usr/local
exec_prefix := ${prefix}
srcdir      := .
bindir      := ${exec_prefix}/bin
libdir      := ${exec_prefix}/lib
includedir  := $(prefix)/include

CC              := gcc
CXX             := g++
INSTALL         := /usr/bin/install -c
INSTALL_PROGRAM := ${INSTALL}
INSTALL_DATA    := ${INSTALL} -m 644

CFLAGS   := -Wall -std=gnu99 -g -O2 -DHAVE_CONFIG_H -fPIC
CXXFLAGS := -Wall -std=gnu++11 -g -O2 -DHAVE_CONFIG_H -fPIC
LDFLAGS  := -Wl,--cref
INCDIRS  := -I$(srcdir)/..
INCDIRS  += -I/usr/local/include/iverilog


all: edif.tgt

%.o: %.c
	@[ -d dep ] || mkdir dep
	$(CC) $(INCDIRS) $(CFLAGS) -MD -c $< -o $*.o
	mv $*.d dep


OBJS := edif.o string_cache.o


ifeq (no,yes)
	TGTLDFLAGS = -L.. -livl
	TGTDEPLIBS = ../libivl.a
else
	TGTLDFLAGS =
	TGTDEPLIBS =
endif


edif.tgt: $(OBJS) $(TGTDEPLIBS)
	$(CC) -shared -o $@ $(OBJS) $(TGTLDFLAGS)


clean:
	rm -f *.o dep/*.d *.bck *.bak core


install: all installdirs $(libdir)/ivl/edif.tgt \
	$(includedir)/vpi_user.h


$(libdir)/ivl/edif.tgt: ./edif.tgt
	$(INSTALL_DATA) ./edif.tgt $(libdir)/ivl/edif.tgt


installdirs: ../mkinstalldirs
	$(srcdir)/../mkinstalldirs $(includedir) $(bindir) $(libdir)/ivl


uninstall:
	rm -f $(libdir)/ivl/edif.tgt


-include $(patsubst %.o, dep/%.d, $(OBJS))
