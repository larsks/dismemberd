CPPFLAGS = -I. $(shell pkg-config --cflags glib-2.0)
LIBS = -lcpg $(shell pkg-config --libs glib-2.0)
CFLAGS = -g

all: testcpg memberd

memberd: memberd.o daemonize.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

