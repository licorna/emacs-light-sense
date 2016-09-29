# emacs source path
ROOT    = /Users/rodrigo.valin/Personal/opensource/emacs
CC      = clang

all: lightsense.so

# make shared library out of the object file
%.so:
	$(CC) -shared -undefined dynamic_lookup -framework IOKit -framework CoreFoundation -I$(ROOT)/src -o lightsense.so lightsense.c
