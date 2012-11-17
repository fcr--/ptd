CFLAGS += -Wall -std=c99
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

main.c: interface.h
interface.h: interface.xml
	echo 'static const char * dbus_interface =' > $@
	xmllint --valid --noblanks interface.xml | tr -d '\n' | \
	  sed -e ':a;s/\(.*\)<!--.*-->/\1/;ta;s/'"'/\&quot;/g;" \
	  -e "s/'\([A-Z_a-z0-9]*\)'/\1/g;"'s/"/'"'/g;"'s/.*/    "&";/' \
	  >> $@
	echo >> $@

clean:
	rm -f example ptd *.o interface.h
