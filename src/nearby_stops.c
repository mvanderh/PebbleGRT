#include <pebble.h>
#include <Foundation/Foundation.h>
#include "nearby_stops.h"
#include "message_types.h"

#define NUM_STOPS 10

#define NEARBY_STOP_TITLE_MAX_LEN 128

#define LOADING_STRING "Loading.."

static struct {
    Window *window;
    SimpleMenuLayer *menu_layer;
    SimpleMenuSection nearby_stops_menu_section;
    char nearby_stops_items_titles[NUM_STOPS][NEARBY_STOP_TITLE_MAX_LEN];
    char nearby_stops_items_subtitles[NUM_STOPS][NEARBY_STOP_TITLE_MAX_LEN];
    SimpleMenuItem nearby_stops_items[NUM_STOPS];
} S;

static void send_phone_message_nearby_stops() {
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    dict_write_uint8(iter, PGKeyMessageType, (uint8_t) MessageTypeNearbyStopes);

    app_message_outbox_send();
}

static void window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect window_bounds = layer_get_bounds(window_layer);

    for (int i = 0; i < NUM_STOPS; ++i) {
        char *title = S.nearby_stops_items_titles[i];
        char *subtitle = S.nearby_stops_items_subtitles[i];
        subtitle[0] = '\0';
        strncpy(title, LOADING_STRING, strlen(LOADING_STRING));
        S.nearby_stops_items[i] = (SimpleMenuItem) {
            .title = title,
            .subtitle = subtitle
        };
    }

    S.nearby_stops_menu_section = (SimpleMenuSection) {
        .title = "Nearby Stops",
        .num_items = NUM_STOPS,
        .items = S.nearby_stops_items
    };

    S.menu_layer = simple_menu_layer_create(window_bounds,
        window,
        &S.nearby_stops_menu_section,
        1,
        NULL);

    layer_add_child(window_layer, simple_menu_layer_get_layer(S.menu_layer));

    send_phone_message_nearby_stops();
}

static void window_unload(Window *window) {
    simple_menu_layer_destroy(S.menu_layer);
}

void app_message_received(DictionaryIterator *iterator, void *context) {
    int32_t index = 0;
    char *name = NULL;
    char *distance = NULL;

    Tuple *t = dict_read_first(iterator);

    while (t != NULL) {
        switch (t->key) {
            case PGKeyMessageType:
                break;
            case PGKeyStopIndex:
                index = t->value->int32;
                break;
            case PGKeyStopName:
                name = t->value->cstring;
                break;
            case PGKeyStopDistance:
                distance = t->value->cstring;
                break;
            default:
                break;
                
        }
        t = dict_read_next(iterator);
    }

    strncpy(S.nearby_stops_items_titles[index], distance, NEARBY_STOP_TITLE_MAX_LEN);
    strncpy(S.nearby_stops_items_subtitles[index], name, NEARBY_STOP_TITLE_MAX_LEN);

    layer_mark_dirty(simple_menu_layer_get_layer(S.menu_layer));
}

void push_nearby_stops_window(int index, void *context) {
    S.window = window_create();

    window_set_window_handlers(S.window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload
    });

    app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
    app_message_register_inbox_received(app_message_received);

    window_stack_push(S.window, true);
}

