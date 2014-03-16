#include <pebble.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "qrencode.h"

#define MAX_DATA_SIZE (7090 * 16) /* from the specification */
#define MAX_CODES 4
static int version = 0;
static QRecLevel level = QR_ECLEVEL_L;

static Window *window=NULL;
static TextLayer *text_layer=NULL;
QRcode *code;
GRect bounds;
Layer * window_layer=NULL;
int index=0;


enum SettingsKeys {
		QRCONTENT1_KEY=0x0,
		QRCONTENT2_KEY=0x1,
		QRCONTENT3_KEY=0x2,
		QRCONTENT4_KEY=0x3,
		QRDESC1_KEY=0x4,
		QRDESC2_KEY=0x5,
		QRDESC3_KEY=0x6,
		QRDESC4_KEY=0x7,
};


#define	v1	17
#define	v2	32
#define	v3	53
#define	v4	78
#define	v5	106
#define	v6	134
#define	v7	154
#define	v8	192
#define	v9	230
#define	v10	271
#define	v11	321
#define	v12	367
#define	v13	425

#define QRCL v4+1
#define DL 16
#define MyTupletCString(_key, _cstring) \
((const Tuplet) { .type = TUPLE_CSTRING, .key = _key, .cstring = { .data = _cstring, .length = strlen(_cstring) + 1 }})
char QRContent1[QRCL]="1";
char QRContent2[QRCL]="2";
char QRContent3[QRCL]="3";
char QRContent4[QRCL]="4";
char QRDesc1[DL]="Sample 1";
char QRDesc2[DL]="Sample 2";
char QRDesc3[DL]="Sample 3";
char QRDesc4[DL]="Sample 4";
static AppSync sync;
static uint8_t sync_buffer[(QRCL+DL+(7*2))*MAX_CODES+2];

static void sync_error_callback(DictionaryResult dict_error, AppMessageResult app_message_error, void *context) {
 //   APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Sync Error: %d", app_message_error);
}

char* getDesc(int i)
{
	switch(i)
	{
		case 0: return QRDesc1;break;
		case 1: return QRDesc2;break;
		case 2: return QRDesc3;break;
		case 3: return QRDesc4;break;
	}
	return NULL;
}

char* getContent(int i)
{
	switch(i)
	{
		case 0: return QRContent1;break;
		case 1: return QRContent2;break;
		case 2: return QRContent3;break;
		case 3: return QRContent4;break;
	}
	return NULL;
}

char* getStr(int i)
{
	if(i>3)
		return getDesc(i-4);
	return getContent(i);
}

int getStrLen(int i)
{
	if(i>3)
		return DL;
	return QRCL;
}

void drawQR(Layer *self, GContext *ctxt)
{
	//app_log(APP_LOG_LEVEL_INFO, "PebbleQRShare.c", 96, "starting drawQR");
	//app_log(APP_LOG_LEVEL_INFO, "PebbleQRShare.c", 97, "Desc: %s", getDesc(index));
	if(code)
		QRcode_free(code);
	code=NULL;
	char *str=getContent(index);

	code = QRcode_encodeData(strlen(str), (unsigned char*)str, version, level);
	int maxSize=140;
	//app_log(APP_LOG_LEVEL_INFO, "PebbleQRShare.c", 104, "qr width: %d, qr version: %d, data: %s", code->width, code->version, str);
	int pixelsPerBlock=maxSize/code->width;
	int imageWidth=pixelsPerBlock*code->width;
	int offset=(maxSize-imageWidth)/2;
	unsigned char* row=NULL;
	graphics_context_set_fill_color(ctxt, GColorWhite);
	graphics_fill_rect(ctxt, bounds, 0, GCornerNone); 
	for(int y=0;y<code->width;y++)
	{
		row = code->data+(y*code->width);
		for(int x=0;x<code->width;x++)
		{
			if(row[x]&0x1)
				graphics_context_set_fill_color(ctxt, GColorBlack);
			else
				graphics_context_set_fill_color(ctxt, GColorWhite);
			GRect rect=(GRect) { .origin = { (x*pixelsPerBlock)+offset+((144-maxSize)/2), (y*pixelsPerBlock)+offset }, .size = { pixelsPerBlock, pixelsPerBlock } };
			graphics_fill_rect(ctxt, rect, 0, GCornerNone); 
		}
	}
	return;
}

void drawWindow(char*content, char* description)
{
	text_layer_set_text(text_layer, description);
}

static void sync_tuple_changed_callback(const uint32_t key, const Tuple *new_tuple, const Tuple *old_tuple, void *context) 
{
	char* str=getStr(key);
	snprintf(str, getStrLen(key), "%s", new_tuple->value->cstring);
	int written=persist_write_string(key, str);
	drawWindow(getContent(index), getDesc(index));
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) 
{
	index++;
	if(index >=MAX_CODES)
		index=0;
	drawWindow(getContent(index), getDesc(index));
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) 
{
	index--;
	if(index<0)
		index=MAX_CODES-1;
	drawWindow(getContent(index), getDesc(index));
}

static void click_config_provider(void *context) 
{
	window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
	window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void window_load(Window *window) 
{
	window_layer = window_get_root_layer(window);
	bounds = layer_get_bounds(window_layer);
	text_layer = text_layer_create((GRect) { .origin = { 0, 137 }, .size = { bounds.size.w, 20 } });
	text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
	layer_add_child(window_layer, text_layer_get_layer(text_layer));
	layer_set_update_proc(window_layer, drawQR);
	drawWindow(getContent(index), getDesc(index));
}

static void window_unload(Window *window) 
{
	text_layer_destroy(text_layer);
}

static void send_cmd(void) {
  Tuplet value = TupletInteger(1, 1);
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  if (iter == NULL) {
    return;
  }

  dict_write_tuplet(iter, &value);
  dict_write_end(iter);

  app_message_outbox_send();
}


static void init(void) 
{
	app_message_open(512, 512);

//	app_log(APP_LOG_LEVEL_INFO, "PebbleQRShare.c", 209, "init");
	persist_read_string(QRCONTENT1_KEY, QRContent1, sizeof(QRContent1));
	persist_read_string(QRCONTENT2_KEY, QRContent2, sizeof(QRContent2));
	persist_read_string(QRCONTENT3_KEY, QRContent3, sizeof(QRContent3));
	persist_read_string(QRCONTENT4_KEY, QRContent4, sizeof(QRContent4));
	persist_read_string(QRDESC1_KEY, QRDesc1, sizeof(QRDesc1));
	persist_read_string(QRDESC2_KEY, QRDesc2, sizeof(QRDesc2));
	persist_read_string(QRDESC3_KEY, QRDesc3, sizeof(QRDesc3));
	persist_read_string(QRDESC4_KEY, QRDesc4, sizeof(QRDesc4));

//	app_log(APP_LOG_LEVEL_INFO, "PebbleQRShare.c", 192, "init");
	Tuplet initial_values[] = {
		MyTupletCString(QRCONTENT1_KEY, QRContent1),
		MyTupletCString(QRCONTENT2_KEY, QRContent2),
		MyTupletCString(QRCONTENT3_KEY, QRContent3),
		MyTupletCString(QRCONTENT4_KEY, QRContent4),
		MyTupletCString(QRDESC1_KEY, QRDesc1),
		MyTupletCString(QRDESC2_KEY, QRDesc2),
		MyTupletCString(QRDESC3_KEY, QRDesc3),
		MyTupletCString(QRDESC4_KEY, QRDesc4)
	};

//	app_log(APP_LOG_LEVEL_INFO, "PebbleQRShare.c", 204, "init");
 	app_sync_init(&sync, sync_buffer, sizeof(sync_buffer), initial_values, ARRAY_LENGTH(initial_values), sync_tuple_changed_callback, sync_error_callback, NULL);
    //send_cmd();  

//	app_log(APP_LOG_LEVEL_INFO, "PebbleQRShare.c", 219, "init");
	code=NULL;

	window = window_create();
	window_set_click_config_provider(window, click_config_provider);
	window_set_window_handlers(window, (WindowHandlers) {
			.load = window_load,
			.unload = window_unload,
			});
	const bool animated = true;
	window_stack_push(window, animated);
}

static void deinit(void) 
{
	window_destroy(window);
	layer_destroy(window_layer);
}

int main(void) 
{
	init();
	app_event_loop();
	deinit();
}
