CC=gcc
#EXPAT_LIB=-lxmlparse -lxmltok -DEXPAT_XMLPARSE -I/usr/include/w3c-libwww
EXPAT_LIB=-lexpat -DEXPAT_EXPAT -I/usr/include/w3c-libwww
CCOPTS=-static -O2 -Wall -fno-strict-aliasing

all: gpxrewrite

gpxrewrite: gpxrewrite.c \
	ini_settings.h mem_str.h waypoint.h \
	ini_settings.o mem_str.o waypoint.o
	$(CC) $(CCOPTS) gpxrewrite.c \
		ini_settings.o mem_str.o waypoint.o \
		$(EXPAT_LIB) -o gpxrewrite
	strip --strip-unneeded gpxrewrite

ini_settings.o: ini_settings.c
	$(CC) $(CCOPTS) -c ini_settings.c -o ini_settings.o

mem_str.o: mem_str.c
	$(CC) $(CCOPTS) -c mem_str.c -o mem_str.o

waypoint.o: waypoint.c mem_str.h waypoint.h
	$(CC) $(CCOPTS) -c waypoint.c -o waypoint.o

test: gpxrewrite
	./gpxrewrite settings.ini test.gpx

clean:
	rm -f *~ *.o gpxrewrite
