/* See LICENSE for license details. */
#include <errno.h>
#include <locale.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <time.h>
#include <unistd.h>
#include <libgen.h>
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
wait_stdin_char(char* buff, int* len)
{
  char inchar;
  char seq[20] = "\0";
  *len = 1;
  inchar = getchar();
  if(inchar == '<') {
      /* This is a special character. It looks like <SpeCas>.
       * 
       * 1- Parse SpeCas.
       * 2- Send the appropriate ascii sequence.
       *
       *
       * Use od -c to find linux escape sequences.
       */
      int i = 0;
      while(inchar != '>'){
        inchar = getchar();
        seq[i] = inchar;
        i++;
      }
      seq[i-1] = '\0';
      if (strcmp(seq, "Esc") == 0)
        buff[0] = 27;
      else if (strcmp(seq, "BckSp") == 0){
        buff[0] = '\b';
        buff[1] = ' ';
        buff[2] = '\b';
        *len = 3;
      }
      else if (strcmp(seq, "Del") == 0)
        buff[0] = 127;
      else if (strcmp(seq, "Up") == 0) {
        buff[0] = 33; //ESC
        buff[1] = 0;
        buff[2] = 'A';
        *len = 3;
      }
      else if (strcmp(seq, "Down") == 0) {
        buff[0] = 27; //ESC
        buff[1] = '[';
        buff[2] = 'B';
        *len = 3;
      }
      else if (strcmp(seq, "Right") == 0) {
        buff[0] = 27; //ESC
        buff[1] = '[';
        buff[2] = 'C';
        *len = 3;
      }
      else if (strcmp(seq, "Left") == 0) {
        buff[0] = 27; //ESC
        buff[1] = '[';
        buff[2] = 'D';
        *len = 3;
      }
      else
        wait_stdin_char(buff, len);
  } else {
    buff[0] = inchar;
  }
}

void
run(void)
{
	int w = win.w, h = win.h;
	fd_set rfd;
	/* Waiting for window mapping */

	ttynew();

  /* Init E-Ink screen */
//  if (init_if() != 0){
//    printf("e-Paper init failed\n");
//    return;
//  }

  unsigned char* frame_buffer = (unsigned char*)malloc(EPD_WIDTH / 8 * EPD_HEIGHT);
  char in[20] = "\0";
  char* str = (char*)malloc((cols + 10) * sizeof(char));
  int clen = 0;
	for (1;;) {
		FD_ZERO(&rfd);
		FD_SET(cmdfd, &rfd);

		if (FD_ISSET(cmdfd, &rfd)) {
			ttyread();
      //TODO: remove this block to remove console printing.
      // ======================
      Line* lines = term.line;
      for(int i=0; i < rows; i++) {
        for(int j=0; j < cols; j++) {
          str[j] = lines[i][j].u;
        }
        printf("%s\n", str);
      }
      //=======================
      wait_stdin_char(in, &clen);
      ttywrite(in, clen);
      //TODO: uncomment that to display on e-ink.
      //==================
/*    pclear(UNCOLORED, frame_buffer);
      pdraw_term(term.line, frame_buffer);
      sdisplay_frame(frame_buffer);*/
      //==================
		}
	}
  free(str);
  free(frame_buffer);
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
