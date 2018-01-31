/* See LICENSE for license details. */
#include <errno.h>
#include <locale.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <time.h>
#include <unistd.h>
#include <libgen.h>
//#include <X11/Xft/Xft.h>
#include "arg.h"

#define Glyph Glyph_
#define Font Font_

#include "screen.h"
#include "st.h"

/* XEMBED messages */
#define XEMBED_FOCUS_IN  4
#define XEMBED_FOCUS_OUT 5

/* macros */
#define TRUERED(x)		(((x) & 0xff0000) >> 8)
#define TRUEGREEN(x)		(((x) & 0xff00))
#define TRUEBLUE(x)		(((x) & 0xff) << 8)

static inline ushort sixd_to_16bit(int);


/* Globals */
static int frclen = 0;

ushort
sixd_to_16bit(int x)
{
	return x == 0 ? 0 : 0x3737 + 0x2828 * x;
}

void
run(void)
{
	int w = win.w, h = win.h;
	fd_set rfd;
	/* Waiting for window mapping */

	ttynew();

  /* Init E-Ink screen */
  if (init_if() != 0){
    printf("e-Paper init failed\n");
    return;
  }

  /* E-ink test */
  unsigned char* frame_buffer = (unsigned char*)malloc(EPD_WIDTH / 8 * EPD_HEIGHT);
  pclear(UNCOLORED, frame_buffer);
  pdraw_string_at(100, 10, "Hello world?", &Font24, UNCOLORED, frame_buffer);
  pdraw_line(20, 80, 180, 280, COLORED, frame_buffer);
  pdraw_filled_rectangle(200, 80, 210, 30, COLORED, frame_buffer);
  sdisplay_frame(frame_buffer);

	for (1;;) {
		FD_ZERO(&rfd);
		FD_SET(cmdfd, &rfd);

		if (FD_ISSET(cmdfd, &rfd)) {
			ttyread();
		}

    // TODO: Display temp ici.
    printf("%c%c%c\n", term.line[0][0].u, term.line[0][1].u, term.line[0][2].u);
	}
}

int
main(int argc, char *argv[])
{
	win.cursor = cursorshape;

	ARGBEGIN {
	case 'a':
		break;
	case 'c':
		opt_class = EARGF(usage());
		break;
	case 'e':
		if (argc > 0)
			--argc, ++argv;
		goto run;
	case 'f':
		opt_font = EARGF(usage());
		break;
	case 'o':
		opt_io = EARGF(usage());
		break;
	case 'l':
		opt_line = EARGF(usage());
		break;
	case 'n':
		opt_name = EARGF(usage());
		break;
	case 't':
	case 'T':
		opt_title = EARGF(usage());
		break;
	case 'w':
		opt_embed = EARGF(usage());
		break;
	case 'v':
		die("%s " VERSION " (c) 2010-2016 st engineers\n", argv0);
		break;
	default:
		usage();
	} ARGEND;

run:
	if (argc > 0) {
		/* eat all remaining arguments */
		opt_cmd = argv;
		if (!opt_title && !opt_line)
			opt_title = basename(xstrdup(argv[0]));
	}
	setlocale(LC_CTYPE, "");
	tnew(MAX(cols, 1), MAX(rows, 1));
	run();

	return 0;
}
