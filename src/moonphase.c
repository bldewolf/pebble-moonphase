/*
Copyright (C) 2013 Brian De Wolf

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"

#include "unix.h"


#define MY_UUID { 0xE2, 0x4C, 0xD6, 0x99, 0x59, 0xD4, 0x49, 0x22, 0xBD, 0xA5, 0x43, 0x07, 0x17, 0xFF, 0xA3, 0xB8 }
PBL_APP_INFO(MY_UUID,
		"Moon Phases", "Brian De Wolf",
		1, 0, /* App version */
		RESOURCE_ID_IMAGE_MENU_ICON,
		APP_INFO_STANDARD_APP);
//		APP_INFO_WATCH_FACE);

// LOSTPIXELS handles the lost pixels for the pebble header when running as an
// app
#define LOSTPIXELS 16
#define HEIGHT (168 - LOSTPIXELS)
#define WIDTH 144
#define TEXTH 16

Window window;
TextLayer phase_text;
TextLayer fudge_text;
Layer moon_layer; // actually just the shadow, now...
BmpContainer the_moon;

int phase = -1;
int day_fudge = 0;
char fudge_str[] = "   ";

void moon_LayerUpdateProc(struct Layer *layer, GContext *ctx) {
/*
	GPath shadow;
	GPathInfo shadow_points = {
		5,
		(GPoint []) {
			{0, 0},
			{0, 0},
			{0, 0},
			{0, 0},
			{0, 0},
		}
	};
*/
	int radius;
	int x, y, error;
	int mx, my;
	GPoint center;
	int edge;

	if(phase == 1) {
		graphics_context_set_fill_color(ctx, GColorBlack);
		graphics_fill_rect(ctx, GRect(0, 0, layer->frame.size.w, layer->frame.size.h), 0, GCornerNone);
		return;
	}
	if(phase == 15)
		return;
	graphics_context_set_fill_color(ctx, GColorWhite);
	center.x = layer->frame.size.w / 2;
	center.y = layer->frame.size.h / 2;
	radius = (layer->frame.size.w > layer->frame.size.h ?
		layer->frame.size.h / 2 :
		layer->frame.size.w / 2) - 4;
//	graphics_fill_circle(ctx, center, radius);

	graphics_context_set_stroke_color(ctx, GColorBlack);
	// circle algo from: http://en.wikipedia.org/wiki/Midpoint_circle_algorithm
	// Even though I studied it in school, it still feels like magic
	error = -radius;
	x = radius;
	y = 0;
	while ( x >= y) {
		if(phase < 15) {
			mx = x * (cos_lookup(TRIG_MAX_ANGLE * (phase / 28.0)) / (0xffff * 1.0));
			my = y * (cos_lookup(TRIG_MAX_ANGLE * (phase / 28.0)) / (0xffff * 1.0));
			edge = 0;
		}
		if(phase > 15) {
			mx = x * (cos_lookup(TRIG_MAX_ANGLE * ((phase - 15) / 32.0)) / (0xffff * 1.0));
			my = y * (cos_lookup(TRIG_MAX_ANGLE * ((phase - 15) / 32.0)) / (0xffff * 1.0));
			edge = layer->frame.size.w;
		}
		graphics_draw_line(ctx, GPoint(center.x + mx, center.y + y), GPoint(edge, center.y + y));
		graphics_draw_line(ctx, GPoint(center.x + mx, center.y - y), GPoint(edge, center.y - y));
		graphics_draw_line(ctx, GPoint(center.x + my, center.y + x), GPoint(edge, center.y + x));
		graphics_draw_line(ctx, GPoint(center.x + my, center.y - x), GPoint(edge, center.y - x));

		error += y;
		++y;
		error += y;
		if( error >= 0) {
			error -= x;
			--x;
			error -= x;
		}
	}
}

// This is the simple method from http://www.ben-daglish.net/moon.shtml
// It's probably also the worst.  Oh well
//
// It counts the number of seconds since Jan 7 20:35 1970, a known New Moon.
// Using the number of seconds in a lunar moon cycle (lp, we calculate what day
// we are in in this current moon month
int current_phase() {
	PblTm tm;
	int current;
	int past;
	int lp = 2551443;
	int phase;

	get_time(&tm);
	tm.tm_sec = 35;
	tm.tm_min = 20;
	tm.tm_hour = 7;
	tm.tm_yday += day_fudge;
	current = get_epoch_seconds(&tm);
	tm.tm_yday = 7;
	tm.tm_year = 70;
	past = get_epoch_seconds(&tm);
	phase = (current - past) % lp;
	return (phase / (24*3600)) + 1;
}

void update_text() {
	// This switch statement was the most amusing solution to this problem
	switch(phase) {
		case 1:
			text_layer_set_text(&phase_text, "NEW MOON");
			break;
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
			text_layer_set_text(&phase_text, "WAXING CRESCENT");
			break;
		case 7:
			text_layer_set_text(&phase_text, "FIRST QUARTER");
			break;
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
			text_layer_set_text(&phase_text, "WAXING GIBBOUS");
			break;
		case 15:
			text_layer_set_text(&phase_text, "FULL MOON");
			break;
		case 16:
		case 17:
		case 18:
		case 19:
		case 20:
		case 21:
		case 22:
			text_layer_set_text(&phase_text, "WANING GIBBOUS");
			break;
		case 23:
			text_layer_set_text(&phase_text, "LAST QUARTER");
			break;
		case 24:
		case 25:
		case 26:
		case 27:
		case 28:
		case 29:
		case 30:
			text_layer_set_text(&phase_text, "WANING CRESCENT");
			break;
		default:
			text_layer_set_text(&phase_text, "I BROKE");
			break;
	}
}

void update_phase() {
	int cph = phase;
	phase = current_phase();
	if(phase == cph)
		return;
	update_text();
	layer_mark_dirty(&moon_layer);
}

void update_fudge_str() {
	int x = abs(day_fudge);
	char *c = fudge_str + strlen(fudge_str) - 1;
	while(x > 0) {
		c[0] = '0' + (x % 10);
		c--;
		x = x / 10;
	}

	if(day_fudge != 0) {
		c[0] = day_fudge > 0 ? '+' : '-';
		c--;
	}

	while(c >= fudge_str) {
		c[0] = ' ';
		c--;
	}

	text_layer_set_text(&fudge_text, fudge_str);
}

void debug_forward(ClickRecognizerRef recognizer, Window *window) {
	(void)recognizer;
	(void)window;

	if(day_fudge == 99)
		return;

	day_fudge++;
	update_fudge_str();
	update_phase();
}
void debug_reset(ClickRecognizerRef recognizer, Window *window) {
	(void)recognizer;
	(void)window;
	
	day_fudge = 0;
	update_fudge_str();
	update_phase();
}

void debug_back(ClickRecognizerRef recognizer, Window *window) {
	(void)recognizer;
	(void)window;

	if(day_fudge == -99)
		return;

	day_fudge--;
	update_fudge_str();
	update_phase();
}


void click_config_provider(ClickConfig **config, Window *window) {
	(void)window;

	config[BUTTON_ID_UP]->click.handler = (ClickHandler) debug_forward;
	config[BUTTON_ID_SELECT]->click.handler = (ClickHandler) debug_reset;
	config[BUTTON_ID_DOWN]->click.handler = (ClickHandler) debug_back;
}



void handle_init(AppContextRef ctx) {
	(void)ctx;

	window_init(&window, "Moon phases");
	window_stack_push(&window, true /* Animated */);
	window_set_background_color(&window, GColorBlack);

	resource_init_current_app(&SOMETHINGSIGNIFICANT);

	bmp_init_container(RESOURCE_ID_THE_MOON, &the_moon);
	layer_set_frame(&the_moon.layer.layer, GRect(0, 0, WIDTH, HEIGHT - TEXTH));
	bitmap_layer_set_alignment(&the_moon.layer, GAlignCenter);

	layer_add_child(&window.layer, &the_moon.layer.layer);

	layer_init(&moon_layer, window.layer.frame);
	layer_set_frame(&moon_layer, GRect(0, 0, WIDTH, HEIGHT - TEXTH));
	moon_layer.update_proc = moon_LayerUpdateProc; // MAGIC!
	layer_add_child(&window.layer, &moon_layer);

	text_layer_init(&phase_text, window.layer.frame);
	text_layer_set_text_color(&phase_text, GColorBlack);
	text_layer_set_background_color(&phase_text, GColorWhite);
	text_layer_set_text_alignment(&phase_text, GTextAlignmentCenter);
	layer_set_frame(&phase_text.layer, GRect(0, HEIGHT - TEXTH, WIDTH, TEXTH));
	layer_add_child(&window.layer, &phase_text.layer);

	text_layer_init(&fudge_text, window.layer.frame);
	text_layer_set_text_color(&fudge_text, GColorBlack);
	text_layer_set_background_color(&fudge_text, GColorClear);
	text_layer_set_text_alignment(&fudge_text, GTextAlignmentRight);
	layer_set_frame(&fudge_text.layer, GRect(0, HEIGHT - TEXTH, WIDTH, TEXTH));
	layer_add_child(&window.layer, &fudge_text.layer);

	window_set_click_config_provider(&window, (ClickConfigProvider) click_config_provider);

	update_phase();
}

void handle_deinit(AppContextRef ctx) {
	(void)ctx;

	bmp_deinit_container(&the_moon);
}


void pbl_main(void *params) {
	PebbleAppHandlers handlers = {
		.init_handler = &handle_init,
		.deinit_handler = &handle_deinit,
	};
	app_event_loop(params, &handlers);
}
