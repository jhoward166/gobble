#include <pebble.h>
  
Window *window;
#define NUM_PATHS 1
  
static Window *s_main_window;
static Layer *s_path_layer;

static GPath *s_path_array[NUM_PATHS];
static GPath *s_current_path;

static bool s_outline_mode;

static const GPathInfo BOLT_PATH_INFO = {
  22,
  (GPoint []) {
    {0, 70},
    {-17, 57},
    {-30, 40},
    {-37, 20},
    {-40, 0},  //5
    {-35, -30},
    {-33, -35},
    {-30, -40},
    {-25, -50},
    {-20, -54}, //10
    {0, -60},
    {20, -57},//15
    {30, -50},
    {40, -40},
    {42, -25},
    {40, -10},
    {33, 0},
    {10, 10},
    {12, 30}, //20
    {20, 55},
    {17, 65},
    {10, 70}
  }
};

static void path_layer_update_callback(Layer *layer, GContext *ctx) {
  // You can rotate the path before rendering
  //gpath_rotate_to(s_current_path, (TRIG_MAX_ANGLE / 360) * s_path_angle);

  // There are two ways you can draw a GPath: outline or filled
  // In this example, only one or the other is drawn, but you can draw
  // multiple instances of the same path filled or outline.
  if (s_outline_mode) {
    // draw outline uses the stroke color
    graphics_context_set_stroke_color(ctx, GColorWhite);
    gpath_draw_outline(ctx, s_current_path);
  } else {
    // draw filled uses the fill color
    graphics_context_set_fill_color(ctx, GColorLimerick);
    gpath_draw_filled(ctx, s_current_path);
  }
}

void window_load(Window *window){
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);

  s_path_layer = layer_create(bounds);
  layer_set_update_proc(s_path_layer, path_layer_update_callback);
  layer_add_child(window_layer, s_path_layer);

  // Move all paths to the center of the screen
  for (int i = 0; i < NUM_PATHS; i++) {
    gpath_move_to(s_path_array[i], GPoint(bounds.size.w/2, bounds.size.h/2));
  }
}

void window_unload(Window *window){
  layer_destroy(s_path_layer);
}

void handle_init(void){
  s_current_path = gpath_create(&BOLT_PATH_INFO);
  
  s_path_array[0] = s_current_path;
  
  s_main_window = window_create();
  window_set_background_color(s_main_window, GColorBlack);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_main_window, true);
}

void handle_deinit(void){
  window_destroy(s_main_window);

  gpath_destroy(s_current_path);
}

int main(void){
  handle_init();
  app_event_loop();
  handle_deinit();
}