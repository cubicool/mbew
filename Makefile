SOURCES = \
	src/mbew.c \
	src/mbew-format.c \
	src/mbew-io.c \
	src/mbew-iterate.c

EXAMPLE01 = examples/mbew-properties.c
EXAMPLE02 = examples/mbew-iterate.c
EXAMPLE03 = examples/mbew-strings.c
EXAMPLE04 = examples/mbew-video-cairo.c

STATIC_LIBS = \
	ext/libvpx/libvpx.a \
	ext/nestegg/libnestegg.a

DYNAMIC_LIBS = \
	-lcairo \
	-lpthread \
	-lm

CFLAGS = $(shell cat .syntastic) -g

all: mbew-properties mbew-iterate mbew-strings mbew-video-cairo

ext/libvpx/libvpx.a:
	@(cd ext/libvpx; ./configure --disable-unit-tests --disable-examples; make)

ext/nestegg/libnestegg.a:
	@(cd ext/nestegg; gcc -Iinclude -c -o nestegg.o src/nestegg.c; ar rcs libnestegg.a nestegg.o)

mbew-properties: $(SOURCES) $(STATIC_LIBS) $(EXAMPLE01)
	@gcc -o $(@) $(SOURCES) $(CFLAGS) $(STATIC_LIBS) $(DYNAMIC_LIBS) $(EXAMPLE01)

mbew-iterate: $(SOURCES) $(STATIC_LIBS) $(EXAMPLE02)
	@gcc -o $(@) $(SOURCES) $(CFLAGS) $(STATIC_LIBS) $(DYNAMIC_LIBS) $(EXAMPLE02)

mbew-strings: $(SOURCES) $(STATIC_LIBS) $(EXAMPLE03)
	@gcc -o $(@) $(SOURCES) $(CFLAGS) $(STATIC_LIBS) $(DYNAMIC_LIBS) $(EXAMPLE03)

mbew-video-cairo: $(SOURCES) $(STATIC_LIBS) $(EXAMPLE04)
	@gcc -o $(@) $(SOURCES) $(CFLAGS) $(STATIC_LIBS) $(DYNAMIC_LIBS) $(EXAMPLE04)

clean:
	@rm -f mbew-properties
	@rm -f mbew-video-cairo
	@rm -f output.png

