# See LICENSE file for copyright and license details.
.POSIX:

include config.mk

SRC_COMMON = st.c screen.c main.c font12.c font16.c font20.c font24.c font8.c
ifeq ($(SCREEN),75_V1)
SRC = $(SRC_COMMON) screen_75_v1.c
endif
ifeq ($(SCREEN),75_V2)
SRC = $(SRC_COMMON) screen_75_v2.c
endif
OBJ = $(SRC:.c=.o)

all: options ultimatewriter

options:
	@echo st build options:
	@echo "CFLAGS  = $(STCFLAGS)"
	@echo "LDFLAGS = $(STLDFLAGS)"
	@echo "CC      = $(CC)"

.c.o:
	$(CC) $(STCFLAGS) -DSCREEN_$(SCREEN) -c $<

st.o: config.h st.h screen.h
screen.o: config.h screen.h
main.o: arg.h st.h screen.h
font.o: font.h

$(OBJ): config.h config.mk

ultimatewriter: $(OBJ)
	$(CC) -o $@ $(OBJ) $(STLDFLAGS) -DSCREEN_$(SCREEN)

clean:
	rm -f st $(OBJ)

install: ultimatewriter
	cp -f ultimatewriter $(PREFIX)/bin
	cp -f scripts/start-ultimatewriter $(PREFIX)/bin
	cp -f scripts/stop-ultimatewriter $(PREFIX)/bin
	cp -f systemd/ultimatewriter.service $(SYSTEMD_PREFIX)
	chmod 755 $(PREFIX)/bin/ultimatewriter
	chmod 755 $(PREFIX)/bin/start-ultimatewriter
	chmod 755 $(PREFIX)/bin/stop-ultimatewriter

uninstall:
	rm -f $(PREFIX)/bin/ultimatewriter
	rm -f $(PREFIX)/bin/start-ultimatewriter
	rm -f $(PREFIX)/bin/stop-ultimatewriter
	rm -f $(SYSTEMD_PREFIX)/ultimatewriter.service

.PHONY: all options clean dist install uninstall
