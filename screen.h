/* See LICENSE for license details. */
#include "fonts.h"
#include "st.h"

// Display orientation
#define ROTATE_0                                    0
#define ROTATE_90                                   1
#define ROTATE_180                                  2
#define ROTATE_270                                  3

// Color inverse. 1 or 0 = set or reset a bit if set a colored pixel
#define IF_INVERT_COLOR                             0

#define COLORED                                     1
#define UNCOLORED                                   0

typedef enum {GOODDISPLAY75, GOODDISPLAY42} screen_type_t;

/*
 * Screen installed on the typewriter.
 *
 * Set to:
 *   - USE_GOODDISPLAY75 for waveshare/good display 7.5" screen.
 *   - USE_GOODDISPLAY42 for waveshare/good display 4.2" screen.
 */
#define USE_GOODDISPLAY42
// #define USE_GOODDISPLAY75

#ifdef USE_GOODDISPLAY75
#include "gooddisplay75_pinout.h"
#endif

#ifdef USE_GOODDISPLAY42
#include "gooddisplay42_pinout.h"
#endif

// Dirty hack. We don't need this 
// struct. TODO clean this.
typedef int GlyphFontSpec;

void run(void);

// E-Ink functions
// Interface-Level primitives.
int init_if(void);
void digital_write(int pin, int value);
int digital_read(int pin);
void delay_ms(unsigned int delayTime);
void spi_transfer(unsigned char data);
// Screen driver functions.
int sinit(void);
int init_waveshare_75(void);
int init_waveshare_42(void);
void sreset(void);
void ssend_command(unsigned char command);
void ssend_data(unsigned char data);
void sdisplay_frame(const unsigned char* frame_buffer);
void sdisplay_frame_75(const unsigned char* frame_buffer);
void sdisplay_frame_42(const unsigned char* frame_buffer);
void sdisplay_frame_fast(const unsigned char* frame_buffer);
void sdisplay_frame_fast_42(const unsigned char* frame_buffer);
void swait_until_idle(void);
void ssleep(void);
void set_lut(void);
void set_lut_42(void);
void set_fast_lut(void);
void set_fast_lut_42(void);
// Framebuffer painting functions.
void pclear(int colored, unsigned char* frame_buffer);
void pdraw_absolute_pixel(int x, int y, int colored, unsigned char* frame_buffer);
void pdraw_pixel(int x, int y, int colored, unsigned char* frame_buffer);
void pdraw_char_at(int x, int y, char ascii_char, sFONT* font, int colored, unsigned char* frame_buffer);
void pdraw_string_at(int x, int y, const char* text, sFONT* font, int colored, unsigned char* frame_buffer);
void pdraw_line(int x0, int y0, int x1, int y1, int colored, unsigned char* frame_buffer);
void pdraw_vertical_line(int x, int y, int height, int colored, unsigned char* frame_buffer);
void pdraw_horizontal_line(int x, int y, int width, int colored, unsigned char* frame_buffer);
void pdraw_rectangle(int x0, int y0, int x1, int y1, int colored, unsigned char* frame_buffer);
void pdraw_filled_rectangle(int x0, int y0, int x1, int y1, int colored, unsigned char* frame_buffer);
void pdraw_term(Line* line, unsigned char* frame_buffer);
// LUTs
extern const unsigned char lut_vcom0_42[];
extern const unsigned char lut_ww_42[];
extern const unsigned char lut_bw_42[];
extern const unsigned char lut_bb_42[];
extern const unsigned char lut_wb_42[];

extern const unsigned char lut_vcom0_fast_42[];
extern const unsigned char lut_ww_fast_42[];
extern const unsigned char lut_bw_fast_42[];
extern const unsigned char lut_bb_fast_42[];
extern const unsigned char lut_wb_fast_42[];

/* config.h globals */
extern int borderpx;
extern float cwscale;
extern float chscale;
extern char termname[];
extern const char *colorname[];
extern size_t colornamelen;
extern unsigned int defaultfg;
extern unsigned int defaultbg;
extern unsigned int defaultcs;
extern unsigned int defaultrcs;
extern unsigned int cursorshape;
extern unsigned int cols;
extern unsigned int screen_type;
extern unsigned int rows;
extern unsigned int defaultattr;
extern size_t mshortcutslen;
extern size_t shortcutslen;
extern uint forceselmod;
extern uint selmasks[];
extern size_t selmaskslen;
extern char ascii_printable[];
