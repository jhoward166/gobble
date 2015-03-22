#include <pebble.h>
  
Window *window;
#define NUM_PATHS 1
  
static Window *s_main_window;
static Layer *s_path_layer;
ActionBarLayer *action_bar;
static GBitmap *checkBitmap;
static GBitmap *clockwiseBitmap;
static GBitmap *counterClockwiseBitmap;
//static RotBitmapLayer *bitmap_layer;
static GPath *s_path_array[NUM_PATHS];
static GPath *s_current_path;
static GPath *hole_path;
static GPath *green_path;
static GPath *sand1_path;
static GPath *sand2_path;
static GPath *sand3_path;
static GPath *teebox_path;

int currentX;
int currentY;
int nextX;
int nextY;
int holeMinX = 80;
int holeMaxX = 85;
int holeMinY = 20;
int holeMaxY = 23;

static bool s_outline_mode;

static const GPathInfo BOLT_PATH_INFO = {
  22,
  (GPoint []) {
    {60, 130},
    {43, 117},
    {30, 100},
    {23, 80},
    {20, 60},  
    {25, 30},
    {27, 25},
    {30, 20},
    {35, 10},
    {40, 6}, 
    {60, 0},
    {80, 3},
    {90, 10},
    {100, 20},
    {102, 35},
    {100, 50},
    {93, 60},
    {70, 70},
    {72, 90}, 
    {80, 115},
    {77, 125},
    {70, 130}
  }
};

static const GPathInfo GREEN = {
  12,
  (GPoint []) {
    {80, 15},
    {76, 17},
    {72, 21},
    {70, 25},
    {72, 29},
    {76, 33},
    {80, 35},
    {84, 33},
    {88, 31},
    {90, 25},
    {88, 21},
    {84, 17}
    
  }
};

static const GPathInfo HOLE = {
  4,
  (GPoint []) {
    {80, 23},
    {85, 23},
    {85, 20},
    {80, 20}
  }
};

static const GPathInfo SAND1 = {
  10,
  (GPoint []) {
    {67,20},
    {83, 15},
    
    {85, 14},
    {85, 11},
    
    {83, 10},
      
    {76, 6},
    {74, 6},
      
    {67,10}, 
    
    {65, 11},
    {65, 14}
  }
};
static const GPathInfo SAND2 = {
  10,
  (GPoint []) {
    {67,65},
    {93, 55},
    {95, 54},
    {95, 51},
    {93, 50},
    {81, 44},
    {79, 44},
    {67,50}, 
    {65, 51},
    {65, 64}
  }
};
  
  static const GPathInfo SAND3 = {
  8,
  (GPoint []) {
    {32,50},
    {30,52},
    {30,78},
    {32,80},
    {38,80},
    {40,78},
    {40,52},
    {38,50} 
  }
};

static const GPathInfo TEEBOX = {
  4,
  (GPoint []) {
    {60, 125},
    {60, 120},
    {70, 120},
    {70, 125} 
  }
};

bool victoryCondition(){
  if(
    (nextX <= holeMaxX && nextX >= holeMinX) 
    &&
    (nextY <= holeMaxY && nextY >= holeMinY)
  ){
    return true;
  }else if(
    (
      (currentX <= holeMinX && nextX >= holeMaxX)
      ||
      (currentX >= holeMaxX && nextX <= holeMinX)
    )&&(
      (currentY <= holeMinY && nextY >= holeMaxY)
      ||
      (currentY >= holeMaxY && nextY <= holeMaxY)
    )
  ){
    return true;
  }else{
    return false;
  }
}

int pythagCeiling(int deltaX, int deltaY){
  int i = 1;
  int cSqrd = ((deltaX*deltaX)+(deltaY*deltaY));
  while((i*i) < cSqrd){
    if(i > cSqrd || i<1){
      return -1;
    }
    i++;
  }
  return i;
}

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
    graphics_context_set_fill_color(ctx, GColorJaegerGreen);
    gpath_draw_filled(ctx, s_current_path);
    graphics_context_set_fill_color(ctx, GColorDarkGreen);
    gpath_draw_filled(ctx, green_path);
    graphics_context_set_fill_color(ctx, GColorWhite);
    gpath_draw_filled(ctx, hole_path);
    graphics_context_set_fill_color(ctx, GColorChromeYellow);
    gpath_draw_filled(ctx, sand1_path);
    graphics_context_set_fill_color(ctx, GColorChromeYellow);
    gpath_draw_filled(ctx, sand2_path);
    graphics_context_set_fill_color(ctx, GColorChromeYellow);
    gpath_draw_filled(ctx, sand3_path);
    graphics_context_set_fill_color(ctx, GColorDarkGray);
    gpath_draw_filled(ctx, teebox_path);
    //rot_bitmap_set_compositing_mode(bitmap_layer, GCompOpSet);
  }
}

/*void click_config_provider() {
  window_single_click_subscribe(BUTTON_ID_DOWN, (ClickHandler) void);
  window_single_click_subscribe(BUTTON_ID_UP, (ClickHandler) void);
}*/

void window_load(Window *window){
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);
  action_bar = action_bar_layer_create();
  s_path_layer = layer_create(bounds);
  layer_set_update_proc(s_path_layer, path_layer_update_callback);
  layer_add_child(window_layer, s_path_layer);
  action_bar_layer_set_background_color(action_bar, GColorWhite);
  counterClockwiseBitmap = gbitmap_create_with_resource(RESOURCE_ID_COUNTER_CLOCKWISE);
  clockwiseBitmap = gbitmap_create_with_resource(RESOURCE_ID_CLOCKWISE);
  checkBitmap = gbitmap_create_with_resource(RESOURCE_ID_CHECK_MARK);
  //action_bar_layer_set_click_config_provider(action, click_config_provider());
  action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, counterClockwiseBitmap);
  action_bar_layer_set_icon(action_bar, BUTTON_ID_SELECT , checkBitmap);
  action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, clockwiseBitmap);
  action_bar_layer_add_to_window(action_bar, window);
  //layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer((BitmapLayer *)bitmap_layer));

  // Move all paths to the center of the screen
  /*for (int i = 0; i < NUM_PATHS; i++) {
    gpath_move_to(s_path_array[i], GPoint(bounds.size.w/2, bounds.size.h/2));
  }
  */
}

void window_unload(Window *window){
  //rot_bitmap_layer_destroy(bitmap_layer);
  action_bar_layer_destroy(action_bar);
  layer_destroy(s_path_layer);
}

void handle_init(void){
  s_current_path = gpath_create(&BOLT_PATH_INFO);
  green_path = gpath_create(&GREEN);
  hole_path = gpath_create(&HOLE);
  sand1_path = gpath_create(&SAND1);
  sand2_path = gpath_create(&SAND2);
  sand3_path = gpath_create(&SAND3);
  teebox_path = gpath_create(&TEEBOX);
  
  //bitMap = gbitmap_create_with_resource(RESOURCE_ID_IRON_FIVE);
  //bitmap_layer = bitmap_layer_create(GRect(0, 0, 144, 156));
  //rot_bitmap_layer_create(bitMap);
  //bitmap_layer_set_background_color(bitmap_layer, GColorClear);
  
  
  
  s_path_array[0] = s_current_path;
  
  s_main_window = window_create();
  window_set_background_color(s_main_window, GColorArmyGreen);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_main_window, true);
}

void handle_deinit(void){
  window_destroy(s_main_window);

  gpath_destroy(s_current_path);
  gpath_destroy(green_path);
  gpath_destroy(hole_path);
  gpath_destroy(sand1_path);
  gpath_destroy(sand2_path);
  gpath_destroy(sand3_path);
  gpath_destroy(teebox_path);
  
  //rot_bitmap_layer_destroy(bitmap_layer);
 
}

int main(void){
  handle_init();
  app_event_loop();
  handle_deinit();
}