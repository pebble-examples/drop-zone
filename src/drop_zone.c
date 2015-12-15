#include <pebble.h>

#define TEXT_BOX_HEIGHT 60
#define ANIMATION_DURATION_IN_MS 1500

static Window *s_main_window;
static TextLayer *s_time_layer;

static bool s_move_direction_is_up;


static void anim_stopped_handler(Animation *animation, bool finished, void *context) {
  // Need to be static because it's used by the system later.
  static char s_time_text[] = "00:00";

  // Show the new time
  time_t tm_t = time(NULL);
  struct tm *time_now = localtime(&tm_t);
  strftime(s_time_text, sizeof(s_time_text), "%R", time_now);
  text_layer_set_text(s_time_layer, s_time_text);
}

static void schedule_animation() {
  // Flip the direction of movement each minute
  s_move_direction_is_up = !s_move_direction_is_up;

  GRect bounds = layer_get_bounds(window_get_root_layer(s_main_window));
  GRect start = layer_get_frame(text_layer_get_layer(s_time_layer));
  GRect finish;
  if (s_move_direction_is_up) {
    // We're going down
    finish = GRect(start.origin.x, bounds.size.h - TEXT_BOX_HEIGHT, start.size.w, start.size.h);
  } else {
    // We're going up
    finish = GRect(start.origin.x, 5, start.size.w, start.size.h);
  }

  // Schedule the animation
  Layer *time_layer = text_layer_get_layer(s_time_layer);
  PropertyAnimation *prop_anim = property_animation_create_layer_frame(time_layer, &start, &finish);
  Animation *anim = property_animation_get_animation(prop_anim);
  animation_set_curve(anim, AnimationCurveEaseOut);
  animation_set_duration(anim, ANIMATION_DURATION_IN_MS);
  animation_set_handlers(anim, (AnimationHandlers) {
    .stopped = anim_stopped_handler
  }, NULL);
  animation_schedule(anim);
}

static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
  schedule_animation();
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_time_layer = text_layer_create(GRect(
    bounds.origin.x, bounds.origin.y, bounds.size.w, TEXT_BOX_HEIGHT));
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  text_layer_set_font(s_time_layer,
    fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_BOLD_CONDENSED_SUBSET_40)));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));

  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
}

static void main_window_unload(Window *window) {
  text_layer_destroy(s_time_layer);
}

static void init() {
  // Start at the top
  s_move_direction_is_up = true;

  s_main_window = window_create();
  window_set_background_color(s_main_window, GColorBlack);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_stack_push(s_main_window, true);
}

static void deinit() {
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
