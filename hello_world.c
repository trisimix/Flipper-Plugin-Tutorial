#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>
#include <stdlib.h>

typedef struct {
    EventType type;
    InputEvent input;
} PluginEvent;

typedef enum {
    EventTypeTick,
    EventTypeKey,
} EventType;

typedef struct {
    int x;
    int y;
} PluginState; 


static void render_callback(Canvas* const canvas, void* ctx) {
    canvas_draw_frame(canvas, 0, 0, 128, 64);
}

static void input_callback(InputEvent* input_event, osMessageQueueId_t event_queue) {
    furi_assert(event_queue); 

    PluginEvent event = {.type = EventTypeKey, .input = *input_event};
    osMessageQueuePut(event_queue, &event, 0, osWaitForever);
}

typedef struct { 
    int x; 
    int y;
} PluginState; 

static void hello_world_state_init(PluginState* const plugin_state) {
    plugin_state->x = 10; 
    plugin_state->y = 10
} 

int32_t hello_world_app(void* p) { 
    osMessageQueueId_t event_queue = osMessageQueueNew(8, sizeof(PluginEvent), NULL); 
    
    PluginState* plugin_state = malloc(sizeof(PluginState));
    hello_world_state_init(plugin_state);
    ValueMutex state_mutex; 
    if (!init_mutex(&state_mutex, plugin_state, sizeof(PluginState))) {
        FURI_LOG_E(TAG, "cannot create mutex\r\n");
        free(game_state); 
        return 255;
    }

    // Set system callbacks
    ViewPort* view_port = view_port_alloc(); 
    view_port_draw_callback_set(view_port, render_callback, &state_mutex);
    view_port_input_callback_set(view_port, input_callback, event_queue);
 
    // Open GUI and register view_port
    Gui* gui = furi_record_open("gui"); 
    gui_add_view_port(gui, view_port, GuiLayerFullscreen); 

   
    PluginEvent event; 
    for(bool processing = true; processing;) { 
        osStatus_t event_status = osMessageQueueGet(event_queue, &event, NULL, 100);
        PluginState* game_state = (PluginState*)acquire_mutex_block(&state_mutex);

        if(event_status == osOK) {
            // press events
            if(event.type == EventTypeKey) {
                if(event.input.type == InputTypePress) {  
                    switch(event.input.key) {
                    case InputKeyUp: 
                            game_state->bird.point.y++;
                        break; 
                    case InputKeyDown: 
                            game_state->bird.point.y--;
                        break; 
                    case InputKeyRight: 
                            game_state->bird.point.x++;
                        break; 
                    case InputKeyLeft:  
                            game_state->bird.point.y--;
                        break; 
                    case InputKeyOk: 
                    case InputKeyBack: 
                        processing = false;
                        break;
                    }
                }
            } 
        } else {
            FURI_LOG_D(TAG, "osMessageQueue: event timeout");
            // event timeout
        }

        view_port_update(view_port);
        release_mutex(&state_mutex, game_state);
    }

    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    furi_record_close("gui");
    view_port_free(view_port);
    osMessageQueueDelete(event_queue); 

    return 0;
}