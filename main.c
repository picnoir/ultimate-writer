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
#include <pthread.h>
#include "arg.h"

#define Glyph Glyph_
#define Font Font_

#include "screen.h"
#include "st.h"

static inline ushort sixd_to_16bit(int);


/* Globals */
static int frclen = 0;

ushort
sixd_to_16bit(int x)
{
	return x == 0 ? 0 : 0x3737 + 0x2828 * x;
}

void* 
read_stdin(void* arg)
{
  char str[20] = "\0";
  char seq[20] = "\0";
  char inchar;
  for(;;){
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
      if (strcmp(seq, "Esc") == 0) {
        str[0]= 27;
        str[1]= '\0';
      }
      else if (strcmp(seq, "BckSp") == 0){
        strcpy(str, "\177");
      }
      else if (strcmp(seq, "Del") == 0)
        strcpy(str, "\033[3~");
      else if (strcmp(seq, "Up") == 0) 
        strcpy(str, "\033[A");
      else if (strcmp(seq, "Down") == 0) 
        strcpy(str, "\033[B");
      else if (strcmp(seq, "Right") == 0)
        strcpy(str, "\033[C");
      else if (strcmp(seq, "Left") == 0) 
        strcpy(str, "\033[D");
      else if (strcmp(seq, "End") == 0) 
        strcpy(str, "\033[F");
      else if (strcmp(seq, "Home") == 0) 
        strcpy(str, "\033[H");
      else
        read_stdin(arg);
    } else {
      str[0] = inchar;
      str[1] = '\0';
  }
  ttywrite(str, strlen(str), 1);
  }
}

void
run(void)
{
	int w = win.w, h = win.h;
  pthread_t input_thread;
	fd_set rfd;
	ttynew();
  ttyread();
  pthread_create(&input_thread, NULL, read_stdin, NULL);


  /* Init E-Ink screen */
  if (sinit() != 0){
    printf("e-Paper init failed\n");
    return;
  }

  unsigned char* frame_buffer = (unsigned char*)malloc(400 / 8 * 300);
  char in[20] = "\0";
  char* str = (char*)malloc((cols + 10) * sizeof(char));
	for (1;;) {
		FD_ZERO(&rfd);
		FD_SET(cmdfd, &rfd);

		if (FD_ISSET(cmdfd, &rfd)) {
			ttyread();
      // Remove this block to remove console printing.
      // ======================
      /*
      Line* lines = term.line;
      for(int i=0; i < rows; i++) {
        for(int j=0; j < cols; j++) {
          str[j] = lines[i][j].u;
        }
        printf("%s\n", str);
      }
      */
      //=======================
      // Uncomment that to display on e-ink.
      //==================
      pclear(UNCOLORED, frame_buffer);
      pdraw_term(term.line, frame_buffer);
      sdisplay_frame(frame_buffer);
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
