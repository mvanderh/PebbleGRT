#include <pebble.h>
#include "nearby_buses.h"

#define MAX_NUM_BUSES 10

#define NEARBY_BUS_TITLE_MAX_LEN 128

static struct {
   Window *window;
   SimpleMenuLayer *menu_layer;
   SimpleMenuSection nearby_buses_menu_section;
   char nearby_buses_items_titles[NEARBY_BUS_TITLE_MAX_LEN][10];
   SimpleMenuItem nearby_buses_items[MAX_NUM_BUSES];
} S;


static void window_load(Window *window) {
   Layer *window_layer = window_get_root_layer(window);
   GRect window_bounds = layer_get_bounds(window_layer);

   for (int i = 0; i < MAX_NUM_BUSES; ++i) {
      snprintf(S.nearby_buses_items_titles[i], NEARBY_BUS_TITLE_MAX_LEN, "Bus %d", i);
      S.nearby_buses_items[i] = (SimpleMenuItem) {
         .title = S.nearby_buses_items_titles[i]
      };
   }

   S.nearby_buses_menu_section = (SimpleMenuSection) {
      .title = "Nearby Buses",
      .num_items = MAX_NUM_BUSES,
      .items = S.nearby_buses_items
   };

   S.menu_layer = simple_menu_layer_create(window_bounds,
      window,
      &S.nearby_buses_menu_section,
      1,
      NULL);

   layer_add_child(window_layer, simple_menu_layer_get_layer(S.menu_layer));
}

static void window_unload(Window *window) {
   simple_menu_layer_destroy(S.menu_layer);
}

void push_nearby_buses_window(int index, void *context) {
   S.window = window_create();
   window_set_window_handlers(S.window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload
   });
   window_stack_push(S.window, true);
}

