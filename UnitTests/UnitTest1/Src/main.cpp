#include "app.h"

static app::TileMap map;
extern app::Bitmap tiles;
extern app::Bitmap dpyBuffer;
extern app::Rect viewWin;
extern app::Rect displayArea;

int main() {

    app::App app;

    app.Run();


    /*ALLEGRO_DISPLAY* display;
    ALLEGRO_EVENT_QUEUE* queue;
    bool scrollEnabled = false;
    int mouse_x = 0, mouse_y = 0, prev_mouse_x = 0, prev_mouse_y = 0;
    ALLEGRO_MOUSE_STATE mouse_state;
    viewWin = app::Rect{ 0, 0, 640, 480 };
    displayArea = app::Rect{ 0, 0, 640, 480 };
    ALLEGRO_EVENT event;

    if (!al_init()) {
        std::cout << "ERROR: Could not init allegro\n";
        return -1;
    }
    display = al_create_display(displayArea.w, displayArea.h);

    queue = al_create_event_queue();
    al_install_keyboard();
    al_install_mouse();
    al_register_event_source(queue, al_get_mouse_event_source());
    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_display_event_source(display));
    al_init_image_addon();
    
    tiles = app::BitmapLoad(".\\hy-454-super-mario\\UnitTests\\UnitTest1\\Media\\Overworld_GrassBiome\\overworld_tileset_grass.png");
    assert(tiles != NULL);

    dpyBuffer = app::BitmapCreate(displayArea.w, displayArea.h);

    app::ReadTextMap(&map, ".\\hy-454-super-mario\\UnitTests\\UnitTest1\\Media\\Overworld_GrassBiome\\map1_Kachelebene 1.csv");
    float x = 0;
    while (true) {
        if (!al_is_event_queue_empty(queue)) {
            al_wait_for_event(queue, &event);
            if (event.type == ALLEGRO_EVENT_MOUSE_AXES) {
                al_get_mouse_cursor_position(&mouse_x, &mouse_y);
                al_get_mouse_state(&mouse_state);
                if (mouse_state.buttons & 1) {
                    app::ScrollWithBoundsCheck(&viewWin, prev_mouse_x - mouse_x, prev_mouse_y - mouse_y);
                }
                prev_mouse_x = mouse_x;
                prev_mouse_y = mouse_y;
            }
            if (event.type == ALLEGRO_EVENT_KEY_DOWN && event.keyboard.keycode == ALLEGRO_KEY_HOME) {
                app::setToStartOfMap(&viewWin);
            }
            else if (event.type == ALLEGRO_EVENT_KEY_DOWN && event.keyboard.keycode == ALLEGRO_KEY_END) {
                app::ScrollWithBoundsCheck(&viewWin, app::GetMapPixelWidth(), app::GetMapPixelHeight());
            }
        }
        app::TileTerrainDisplay(&map, al_get_backbuffer(display), viewWin, displayArea);

        al_flip_display();
    }
    */
    return 0;
}
