CC := gcc
C_FILES := $(wildcard *.c)
HEADER_FILES := $(wildcard *.h)
OBJ_FILES := $(patsubst %.c,%.o,$(C_FILES))
INCLUDE_DIRS := -I/usr/include/luajit-2.1/
LDFLAGS := -Wl,-rpath,/usr/lib/x86_64-linux-gnu
LIBS := -lluajit-5.1

all: rslb

install: rslb
	cp rslb ..

clean:
	rm -f $(OBJ_FILES)
	rm -f rslb

%.o: %.c $(HEADER_FILES)
	$(CC) -c -Wall -g -std=gnu99 $(INCLUDE_DIRS) $<

rslb: $(OBJ_FILES)
	$(CC) -o $@ $(OBJ_FILES) $(LDFLAGS) $(LIBS)
