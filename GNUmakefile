# Common
version= $(shell cat version)
build_id= $(shell [ -d .git ] && git describe --tags --dirty)

prefix= /usr/local
libdir= $(prefix)/lib
incdir= $(prefix)/include

CC=   clang

CFLAGS+= -std=c99
CFLAGS+= -Wall -Wextra -Werror -Wsign-conversion
CFLAGS+= -Wno-unused-parameter -Wno-unused-function

CFLAGS+= -DBF_VERSION=\"$(version)\"
CFLAGS+= -DBF_BUILD_ID=\"$(build_id)\"

LDFLAGS=

PANDOC_OPTS= -s --toc --email-obfuscation=none

# Platform specific
platform= $(shell uname -s)

ifeq ($(platform), Linux)
	CFLAGS+= -DBF_PLATFORM_LINUX
	CFLAGS+= -D_POSIX_C_SOURCE=200809L -D_BSD_SOURCE
endif

# Debug
debug=0
ifeq ($(debug), 1)
	CFLAGS+= -g -ggdb
else
	CFLAGS+= -O2
endif

# Coverage
coverage?= 0
ifeq ($(coverage), 1)
	CC= gcc
	CFLAGS+= -fprofile-arcs -ftest-coverage
	LDFLAGS+= --coverage
endif

# Target: libbuffer
libbbuffer_LIB= libbuffer.a
libbbuffer_SRC= $(wildcard src/*.c)
libbbuffer_INC= src/buffer.h
libbbuffer_OBJ= $(subst .c,.o,$(libbbuffer_SRC))

$(libbbuffer_LIB): CFLAGS+=

# Target: tests
tests_SRC= $(wildcard tests/*.c)
tests_OBJ= $(subst .c,.o,$(tests_SRC))
tests_BIN= $(subst .o,,$(tests_OBJ))

$(tests_BIN): CFLAGS+= -Isrc
$(tests_BIN): LDFLAGS+= -L.
$(tests_BIN): LDLIBS+= -lbuffer -lutest

# Target: doc
doc_SRC= $(wildcard doc/*.mkd)
doc_HTML= $(subst .mkd,.html,$(doc_SRC))

# Rules
all: lib tests doc

lib: $(libbbuffer_LIB)

tests: lib $(tests_BIN)

doc: $(doc_HTML)

$(libbbuffer_LIB): $(libbbuffer_OBJ)
	$(AR) cr $@ $(libbbuffer_OBJ)

tests/%: tests/%.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

doc/%.html: doc/*.mkd
	pandoc $(PANDOC_OPTS) -t html5 -o $@ $<

clean:
	$(RM) $(libbbuffer_LIB) $(wildcard src/*.o)
	$(RM) $(tests_BIN) $(wildcard tests/*.o)
	$(RM) $(wildcard **/*.gc??)
	$(RM) -r coverage
	$(RM) -r $(doc_HTML)

coverage:
	lcov -o /tmp/libbbuffer.info -c -d . -b .
	genhtml -o coverage -t libbbuffer /tmp/libbbuffer.info
	rm /tmp/libbbuffer.info

install: lib
	mkdir -p $(libdir) $(incdir)
	install -m 644 $(libbbuffer_LIB) $(libdir)
	install -m 644 $(libbbuffer_INC) $(incdir)

uninstall:
	$(RM) $(addprefix $(libdir)/,$(libbbuffer_LIB))
	$(RM) $(addprefix $(incdir)/,$(libbbuffer_INC))

tags:
	ctags -o .tags -a $(wildcard src/*.[hc])

.PHONY: all lib tests doc clean coverage install uninstall tags
