#include <pebble.h>

#define SCREENW 144
#define SCREENH 168
#define CX 72
#define CY 84
#define NUM_COLS 9
#define NUM_ROWS 7

#define NUM_BMP 7
const int bmpId[NUM_BMP] = {
	RESOURCE_ID_IMAGE_B1, RESOURCE_ID_IMAGE_B2, RESOURCE_ID_IMAGE_B3,
	RESOURCE_ID_IMAGE_B4, RESOURCE_ID_IMAGE_B5, RESOURCE_ID_IMAGE_B6,
	RESOURCE_ID_IMAGE_B7
};

#define MAX_NEW_CELLS 3

enum {
	CONFIG_KEY_STRAIGHT = 0,
	CONFIG_KEY_SEMICOLON = 1
};

Window *window;
bool clock12;

typedef struct {
	TextLayer *textLayer;
	BitmapLayer *bitmapLayer;
	int step;
} MatrixCell;

MatrixCell cells[NUM_ROWS*NUM_COLS];
int dx, dy, cellWidth, cellHeight;
GFont matrixFont, digitFont;
int32_t rand_seed = 3;
time_t now;
struct tm last = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, "" };
GBitmap *bmp[NUM_BMP];
int M_ROWS = NUM_ROWS/2;
int M_COLS = NUM_COLS/2;
GRect bmpRect;
char *glyphs[] = {
	"a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m",
	"n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z"
};
char *uppers[] = {
	"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M",
	"N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z"
};
char *digits[] = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9" };
char space[] = " ";
char semicolon[] = ":";

int straight_digits = 0;
int blinking_semicolon = 0;

static inline void set_random_seed(int32_t seed) {
	rand_seed = (seed & 32767);
}

// Returns pseudo-random integer in the range [0,max[
static int random(int max) {
	rand_seed = (((rand_seed * 214013L + 2531011L) >> 16) & 32767);
	return (rand_seed%max);
}

static inline void setCellEmpty(int r, int c) {
	MatrixCell *cell = &(cells[r*NUM_COLS+c]);
	text_layer_set_text(cell->textLayer, space);
}

static inline void setCellSemiColon(int r, int c) {
	MatrixCell *cell = &(cells[r*NUM_COLS+c]);
	text_layer_set_text(cell->textLayer, semicolon);
}

static inline void setCellDigit(int r, int c, int n) {
	MatrixCell *cell = &(cells[r*NUM_COLS+c]);
	text_layer_set_text(cell->textLayer, digits[n]);
}

static inline void setCellGlyph(int r, int c, int n) {
	MatrixCell *cell = &(cells[r*NUM_COLS+c]);
	text_layer_set_text(cell->textLayer, glyphs[n]);
}

static inline void setCellChar(int r, int c, char t, int s) {
	MatrixCell *cell = &(cells[r*NUM_COLS+c]);
	if (t == ' ') {
		text_layer_set_text(cell->textLayer, space);
	} else {
		text_layer_set_text(cell->textLayer, uppers[t - 'A']);
	}
	cell->step = s;
}

static inline void setCellBitmap(int r, int c, GBitmap *bmp) {
	bitmap_layer_set_bitmap(cells[r*NUM_COLS+c].bitmapLayer, bmp);
}

static void setHour(struct tm *now) {
	int h = now->tm_hour;

	if (clock12) {
		h = h%12;
		if (h == 0) h = 12;
	}
	setCellDigit(M_ROWS, M_COLS-2, h/10);
	setCellDigit(M_ROWS, M_COLS-1, h%10);
	setCellDigit(M_ROWS, M_COLS+1, now->tm_min/10);
	setCellDigit(M_ROWS, M_COLS+2, now->tm_min%10);
}

void handle_tick(struct tm *now, TimeUnits units_changed) {
	int r, c, i, max;
	MatrixCell *cell;
		
	for (r=0; r<NUM_ROWS; r++) {
		for (c=0; c<NUM_COLS; c++) {
			cell = &(cells[r*NUM_COLS+c]);
			if (cell->step > 0) {
				cell->step = cell->step - 1;
				if (cell->step == 0) {
					setCellBitmap(r, c, NULL);
					setCellEmpty(r, c);
				} else {
					if (cell->step <= NUM_BMP) setCellBitmap(r, c, bmp[cell->step-1]);
				}
			}
		}
	}
	
	max = 1+random(MAX_NEW_CELLS);
	for (i=0; i<max; i++) {
		r = random(NUM_ROWS);
		c = random(NUM_COLS);
		
		if (cells[r*NUM_COLS+c].step == 0) {
			cells[r*NUM_COLS+c].step = NUM_BMP+1;
			setCellGlyph(r, c, random(26));
			setCellBitmap(r, c, NULL);
		}
	}

	if (now->tm_min != last.tm_min) {
		setHour(now);
	}
	
	if (blinking_semicolon) {
		if (now->tm_sec%2) {
			setCellEmpty(M_ROWS, M_COLS);
		} else {
			setCellSemiColon(M_ROWS, M_COLS);
		}
	}
	last = *now;
}

void readConfig() {
	if (persist_exists(CONFIG_KEY_STRAIGHT)) {
		straight_digits = persist_read_int(CONFIG_KEY_STRAIGHT);
	} else {
		straight_digits = 0;
	}

	if (persist_exists(CONFIG_KEY_SEMICOLON)) {
		blinking_semicolon = persist_read_int(CONFIG_KEY_SEMICOLON);
	} else {
		blinking_semicolon = 0;
	}
}

void applyConfig() {
	int x;
	MatrixCell *cell;
	for (x=M_COLS-2; x<=M_COLS+2; x++) {
		if (x != M_COLS) {
			cell = &(cells[M_ROWS*NUM_COLS+x]);
			if (straight_digits) {
				text_layer_set_font(cell->textLayer, digitFont);
			} else {
				text_layer_set_font(cell->textLayer, matrixFont);
			}
		}
	}
	
	if (!blinking_semicolon) {
		setCellEmpty(M_ROWS, M_COLS);
	}
}

void initCell(int col, int row) {
	GRect rect = GRect(dx + col * cellWidth, dy + row * cellHeight, cellWidth, cellHeight);
	MatrixCell *cell = &(cells[row*NUM_COLS+col]);
	Layer *rootLayer = window_get_root_layer(window);
	
	cell->step = 0;
	
	cell->textLayer = text_layer_create(rect);
	text_layer_set_font(cell->textLayer, matrixFont);
	text_layer_set_background_color(cell->textLayer, GColorBlack);
	text_layer_set_text_color(cell->textLayer, GColorWhite);
	text_layer_set_text_alignment(cell->textLayer, GTextAlignmentCenter);
	layer_add_child(rootLayer, text_layer_get_layer(cell->textLayer));
	
	setCellEmpty(row, col);
	
	cell->bitmapLayer = bitmap_layer_create(rect);
	bitmap_layer_set_compositing_mode(cell->bitmapLayer, GCompOpClear);
	layer_add_child(rootLayer, bitmap_layer_get_layer(cell->bitmapLayer));
}

void deinitCell(int col, int row) {
	MatrixCell *cell = &(cells[row*NUM_COLS+col]);
	bitmap_layer_destroy(cell->bitmapLayer);
	text_layer_destroy(cell->textLayer);
}

void in_dropped_handler(AppMessageResult reason, void *context) {
}

void in_received_handler(DictionaryIterator *received, void *context) {
	Tuple *straight = dict_find(received, CONFIG_KEY_STRAIGHT);
	Tuple *semicolon = dict_find(received, CONFIG_KEY_SEMICOLON);
	
	if (straight && semicolon) {
		persist_write_int(CONFIG_KEY_STRAIGHT, straight->value->int32);
		persist_write_int(CONFIG_KEY_SEMICOLON, semicolon->value->int32);
		
		straight_digits = straight->value->int32;
		blinking_semicolon = semicolon->value->int32;
		
		applyConfig();
	}
}

static void app_message_init(void) {
	app_message_register_inbox_received(in_received_handler);
	app_message_register_inbox_dropped(in_dropped_handler);
	app_message_open(64, 64);
}

void handle_init() {
	int x, y;
	MatrixCell *cell;
	
	window = window_create();
	window_stack_push(window, true);
    window_set_background_color(window, GColorBlack);
    
	app_message_init();
	
	readConfig();
	
	matrixFont = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_MATRIX_23));
	digitFont = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_SOURCECODE_23));
	
	cellWidth = SCREENW / NUM_COLS;
	cellHeight = SCREENH / NUM_ROWS;
	dx = (SCREENW - NUM_COLS*cellWidth)/2;
	dy = (SCREENH - NUM_ROWS*cellHeight)/2;
	bmpRect = GRect(0, 0, cellWidth, cellHeight);
	
	now = time(NULL);
	set_random_seed(now);
	
	clock12 = !clock_is_24h_style();

	for (x=0; x<NUM_BMP; x++) {
		bmp[x] = gbitmap_create_with_resource(bmpId[x]);
	}

	for (y=0; y<NUM_ROWS; y++) {
		for (x=0; x<NUM_COLS; x++) {
			initCell(x, y);
		}
	}
	
	// Reserve cells for displaying hour
	for (x=M_COLS-2; x<=M_COLS+2; x++) {
		cell = &(cells[M_ROWS*NUM_COLS+x]);
		cell->step = -1;
		if (straight_digits) {
			text_layer_set_font(cell->textLayer, digitFont);
		}
	}
	// Set Font for the semicolon as the MatrixFont doesn't have it
	text_layer_set_font(cells[M_ROWS*NUM_COLS+M_COLS].textLayer, digitFont);
	
	// Start with a nice Splash Screen
	setCellChar(0, 0, 'M', NUM_BMP+1);
	setCellChar(0, 1, 'A', NUM_BMP+2);
	setCellChar(0, 2, 'T', NUM_BMP+3);
	setCellChar(0, 3, 'R', NUM_BMP+4);
	setCellChar(0, 4, 'I', NUM_BMP+5);
	setCellChar(0, 5, 'X', NUM_BMP+4);
	setCellChar(0, 6, ' ', NUM_BMP+3);
	setCellChar(0, 7, ' ', NUM_BMP+2);
	setCellChar(0, 8, ' ', NUM_BMP+1);
	
	setCellChar(1, 0, ' ', NUM_BMP+1);
	setCellChar(1, 1, ' ', NUM_BMP+2);
	setCellChar(1, 2, ' ', NUM_BMP+3);
	setCellChar(1, 3, ' ', NUM_BMP+4);
	setCellChar(1, 4, ' ', NUM_BMP+5);
	setCellChar(1, 5, 'C', NUM_BMP+4);
	setCellChar(1, 6, 'O', NUM_BMP+3);
	setCellChar(1, 7, 'D', NUM_BMP+2);
	setCellChar(1, 8, 'E', NUM_BMP+1);

	setCellChar(6, 0, ' ', NUM_BMP+1);
	setCellChar(6, 1, 'B', NUM_BMP+2);
	setCellChar(6, 2, 'Y', NUM_BMP+3);
	setCellChar(6, 3, ' ', NUM_BMP+4);
	setCellChar(6, 4, ' ', NUM_BMP+5);
	setCellChar(6, 5, 'J', NUM_BMP+4);
	setCellChar(6, 6, 'N', NUM_BMP+3);
	setCellChar(6, 7, 'M', NUM_BMP+2);
	setCellChar(6, 8, ' ', NUM_BMP+1);

	setHour(localtime(&now));
	
	tick_timer_service_subscribe(SECOND_UNIT, handle_tick);
}

void handle_deinit() {
	int i, x, y;
	
	tick_timer_service_unsubscribe();

	fonts_unload_custom_font(matrixFont);
	fonts_unload_custom_font(digitFont);
	
	for (y=0; y<NUM_ROWS; y++) {
		for (x=0; x<NUM_COLS; x++) {
			deinitCell(x, y);
		}
	}

	for (i=0; i<NUM_BMP; i++) {
		gbitmap_destroy(bmp[i]);
	}
	
	window_destroy(window);
}

int main(void) {
	handle_init();
	app_event_loop();
	handle_deinit();
}
