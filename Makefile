CFLAGS += -Wall
PKGS += glib-2.0
PKGS += gio-2.0

CFLAGS += `pkg-config --cflags $(PKGS)`
LIBS += `pkg-config --libs $(PKGS)` -lnetfilter_queue

all: example main

example: example.c
	$(CC) example.c -o example $(CFLAGS) $(LIBS)

main: main.c
	$(CC) $< -o $@ $(CFLAGS) $(LIBS)

clean:
	rm -f example main
