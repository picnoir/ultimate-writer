# st - simple terminal
# See LICENSE file for copyright and license details.
.POSIX:

include config.mk

SRC = st.c screen.c main.c font12.c font16.c font20.c font24.c font8.c
OBJ = $(SRC:.c=.o)

all: options writerpi 

options:
	@echo st build options:
	@echo "CFLAGS  = $(STCFLAGS)"
	@echo "LDFLAGS = $(STLDFLAGS)"
	@echo "CC      = $(CC)"

config.h:
	cp config.def.h config.h

.c.o:
	$(CC) $(STCFLAGS) -c $<

st.o: config.h st.h screen.h
screen.o: config.h screen.h
main.o: arg.h st.h screen.h
font.o: font.h

$(OBJ): config.h config.mk

writerpi: $(OBJ)
	$(CC) -o $@ $(OBJ) $(STLDFLAGS)

clean:
	rm -f st $(OBJ) st-$(VERSION).tar.gz

dist: clean
	mkdir -p st-$(VERSION)
	cp -R LICENSE Makefile README config.mk config.def.h st.info st.1 arg.h $(SRC) st-$(VERSION)
	tar -cf - st-$(VERSION) | gzip > st-$(VERSION).tar.gz
	rm -rf st-$(VERSION)

install: st
	cp -f writerpi /usr/bin
	cp -f writerpi.service /lib/systemd/system
	chmod 755 /usr/bin/writerpi

uninstall:
	rm -f /usr/bin/writerpi
	rm -f /lib/systemd/system/writerpi.service

.PHONY: all options clean dist install uninstall
