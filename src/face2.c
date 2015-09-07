#include "pebble.h"

// COMMON VARIABLES
#define BACKGROUND_COLOR GColorElectricUltramarine
#define FOREGROUND_COLOR GColorBlack
#define HOUR_HAND_COLOR GColorRed
#define MINUTE_HAND_COLOR GColorKellyGreen
#define SECOND_HAND_COLOR GColorBlueMoon
#define TICK_COLOR GColorWhite
#define TICK_CIRCLE_RADIUS 4
#define ANALOG_BORDER_COLOR GColorBlack
#define ANALOG_FACE_COLOR GColorBabyBlueEyes

	
// SOME BASIC DECLARATION
static Window *window;
static Layer *s_simple_bg_layer, *s_info_layer, *s_analog_clock_layer, *s_battery_draw_layer, *s_calendar_draw_layer;
static TextLayer *s_date_layer, *s_time_layer, *s_day_layer, *s_battery_layer, *s_month_layer;
static BitmapLayer *s_bitmap_layer_bluetooth;
static char s_time_buffer[7], s_date_buffer[3], s_day_buffer[5], s_month_buffer[4];
static GBitmap *s_bitmap_bluetooth;
static GFont s_carbon_font_battery, s_carbon_font_time, s_carbon_font_date, s_carbon_font_month;

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

// HAND UPDATE PROCESS
static void analog_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  GPoint center = grect_center_point(&bounds);
  // BACKGROUND COLOR
  graphics_context_set_fill_color(ctx, BACKGROUND_COLOR);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

	// HAND LENGTHs
	int16_t second_hand_length = 46;
  int16_t minute_hand_length = 41;
  int16_t hour_hand_length = 30;

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
	graphics_context_set_fill_color(ctx, ANALOG_FACE_COLOR);
  graphics_fill_circle(ctx, center, 50);
  // BORDER of the ANALOG clock
	graphics_context_set_stroke_width(ctx, 5);
	graphics_context_set_stroke_color(ctx, ANALOG_BORDER_COLOR);
  graphics_draw_circle(ctx, center, 50);
  	
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
  graphics_draw_line(ctx, second_hand, center);

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
	graphics_context_set_stroke_width(ctx, 2);
  graphics_draw_round_rect(ctx, GRect(2, 3, 25, 12), 2);

// battery level
	graphics_context_set_fill_color(ctx, FOREGROUND_COLOR);
	graphics_context_set_stroke_width(ctx, 1);

	BatteryChargeState charge_state = battery_state_service_peek();
	int battery_level = charge_state.charge_percent;
	
	switch (battery_level) {
		case 100:
			graphics_fill_rect(ctx, GRect(5, 6, 19, 6), 1, GCornersAll);
		  break;
		case 90:
			graphics_fill_rect(ctx, GRect(5, 6, 18, 6), 1, GCornersAll);
		  break;
		case 80:
			graphics_fill_rect(ctx, GRect(5, 6, 16, 6), 1, GCornersAll);
		  break;
		case 70:
			graphics_fill_rect(ctx, GRect(5, 6, 14, 6), 1, GCornersAll);
		  break;
		case 60:
			graphics_fill_rect(ctx, GRect(5, 6, 12, 6), 1, GCornersAll);
		  break;
		case 50:
			graphics_fill_rect(ctx, GRect(5, 6, 10, 6), 1, GCornersAll);
		  break;
		case 40:
			graphics_fill_rect(ctx, GRect(5, 6, 8, 6), 1, GCornersAll);
		  break;
		case 30:
			graphics_fill_rect(ctx, GRect(5, 6, 6, 6), 1, GCornersAll);
		  break;
		case 20:
			graphics_context_set_fill_color(ctx, GColorRed);
			graphics_fill_rect(ctx, GRect(5, 6, 4, 6), 1, GCornersAll);
		  break;
		case 10:
			graphics_context_set_fill_color(ctx, GColorRed);
			graphics_fill_rect(ctx, GRect(5, 6, 2, 6), 1, GCornersAll);
		  break;
	}

}

static void calendar_draw_update_proc(Layer *layer, GContext *ctx) {
  //GRect bounds = layer_get_bounds(layer);
  //GPoint center = grect_center_point(&bounds);
  // BACKGROUND COLOR for calendar drawing
  //graphics_context_set_fill_color(ctx, BACKGROUND_COLOR);
  //graphics_fill_rect(ctx, bounds, 0, GCornerNone);
	// calendar drawing
	graphics_context_set_antialiased(ctx, true);
	graphics_context_set_stroke_color(ctx, FOREGROUND_COLOR);
	graphics_context_set_stroke_width(ctx, 2);
	graphics_context_set_fill_color(ctx, FOREGROUND_COLOR);
	graphics_fill_rect(ctx, GRect(1, 4, 43, 44), 4, GCornersAll);
	graphics_context_set_fill_color(ctx, BACKGROUND_COLOR);
	graphics_fill_rect(ctx, GRect(3, 22, 39, 24), 0, GCornerNone);
  graphics_fill_circle(ctx, GPoint(22,4), 3);
  graphics_fill_circle(ctx, GPoint(11,4), 3);
  graphics_fill_circle(ctx, GPoint(33,4), 3);
	graphics_context_set_fill_color(ctx, FOREGROUND_COLOR);
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
  GRect analog_bounds = GRect(0, 0, 111, 115);
  s_analog_clock_layer = layer_create(analog_bounds);
  layer_set_update_proc(s_analog_clock_layer, analog_update_proc);
  layer_add_child(window_layer, s_analog_clock_layer);

	// BATTERY drawings
  s_battery_draw_layer = layer_create(GRect(113, 4, 30, 18));
  layer_set_update_proc(s_battery_draw_layer, battery_draw_update_proc);
  layer_add_child(window_layer, s_battery_draw_layer);

	// CALENDAR drawings
  s_calendar_draw_layer = layer_create(GRect(95, 116, 45, 50));
  layer_set_update_proc(s_calendar_draw_layer, calendar_draw_update_proc);
  layer_add_child(window_layer, s_calendar_draw_layer);

	// BITMAP for Bluetooth
  s_bitmap_bluetooth = gbitmap_create_with_resource(RESOURCE_ID_BT_ICON);
  s_bitmap_layer_bluetooth = bitmap_layer_create(GRect(116, 55, 60, 70));
  bitmap_layer_set_bitmap(s_bitmap_layer_bluetooth, s_bitmap_bluetooth);
  bitmap_layer_set_compositing_mode(s_bitmap_layer_bluetooth, GCompOpSet);
  bitmap_layer_set_alignment(s_bitmap_layer_bluetooth, GAlignTopLeft);
  //layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_bitmap_layer_bluetooth));

	// INFO LAYER containing child text layers
  s_info_layer = layer_create(bounds);
  layer_set_update_proc(s_info_layer, info_update_proc);
  layer_add_child(window_layer, s_info_layer);
	
	// TEXT LAYER for DATE (hanyadika)
  s_date_layer = text_layer_create(GRect(99, 138, 37, 23));
  text_layer_set_text(s_date_layer, s_date_buffer);
  text_layer_set_background_color(s_date_layer, GColorClear);
	s_carbon_font_date = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_CARBON_20));
	text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  text_layer_set_text_color(s_date_layer, FOREGROUND_COLOR);
  text_layer_set_font(s_date_layer, s_carbon_font_date);
  layer_add_child(s_info_layer, text_layer_get_layer(s_date_layer));

	// TEXT LAYER for MONTH
  s_month_layer = text_layer_create(GRect(99, 123, 37, 23));
  text_layer_set_text(s_month_layer, s_month_buffer);
  text_layer_set_background_color(s_month_layer, GColorClear);
	s_carbon_font_month = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_CARBON_13));
	text_layer_set_text_alignment(s_month_layer, GTextAlignmentCenter);
  text_layer_set_text_color(s_month_layer, BACKGROUND_COLOR);
  text_layer_set_font(s_month_layer, s_carbon_font_month);
  layer_add_child(s_info_layer, text_layer_get_layer(s_month_layer));

	// TEXT LAYER for TIME
  s_time_layer = text_layer_create(GRect(4, 110, 90, 50));
  text_layer_set_text(s_time_layer, s_time_buffer);
  text_layer_set_background_color(s_time_layer, GColorClear);
	s_carbon_font_time = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_CARBON_36));
	text_layer_set_text_alignment(s_time_layer, GTextAlignmentLeft);
  text_layer_set_text_color(s_time_layer, FOREGROUND_COLOR);
  text_layer_set_font(s_time_layer, s_carbon_font_time);
  layer_add_child(s_info_layer, text_layer_get_layer(s_time_layer));
	
	// TEXT LAYER for DAY
  s_day_layer = text_layer_create(GRect(0, 144, 144, 24));
  text_layer_set_text(s_day_layer, s_day_buffer);
  text_layer_set_background_color(s_day_layer, GColorClear);
	text_layer_set_text_alignment(s_day_layer, GTextAlignmentRight);
  text_layer_set_text_color(s_day_layer, GColorWhite);
  text_layer_set_font(s_day_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  //layer_add_child(s_info_layer, text_layer_get_layer(s_day_layer));
	
	// BATTERY LAYER
  s_battery_layer = text_layer_create(GRect(114, 20, 29, 18));
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