CC = gcc
CFLAGS = -Wall
SRCDIR = src

CFLAGS += -Ilibs/include/json-c -Ilibs/include/curl -Ilibs/include/openssl -Llibs/lib
LDFLAGS += -Llibs/lib -l:libjson-c.a -l:libcurl.a -l:libssl.a -l:libcrypto.a -ldl

all: check-versions moodle-curl

check-versions: $(SRCDIR)/check-versions.c
	$(CC) $(CFLAGS) $(SRCDIR)/check-versions.c -o check-versions $(LDFLAGS)

moodle-curl: $(SRCDIR)/moodle-curl.c
	$(CC) $(CFLAGS) $(SRCDIR)/moodle-curl.c -o moodle-curl $(LDFLAGS)

clean:
	$(RM) check-versions moodle-curl