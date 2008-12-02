# Uncomment one set of these lines for your expat libraries

#EXPAT_OPTS=-DEXPAT_XMLPARSE -I/usr/include/w3c-libwww
#EXPAT_LIB=-lxmlparse -lxmltok -DEXPAT_XMLPARSE

EXPAT_OPTS=-DEXPAT_EXPAT
EXPAT_LIB=-lexpat

### End of changes

LIBS=$(EXPAT_LIB)
CC=gcc
CCOPTS=-I/usr/include/w3c-libwww -O2 -Wall -fno-strict-aliasing $(EXPAT_OPTS)

all: gpxrewrite

gpxrewrite: gpxrewrite.c
	$(CC) $(CCOPTS) gpxrewrite.c $(LIBS) -o gpxrewrite
	strip --strip-unneeded gpxrewrite

test: gpxrewrite
	./gpxrewrite settings.ini test.gpx

clean:
	rm -f *~ gpxrewrite
