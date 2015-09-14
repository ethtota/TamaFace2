#include "pebble.h"

// COMMON VARIABLES
#define BACKGROUND_COLOR GColorBlack
#define FOREGROUND_COLOR GColorWhite
#define CALENDAR_COLOR GColorLightGray
#define HOUR_HAND_COLOR GColorRichBrilliantLavender
#define MINUTE_HAND_COLOR GColorMelon
#define SECOND_HAND_COLOR GColorPictonBlue
#define TICK_COLOR GColorWhite
#define TICK_CIRCLE_RADIUS 4
#define ANALOG_BORDER_COLOR GColorWhite
#define ANALOG_FACE_COLOR GColorRajah

// SOME BASIC DECLARATION
static Window *window;
static Layer *s_simple_bg_layer, *s_info_layer, *s_analog_clock_layer, *s_battery_draw_layer, *s_calendar_draw_layer;
static TextLayer *s_date_layer, *s_time_layer, *s_seconds_layer, *s_day_layer, *s_battery_layer, *s_month_layer;
static BitmapLayer *s_bitmap_layer_bluetooth;
static char s_time_buffer[7], s_date_buffer[3], s_day_buffer[5], s_month_buffer[4], s_seconds_buffer[3];
static GBitmap *s_bitmap_bluetooth_ok, *s_bitmap_bluetooth_ng;
static GFont s_carbon_font_battery, s_carbon_font_time, s_carbon_font_seconds, s_carbon_font_date, s_carbon_font_month, s_carbon_font_day;

// BACKGROUND UPDATE PROCESS
static void bg_update_proc(Layer *layer, GContext *ctx) {
	// BACKGROUND COLOR
  graphics_context_set_fill_color(ctx, BACKGROUND_COLOR);
  graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
  // ORA VONALAK/KOROR
  //graphics_context_set_fill_color(ctx, TICK_COLOR);
  //for (int i = 0; i < 60; i+=5) {
  //  GPoint draw_to = (GPoint) {
  //  .x = (int16_t)(sin_lookup(TRIG_MAX_ANGLE * i / 60) * 67 / TRIG_MAX_RATIO) + 72,
  //  .y = (int16_t)(-cos_lookup(TRIG_MAX_ANGLE * i / 60) * 67 / TRIG_MAX_RATIO) + 72 };
  //graphics_fill_circle(ctx, draw_to, TICK_CIRCLE_RADIUS);
  //printf("Numbers: %d - %i - %i", i,(int16_t)draw_to.x,(int16_t)draw_to.y);
  //}

  // MANUAL CIRCLE TICKS
  //graphics_fill_circle(ctx, GPoint(72,5), TICK_CIRCLE_RADIUS);

	// INFO LAYER BACKGROUND
	//graphics_context_set_fill_color(ctx, INFO_LAYER_BKG_COLOR);
	//graphics_fill_rect(ctx, GRect(0, 146, 144, 24), 1, GCornerNone);
	
  // FONT TEST
  //GFont dense_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_DENSE_22));
  //graphics_context_set_text_color(ctx, GColorWhite);
  //graphics_draw_text(ctx, "12/25 WED 18:33", dense_font, GRect(0, 143, 144, 24), GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
  //graphics_draw_text(ctx, "88:88", fonts_get_system_font(FONT_KEY_GOTHIC_18), GRect(0, 143, 144, 24), GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
}

// BATTERY HANDLER
static void handle_battery(BatteryChargeState charge_state) {
  static char battery_text[] = "charging";

  if (charge_state.is_charging) {
    snprintf(battery_text, sizeof(battery_text), "chg");
  } else {
    snprintf(battery_text, sizeof(battery_text), "%d%%", charge_state.charge_percent);
  }
  text_layer_set_text(s_battery_layer, battery_text);
}

// BLUETOOTH HANDLER
static void bt_handler(bool connected) {
  // Show current connection state
  if (connected) {
  bitmap_layer_set_bitmap(s_bitmap_layer_bluetooth, s_bitmap_bluetooth_ok);
  } else {
  bitmap_layer_set_bitmap(s_bitmap_layer_bluetooth, s_bitmap_bluetooth_ng);
  }
}

// HAND UPDATE PROCESS
static void analog_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  GPoint center = grect_center_point(&bounds);
  //int16_t analog_radius = bounds.size.w/2-2;
  int16_t analog_radius = bounds.size.w/2;
	// BACKGROUND COLOR
  graphics_context_set_fill_color(ctx, BACKGROUND_COLOR);
  //graphics_context_set_fill_color(ctx, GColorRed); //temp
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

	// HAND LENGTHs calculation
	int16_t second_hand_length = analog_radius*0.95;
  int16_t minute_hand_length = analog_radius*0.75;
  int16_t hour_hand_length = analog_radius*0.55;

	// ANGLE CALCULATION
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  int32_t second_angle = TRIG_MAX_ANGLE * t->tm_sec / 60;
  //int32_t minute_angle = TRIG_MAX_ANGLE * t->tm_min / 60;
	float hour_angle = TRIG_MAX_ANGLE * (((t->tm_hour % 12) * 6) + (t->tm_min / 10)) / (12 * 6);
	graphics_context_set_antialiased(ctx, true);
  
  GPoint second_hand = {
    .x = (int16_t)(sin_lookup(second_angle) * (int32_t)second_hand_length / TRIG_MAX_RATIO) + center.x,
    .y = (int16_t)(-cos_lookup(second_angle) * (int32_t)second_hand_length / TRIG_MAX_RATIO) + center.y,
  };
  GPoint second_hand_from = {
    .x = (int16_t)(sin_lookup(second_angle) * (int32_t)-15 / TRIG_MAX_RATIO) + center.x,
    .y = (int16_t)(-cos_lookup(second_angle) * (int32_t)-15 / TRIG_MAX_RATIO) + center.y,
  };
  GPoint minute_hand = (GPoint) {
    .x = (int16_t)(sin_lookup(TRIG_MAX_ANGLE * t->tm_min / 60) * (int32_t)(minute_hand_length) / TRIG_MAX_RATIO) + center.x,
    .y = (int16_t)(-cos_lookup(TRIG_MAX_ANGLE * t->tm_min / 60) * (int32_t)(minute_hand_length) / TRIG_MAX_RATIO) + center.y,
  };
  GPoint hour_hand = (GPoint) {
    .x = (int16_t)(sin_lookup(hour_angle) * (int32_t)(hour_hand_length) / TRIG_MAX_RATIO) + center.x,
    .y = (int16_t)(-cos_lookup(hour_angle) * (int32_t)(hour_hand_length) / TRIG_MAX_RATIO) + center.y,
  };

	// DRAWING the ANALOG clock
	// FACE of the ANALOG clock
	//graphics_context_set_fill_color(ctx, ANALOG_FACE_COLOR);
  //graphics_fill_circle(ctx, center, analog_radius);
  // BORDER of the ANALOG clock
	//graphics_context_set_stroke_width(ctx, 5);
	//graphics_context_set_stroke_color(ctx, ANALOG_BORDER_COLOR);
  //graphics_draw_circle(ctx, center, analog_radius);

	// TICK DRAWING
  graphics_context_set_stroke_color(ctx, TICK_COLOR);
  graphics_context_set_stroke_width(ctx, 1);
  for (int i = 0; i < 60; i+=1) {
    GPoint draw_from = (GPoint) {
    .x = (int16_t)(sin_lookup(TRIG_MAX_ANGLE * i / 60) * (analog_radius-3) / TRIG_MAX_RATIO) + center.x,
    .y = (int16_t)(-cos_lookup(TRIG_MAX_ANGLE * i / 60) * (analog_radius-3) / TRIG_MAX_RATIO) + center.y };
    GPoint draw_to = (GPoint) {
    .x = (int16_t)(sin_lookup(TRIG_MAX_ANGLE * i / 60) * analog_radius / TRIG_MAX_RATIO) + center.x,
    .y = (int16_t)(-cos_lookup(TRIG_MAX_ANGLE * i / 60) * analog_radius / TRIG_MAX_RATIO) + center.y };
  graphics_draw_line(ctx, draw_from, draw_to);
	}
  graphics_context_set_stroke_width(ctx, 3);
	for (int i = 0; i < 60; i+=5) {
    GPoint draw_from = (GPoint) {
    .x = (int16_t)(sin_lookup(TRIG_MAX_ANGLE * i / 60) * (analog_radius-5) / TRIG_MAX_RATIO) + center.x,
    .y = (int16_t)(-cos_lookup(TRIG_MAX_ANGLE * i / 60) * (analog_radius-5) / TRIG_MAX_RATIO) + center.y };
    GPoint draw_to = (GPoint) {
    .x = (int16_t)(sin_lookup(TRIG_MAX_ANGLE * i / 60) * analog_radius / TRIG_MAX_RATIO) + center.x,
    .y = (int16_t)(-cos_lookup(TRIG_MAX_ANGLE * i / 60) * analog_radius / TRIG_MAX_RATIO) + center.y };
  graphics_draw_line(ctx, draw_from, draw_to);
	}

  // minute/hour hand
	// hour hand base circle
  graphics_context_set_fill_color(ctx, HOUR_HAND_COLOR);
  graphics_fill_circle(ctx, center, 6);
  // hour hand
  graphics_context_set_stroke_width(ctx, 5);
  graphics_context_set_stroke_color(ctx, HOUR_HAND_COLOR);
  graphics_draw_line(ctx, center, hour_hand);
  // minute hand
  graphics_context_set_stroke_width(ctx, 5);
  graphics_context_set_stroke_color(ctx, MINUTE_HAND_COLOR);
  graphics_draw_line(ctx, center, minute_hand);
  // second hand
  graphics_context_set_stroke_width(ctx, 2);
  graphics_context_set_stroke_color(ctx, SECOND_HAND_COLOR);
  graphics_draw_line(ctx, second_hand_from, second_hand);

  // circle in the middle
  graphics_context_set_fill_color(ctx, SECOND_HAND_COLOR);
  graphics_fill_circle(ctx, center, 5);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_circle(ctx, center, 2);
	
}

// BATTERY DRAW UPDATE PROCESS
static void battery_draw_update_proc(Layer *layer, GContext *ctx) {
  //GRect bounds = layer_get_bounds(layer);
  //GPoint center = grect_center_point(&bounds);
  // BACKGROUND COLOR for battery drawing
  //graphics_context_set_fill_color(ctx, GColorClear);
  //graphics_fill_rect(ctx, bounds, 0, GCornerNone);
  graphics_context_set_antialiased(ctx, true);
	
	// battery outline
	graphics_context_set_stroke_color(ctx, FOREGROUND_COLOR);
	graphics_context_set_stroke_width(ctx, 1);
  graphics_draw_round_rect(ctx, GRect(2, 3, 28, 12), 4);
  graphics_draw_round_rect(ctx, GRect(30, 7, 1, 4), 1);

// battery level
	graphics_context_set_fill_color(ctx, FOREGROUND_COLOR);

	BatteryChargeState charge_state = battery_state_service_peek();
	int battery_level = charge_state.charge_percent;
	
	if (charge_state.is_charging) {
			graphics_context_set_fill_color(ctx, GColorBlueMoon);
			graphics_fill_rect(ctx, GRect(4, 5, 24, 8), 1, GCornersAll);
  } else {
	switch (battery_level) {
		case 100:
			graphics_context_set_fill_color(ctx, GColorGreen);
			graphics_fill_rect(ctx, GRect(4, 5, 24, 8), 1, GCornersAll);
		  break;
		case 90:
			graphics_context_set_fill_color(ctx, GColorGreen);
			graphics_fill_rect(ctx, GRect(4, 5, 22, 8), 1, GCornersAll);
		  break;
		case 80:
			graphics_context_set_fill_color(ctx, GColorMalachite);
			graphics_fill_rect(ctx, GRect(4, 5, 19, 8), 1, GCornersAll);
		  break;
		case 70:
			graphics_context_set_fill_color(ctx, GColorInchworm);
			graphics_fill_rect(ctx, GRect(4, 5, 16, 8), 1, GCornersAll);
		  break;
		case 60:
			graphics_context_set_fill_color(ctx, GColorInchworm);
			graphics_fill_rect(ctx, GRect(4, 5, 14, 8), 1, GCornersAll);
		  break;
		case 50:
			graphics_context_set_fill_color(ctx, GColorRajah);
			graphics_fill_rect(ctx, GRect(4, 5, 12, 8), 1, GCornersAll);
		  break;
		case 40:
			graphics_context_set_fill_color(ctx, GColorChromeYellow);
			graphics_fill_rect(ctx, GRect(4, 5, 9, 8), 1, GCornersAll);
		  break;
		case 30:
			graphics_context_set_fill_color(ctx, GColorChromeYellow);
			graphics_fill_rect(ctx, GRect(4, 5, 7, 8), 1, GCornersAll);
		  break;
		case 20:
			graphics_context_set_fill_color(ctx, GColorRed);
			graphics_fill_rect(ctx, GRect(4, 5, 4, 8), 1, GCornersAll);
		  break;
		case 10:
			graphics_context_set_fill_color(ctx, GColorRed);
			graphics_fill_rect(ctx, GRect(4, 5, 2, 8), 1, GCornersAll);
		  break;
		case 0:
			graphics_context_set_fill_color(ctx, GColorRed);
			graphics_fill_rect(ctx, GRect(4, 5, 1, 8), 1, GCornersAll);
		  break;
	}
	}
}

static void calendar_draw_update_proc(Layer *layer, GContext *ctx) {
  //GRect bounds = layer_get_bounds(layer);
  //GPoint center = grect_center_point(&bounds);
  // BACKGROUND COLOR for calendar drawing
  //graphics_context_set_fill_color(ctx, GColorTiffanyBlue);
  //graphics_fill_rect(ctx, bounds, 0, GCornerNone);
	// calendar drawing
	graphics_context_set_antialiased(ctx, true);
	graphics_context_set_stroke_color(ctx, CALENDAR_COLOR);
	graphics_context_set_stroke_width(ctx, 2);
	graphics_context_set_fill_color(ctx, CALENDAR_COLOR);
	graphics_fill_rect(ctx, GRect(1, 4, 44, 48), 5, GCornersAll);
	graphics_context_set_fill_color(ctx, BACKGROUND_COLOR);
	graphics_fill_rect(ctx, GRect(3, 25, 40, 25), 2, GCornersAll);
  graphics_fill_circle(ctx, GPoint(22,4), 3);
  graphics_fill_circle(ctx, GPoint(11,4), 3);
  graphics_fill_circle(ctx, GPoint(33,4), 3);
	graphics_context_set_fill_color(ctx, CALENDAR_COLOR);
	graphics_fill_rect(ctx, GRect(20, 0, 5, 7), 2, GCornersAll);
	graphics_fill_rect(ctx, GRect(9, 0, 5, 7), 2, GCornersAll);
	graphics_fill_rect(ctx, GRect(31, 0, 5, 7), 2, GCornersAll);
}
static void info_update_proc(Layer *layer, GContext *ctx) {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);

  strftime(s_date_buffer, sizeof(s_date_buffer), "%d", t);
  text_layer_set_text(s_date_layer, s_date_buffer);

  strftime(s_month_buffer, sizeof(s_month_buffer), "%b", t);
  text_layer_set_text(s_month_layer, s_month_buffer);

	strftime(s_time_buffer, sizeof(s_time_buffer), "%H:%M", t);
  text_layer_set_text(s_time_layer, s_time_buffer);

	strftime(s_seconds_buffer, sizeof(s_seconds_buffer), "%S", t);
  text_layer_set_text(s_seconds_layer, s_seconds_buffer);

  strftime(s_day_buffer, sizeof(s_day_buffer), "%a", t);
  text_layer_set_text(s_day_layer, s_day_buffer);
}

static void handle_second_tick(struct tm *tick_time, TimeUnits units_changed) {
  layer_mark_dirty(window_get_root_layer(window));
	handle_battery(battery_state_service_peek());
}

// -------- MAIN WINDOW HANDLERS -------- //

// LOAD
static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
	//GFont dense_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_DENSE_22));
  //GFont battery_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_DENSE_14));

	// BACKGROUND LAYER
  s_simple_bg_layer = layer_create(bounds);
  layer_set_update_proc(s_simple_bg_layer, bg_update_proc);
  layer_add_child(window_layer, s_simple_bg_layer);
  
	// ANALOG CLOCK LAYER
  GRect analog_bounds = GRect(0, 0, 128, 128);
  s_analog_clock_layer = layer_create(analog_bounds);
  layer_set_update_proc(s_analog_clock_layer, analog_update_proc);
  layer_add_child(window_layer, s_analog_clock_layer);

	// BATTERY drawings
  s_battery_draw_layer = layer_create(GRect(113, 0, 35, 18));
  layer_set_update_proc(s_battery_draw_layer, battery_draw_update_proc);
  layer_add_child(window_layer, s_battery_draw_layer);

	// CALENDAR drawings
  s_calendar_draw_layer = layer_create(GRect(99, 116, 60, 60));
  layer_set_update_proc(s_calendar_draw_layer, calendar_draw_update_proc);
  layer_add_child(window_layer, s_calendar_draw_layer);

	// BITMAP for Bluetooth
  s_bitmap_bluetooth_ok = gbitmap_create_with_resource(RESOURCE_ID_BT_CONNECTED);
  s_bitmap_bluetooth_ng = gbitmap_create_with_resource(RESOURCE_ID_BT_NOT_CONNECTED);
  s_bitmap_layer_bluetooth = bitmap_layer_create(GRect(122, 91, 20, 20));
  bitmap_layer_set_bitmap(s_bitmap_layer_bluetooth, s_bitmap_bluetooth_ok);
  bitmap_layer_set_compositing_mode(s_bitmap_layer_bluetooth, GCompOpSet);
  bitmap_layer_set_alignment(s_bitmap_layer_bluetooth, GAlignTopLeft);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_bitmap_layer_bluetooth));
	bt_handler(bluetooth_connection_service_peek());

	// INFO LAYER containing child text layers
  s_info_layer = layer_create(bounds);
  layer_set_update_proc(s_info_layer, info_update_proc);
  layer_add_child(window_layer, s_info_layer);
	
	// TEXT LAYER for DATE (hanyadika)
  s_date_layer = text_layer_create(GRect(104, 140, 37, 26));
  text_layer_set_text(s_date_layer, s_date_buffer);
  text_layer_set_background_color(s_date_layer, GColorClear);
	s_carbon_font_date = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_CARBON_22));
	text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  text_layer_set_text_color(s_date_layer, FOREGROUND_COLOR);
  text_layer_set_font(s_date_layer, s_carbon_font_date);
  layer_add_child(s_info_layer, text_layer_get_layer(s_date_layer));

	// TEXT LAYER for MONTH
  s_month_layer = text_layer_create(GRect(104, 122, 37, 26));
  text_layer_set_text(s_month_layer, s_month_buffer);
  text_layer_set_background_color(s_month_layer, GColorClear);
	s_carbon_font_month = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_CARBON_17));
	text_layer_set_text_alignment(s_month_layer, GTextAlignmentCenter);
  text_layer_set_text_color(s_month_layer, BACKGROUND_COLOR);
  text_layer_set_font(s_month_layer, s_carbon_font_month);
  layer_add_child(s_info_layer, text_layer_get_layer(s_month_layer));

	// TEXT LAYER for TIME
  s_time_layer = text_layer_create(GRect(0, 135, 74, 40));
  text_layer_set_text(s_time_layer, s_time_buffer);
  text_layer_set_background_color(s_time_layer, GColorClear);
	s_carbon_font_time = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_CARBON_30));
	text_layer_set_text_alignment(s_time_layer, GTextAlignmentRight);
  text_layer_set_text_color(s_time_layer, FOREGROUND_COLOR);
  text_layer_set_font(s_time_layer, s_carbon_font_time);
  layer_add_child(s_info_layer, text_layer_get_layer(s_time_layer));

	// TEXT LAYER for seconds (TIME)
  s_seconds_layer = text_layer_create(GRect(79, 150, 25, 25));
  text_layer_set_text(s_seconds_layer, s_seconds_buffer);
  text_layer_set_background_color(s_seconds_layer, GColorClear);
	s_carbon_font_seconds = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_CARBON_15));
	text_layer_set_text_alignment(s_seconds_layer, GTextAlignmentLeft);
  text_layer_set_text_color(s_seconds_layer, SECOND_HAND_COLOR);
  text_layer_set_font(s_seconds_layer, s_carbon_font_seconds);
  layer_add_child(s_info_layer, text_layer_get_layer(s_seconds_layer));

	// TEXT LAYER for DAY
  s_day_layer = text_layer_create(GRect(59, 128, 40, 24));
  text_layer_set_text(s_day_layer, s_day_buffer);
  text_layer_set_background_color(s_day_layer, GColorClear);
	text_layer_set_text_alignment(s_day_layer, GTextAlignmentRight);
  text_layer_set_text_color(s_day_layer, CALENDAR_COLOR);
	s_carbon_font_day = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_CARBON_13));
  text_layer_set_font(s_day_layer, s_carbon_font_day);
  layer_add_child(s_info_layer, text_layer_get_layer(s_day_layer));
	
	// BATTERY text LAYER
  s_battery_layer = text_layer_create(GRect(116, 15, 29, 18));
  text_layer_set_text_color(s_battery_layer, FOREGROUND_COLOR);
  text_layer_set_background_color(s_battery_layer, GColorClear);
	s_carbon_font_battery = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_CARBON_11));
  text_layer_set_font(s_battery_layer, s_carbon_font_battery);
  text_layer_set_text_alignment(s_battery_layer, GTextAlignmentCenter);
  text_layer_set_text(s_battery_layer, "chg");
  layer_add_child(window_layer, text_layer_get_layer(s_battery_layer));
  
}

// UNLOAD
static void window_unload(Window *window) {
  layer_destroy(s_simple_bg_layer);
  layer_destroy(s_info_layer);

  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_time_layer);

  layer_destroy(s_analog_clock_layer);
}

// -------- MAIN, INIT, DE-INIT -------- //

// INIT
static void init() {
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(window, true);

  tick_timer_service_subscribe(SECOND_UNIT, handle_second_tick);
  battery_state_service_subscribe(handle_battery);
  bluetooth_connection_service_subscribe(bt_handler);
}

// DE-INIT
static void deinit() {

  tick_timer_service_unsubscribe();
	battery_state_service_unsubscribe();
  window_destroy(window);
}

// MAIN program
int main() {
  init();
  app_event_loop();
  deinit();
}