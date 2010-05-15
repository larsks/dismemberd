prefix = /usr/local
exec_prefix = $(prefix)
bindir = $(exec_prefix)/bin
sysconfdir = /etc

OBJS = dismemberd.o

CPPFLAGS = -I. $(shell pkg-config --cflags glib-2.0)
LIBS = -lcpg $(shell pkg-config --libs glib-2.0)
CFLAGS = -g

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
