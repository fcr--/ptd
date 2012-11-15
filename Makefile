CFLAGS += -Wall

CFLAGS += `pkg-config --cflags glib-2.0`
LIBS += `pkg-config --libs glib-2.0` -lnetfilter_queue

all: example main

example: example.c
	$(CC) example.c -o example $(CFLAGS) $(LIBS)

main: main.c
	$(CC) $< -o $@ $(CFLAGS) $(LIBS)

clean:
	rm -f example main
