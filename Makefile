prefix = /usr/local
exec_prefix = $(prefix)
bindir = $(exec_prefix)/bin

CPPFLAGS = -I. $(shell pkg-config --cflags glib-2.0)
LIBS = -lcpg $(shell pkg-config --libs glib-2.0)
CFLAGS = -g

all: memberd

memberd: memberd.o daemonize.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

install: all
	install -d -m 755 $(bindir)
	install -m 755 memberd $(bindir)/memberd
