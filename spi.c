#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "midi.h"
#include "screen.h"

uint8_t leds[3] = {0};
uint8_t steps[16] = {0};
unsigned long time_ms = 0;
unsigned long time_ms_start = 0;
unsigned long last_lcd_update = 0;

void update_clock() {
	static struct timespec tp;
	unsigned long t;
	clock_gettime(CLOCK_MONOTONIC_COARSE, &tp);
	t = tp.tv_sec * 1000 + tp.tv_nsec / 1000000;
	if(time_ms_start == 0) time_ms_start = t;
	time_ms = t - time_ms_start;
	
}

void update_lcd() {
	char cc_buf[16], step_buf[17] = {0};
	snprintf(cc_buf, 16, "%02X %02X %02X", leds[0], leds[1], leds[2]);
	for(int i = 0; i < 16; i++) {
		step_buf[i] = (steps[i] ? '#' : '.');
	}
	
	write_screen("Hello, Synaseura", cc_buf, step_buf);
	last_lcd_update = time_ms;	
}

int main(int argc, char **argv)
{
	if(open_midi()) {
		fprintf(stderr, "unable to open midi\n");
		return 1;
	}
	open_screen();
	intro_screen();
	white_screen();
	
	for(;;) {
		update_clock();
		MidiEvent ev;
		if(poll_midi(&ev)) {
			//printf("ev: %d %d %d\n", ev.type, ev.control, ev.value);
			if(ev.type == 1) {
				if(ev.control < 100) {
					leds[(ev.control % 3)] = ev.value;
				} else {				
					uint8_t step = (ev.control - 100) % 8;
					if(ev.value > 100) {
						steps[step] = !steps[step];
						printf("%d = %d\n", step, steps[step]);
					}
				}
			}
			if(ev.type == 2) {
				break;
			}
		}
		for(int n = 0; n < 3; n++) {
			for(int i = 0; i < 8; i++) {
				int cc = 100 + n * 10 + i;
				uint8_t lval = (leds[n] - 8) / 16;
				uint8_t led_state = (i <= lval ? 127 : 0);
				if(n == 0) led_state = steps[i] ? 127 : 0;
				write_midi_cc(cc, led_state);
			}
		}
		if(time_ms - last_lcd_update >= 300) {
			update_lcd();
		}
	}
	white_screen();
	return 0;
}

