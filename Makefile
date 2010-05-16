#
#  This file is part of Dismemberd.
#
#  Dismemberd is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  Dismemberd is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with Dismemberd.  If not, see <http://www.gnu.org/licenses/>.

prefix = /usr/local
exec_prefix = $(prefix)
bindir = $(exec_prefix)/bin
sysconfdir = /etc

OBJS = dismemberd.o logging.o groups.o options.o

CPPFLAGS = -I. $(shell pkg-config --cflags glib-2.0)
LIBS = -lcpg $(shell pkg-config --libs glib-2.0)
CFLAGS = -g

%.func.h: %.c
	cproto $(CPPFLAGS) -ev -o $@ $<

all: dismemberd

dismemberd: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

install: all
	install -d -m 755 $(DESTDIR)$(bindir)
	install -d -m 755 $(DESTDIR)$(sysconfdir)/event.d
	install -d -m 755 $(DESTDIR)$(sysconfdir)/dsh/group
	install -m 755 dismemberd $(DESTDIR)$(bindir)/dismemberd
	install -m 644 dismemberd.upstart $(DESTDIR)$(sysconfdir)/event.d/dismemberd

clean:
	rm -f $(OBJS)
	rm -f dismemberd

dismemberd.o: dismemberd.c dismemberd.h groups.h logging.h options.h version.h

