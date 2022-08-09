/* See LICENSE for license details. */
#include "fonts.h"
#include "st.h"

// Pin level definition
#define LOW                                         0
#define HIGH                                        1

// Pin definition
#define RST_PIN                                     17
#define DC_PIN                                      25
#define CS_PIN                                      8
#define BUSY_PIN                                    24

// Display orientation
#define ROTATE_0                                    0
#define ROTATE_90                                   1
#define ROTATE_180                                  2
#define ROTATE_270                                  3

// Color inverse. 1 or 0 = set or reset a bit if set a colored pixel
#define IF_INVERT_COLOR                             0

#define COLORED                                     1
#define UNCOLORED                                   0

// See makefile to configure
#ifdef SCREEN_75_V1
#include "screen_75_v1.h"
#endif

#ifdef SCREEN_75_V2
#include "screen_75_v2.h"
#endif

// Dirty hack. We don't need this
// struct. TODO clean this.
// typedef int GlyphFontSpec;

// E-Ink functions
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
#endif
