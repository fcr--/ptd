CFLAGS += -Wall
PKGS += glib-2.0
PKGS += gio-2.0

CFLAGS += `pkg-config --cflags $(PKGS)`
LIBS += `pkg-config --libs $(PKGS)` -lnetfilter_queue

all: example ptd

example: example.c
	$(CC) example.c -o example $(CFLAGS) $(LIBS)

ptd: main.o rules.o
	$(CC) $^ -o $@ $(LIBS)

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	rm -f example ptd *.o
