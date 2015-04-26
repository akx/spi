#include "dot3k.h"
#include <stdio.h>

static DOT3K *dot3k;
static char screen_buf[16 * 3];

void white_screen() {
	dot3k_bl_set_brightness(dot3k, 0, 18, 0);
	dot3k_bl_set_brightness(dot3k, 0, 9, 64);
	dot3k_bl_update_brightnesses(dot3k);
	dot3k_lcd_clear(dot3k);
}	

void open_screen() {
	dot3k = dot3k_init();
	dot3k_bl_open(dot3k);
	dot3k_bl_reset(dot3k);
	dot3k_bl_calibrate(dot3k, 1, 1.6, 1.4, 1);
	dot3k_bl_enable(dot3k, 1);
	dot3k_bl_enable_leds(dot3k, 0xFFFFFF);
	dot3k_lcd_open(dot3k);
	white_screen();
}


void intro_screen() {
	dot3k_lcd_clear(dot3k);
	for(int pos = 0; pos < 3; pos++) {
		dot3k_bl_set_screen_rgb(dot3k, -1, 0, 0, 0);
		for(int shift = 0; shift < 3; shift++) {
			int c = 255 << (shift * 8);
			dot3k_bl_set_screen_rgb(dot3k, pos, c & 0xFF, (c >> 8) & 0xFF, (c >> 16) & 0xFF);
			dot3k_bl_update_brightnesses(dot3k);
			usleep(70000);
		}
	}
}

void write_screen(const char *line1, const char *line2, const char *line3) {
	snprintf(screen_buf, 16 * 3, "%-16s%-16s%-16s",
		(line1 ? line1 : ""),
		(line2 ? line2 : ""),
		(line3 ? line3 : "")
	);
	dot3k_lcd_set_pos(dot3k, 0, 0);
	dot3k_lcd_write_text(dot3k, screen_buf);
}
