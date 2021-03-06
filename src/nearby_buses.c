#include <pebble.h>
#include "message_types.h"
#include "bus_detail.h"
#include "pgbus.h"

#define NUM_INFO_MENU_ITEMS 6
static const int REFRESH_INTERVAL = 27;

static Window *s_window;

static SimpleMenuItem s_menu_items[NUM_INFO_MENU_ITEMS];
static struct PGBus *s_buses[NUM_INFO_MENU_ITEMS] = {NULL};

static SimpleMenuSection s_default_menu_section = {
    .items = s_menu_items,
    .num_items = NUM_INFO_MENU_ITEMS,
    .title = "Nearby Buses"
};

static SimpleMenuLayer *s_nearby_buses_layer;

static void send_report_nearby_buses_msg()
{
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    dict_write_uint8(iter, PGKeyMessageType, (uint8_t) PGMessageTypeReportNearbyBuses);

    app_message_outbox_send();
}

static void window_load(Window *window)
{
    s_menu_items[0] = (SimpleMenuItem) {
        .title = "Loading.."
    };

    Layer *window_layer = window_get_root_layer(window);
    GRect window_bounds = layer_get_bounds(window_layer);
    s_nearby_buses_layer = simple_menu_layer_create(window_bounds,
        window,
        &s_default_menu_section,
        1,
        NULL);
    layer_add_child(window_layer, simple_menu_layer_get_layer(s_nearby_buses_layer));
    send_report_nearby_buses_msg();
}

static void window_unload(Window *window)
{
    simple_menu_layer_destroy(s_nearby_buses_layer);
    window_destroy(window);
    app_message_deregister_callbacks();
    tick_timer_service_unsubscribe();

    for (int i = 0; i < NUM_INFO_MENU_ITEMS; i++) {
        if (s_buses[i] != NULL) {
            pgbus_destroy(s_buses[i]);
            s_buses[i] = NULL;
        }
        s_menu_items[i] = (SimpleMenuItem) {
            .title = NULL,
            .subtitle = NULL,
            .icon = NULL,
            .callback = NULL,
        };
    }
}


static void tick_handler(struct tm *tick_time, TimeUnits units_changed)
{
    if (tick_time->tm_sec % REFRESH_INTERVAL == 0) {
        send_report_nearby_buses_msg();
    }
}

static void menu_item_clicked(int index, void *context)
{
    tick_timer_service_unsubscribe();
    app_message_deregister_callbacks();

    Window *bus_detail_window = create_bus_detail_window(s_buses[index]);

    window_stack_push(bus_detail_window, true);
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context)
{
    Tuple *t = dict_read_first(iterator);

    while (t != NULL) {
        switch (t->key) {
            case PGKeyMessageType:
                break;
            default: {
                int index = t->key - 1;
                struct PGBus **bus = &s_buses[index];
                if (*bus != NULL) {
                    pgbus_destroy(*bus);
                }
                *bus = pgbus_parse_from_string(t->value->cstring);
                SimpleMenuItem bus_item = {
                    .title = (*bus)->distance,
                    .subtitle = (*bus)->description,
                    .icon = NULL,
                    .callback = menu_item_clicked
                };
                s_menu_items[index] = bus_item;
                break;
            }
        }

        Tuple *next = dict_read_next(iterator);
        if (t == next) {
            break;
        }
        t = next;
    }

    layer_mark_dirty(simple_menu_layer_get_layer(s_nearby_buses_layer));
}

static void inbox_dropped_callback(AppMessageResult reason, void *context)
{
    APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context)
{
    APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context)
{
    APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}


Window *create_nearby_buses_window()
{
    s_window = window_create();

    window_set_window_handlers(s_window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload
    });

    tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
    app_message_register_inbox_received(inbox_received_callback);
    app_message_register_inbox_dropped(inbox_dropped_callback);
    app_message_register_outbox_failed(outbox_failed_callback);
    app_message_register_outbox_sent(outbox_sent_callback);

    return s_window;
}
