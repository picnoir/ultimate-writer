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
	rm -f st $(OBJ)

install: writerpi
	cp -f writerpi $(PREFIX)/bin
	cp -f start-writerpi $(PREFIX)/bin
	cp -f stop-writerpi $(PREFIX)/bin
	cp -f writerpi.service $(SYSTEMD_PREFIX)
	chmod 755 $(PREFIX)/bin/writerpi
	chmod 755 $(PREFIX)/bin/start-writerpi
	chmod 755 $(PREFIX)/bin/stop-writerpi

uninstall:
	rm -f $(PREFIX)/bin/writerpi
	rm -f $(PREFIX)/bin/start-writerpi
	rm -f $(PREFIX)/bin/stop-writerpi
	rm -f $(SYSTEMD_PREFIX)/writerpi.service

.PHONY: all options clean dist install uninstall
