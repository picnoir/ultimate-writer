/* See LICENSE file for copyright and license details. */

/*
 * appearance
 *
 * font: see http://freedesktop.org/software/fontconfig/fontconfig-user.html
 */
int borderpx = 2;

/*
 * What program is execed by st depends of these precedence rules:
 * 1: program passed with -e
 * 2: utmp option
 * 3: SHELL environment variable
 * 4: value of shell in /etc/passwd
 * 5: value of shell in config.h
 */
static char shell[] = "/bin/sh";
static char *utmp = NULL;
static char stty_args[] = "stty raw pass8 nl -echo -iexten -cstopb 38400";

/* identification sequence returned in DA and DECID */
static char vtiden[] = "\033[?6c";

/* Kerning / character bounding-box multipliers */
float cwscale = 1.0;
float chscale = 1.0;

/*
 * word delimiter string
 *
 * More advanced example: " `'\"()[]{}"
 */
static char worddelimiters[] = " ";

/*
 * bell volume. It must be a value between -100 and 100. Use 0 for disabling
 * it
 */
static int bellvolume = 0;

/* default TERM value */
char termname[] = "st-256color";

/*
 * spaces per tab
 *
 * When you are changing this value, don't forget to adapt the »it« value in
 * the st.info and appropriately install the st.info in the environment where
 * you use this st version.
 *
 *	it#$tabspaces,
 *
 * Secondly make sure your kernel is not expanding tabs. When running `stty
 * -a` »tab0« should appear. You can tell the terminal to not expand tabs by
 *  running following command:
 *
 *	stty tabs
 */
static unsigned int tabspaces = 8;

/* Terminal colors (16 first used in escape sequence) */
const char *colorname[] = {
	/* 8 normal colors */
	"black",
	"red3",
	"green3",
	"yellow3",
	"blue2",
	"magenta3",
	"cyan3",
	"gray90",

	/* 8 bright colors */
	"gray50",
	"red",
	"green",
	"yellow",
	"#5c5cff",
	"magenta",
	"cyan",
	"white",

	[255] = 0,

	/* more colors can be added after 255 to use with DefaultXX */
	"#cccccc",
	"#555555",
};


/*
 * Default colors (colorname index)
 * foreground, background, cursor, reverse cursor
 */
unsigned int defaultfg = 7;
unsigned int defaultbg = 0;
unsigned int defaultcs = 256;
unsigned int defaultrcs = 257;

/*
 * Default shape of cursor
 * 2: Block ("█")
 * 4: Underline ("_")
 * 6: Bar ("|")
 * 7: Snowman ("☃")
 */
unsigned int cursorshape = 2;

/*
 * Default columns and rows numbers
 */

unsigned int cols = 58;
unsigned int rows = 24;

/*
 * Color used to display font attributes when fontconfig selected a font which
 * doesn't match the ones requested.
 */
unsigned int defaultattr = 11;

/*
 * Screen installed on the typewriter.
 *
 * GOODDISPLAY75 for waveshare/good display 7.5" screen.
 * GOODDISPLAY42 for waveshare/good display 4.2" screen.
 */
unsigned int screen_type = GOODDISPLAY75;

/*
 * Printable characters in ASCII, used to estimate the advance width
 * of single wide characters.
 */
char ascii_printable[] =
	" !\"#$%&'()*+,-./0123456789:;<=>?"
	"@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
	"`abcdefghijklmnopqrstuvwxyz{|}~";

