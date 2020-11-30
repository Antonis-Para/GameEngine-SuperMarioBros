#include <iostream>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include "app.h"

static app::TileMap map;
extern app::Bitmap tiles;
extern app::Bitmap dpyBuffer;
extern app::Rect viewWin;
extern app::Rect displayArea;

int main() {
    ALLEGRO_DISPLAY* display;
    ALLEGRO_EVENT_QUEUE* queue;
    //ALLEGRO_BITMAP *bitmap = NULL;
    //app::TileMap m;
    bool scrollEnabled = false;
    int mouse_x = 0, mouse_y = 0, prev_mouse_x = 0, prev_mouse_y = 0;
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
                if (scrollEnabled) {
                    app::ScrollWithBoundsCheck(&viewWin, prev_mouse_x - mouse_x, prev_mouse_y - mouse_y); //Not ready yet but I got it :)
                }
                prev_mouse_x = mouse_x;
                prev_mouse_y = mouse_y;
            }
            if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && event.mouse.button == 2) {
                scrollEnabled = true;
            }
            else if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP && event.mouse.button == 2) {
                scrollEnabled = false;
            }
            else if (event.type == ALLEGRO_EVENT_KEY_DOWN && event.keyboard.keycode == ALLEGRO_KEY_HOME) {
                viewWin.x = 0;
                viewWin.y = 0;
            }
            else if (event.type == ALLEGRO_EVENT_KEY_DOWN && event.keyboard.keycode == ALLEGRO_KEY_END) {
                viewWin.x = viewWin.w - displayArea.w; //Not ready yet but I got it :)
                viewWin.y = viewWin.h - displayArea.h; //Not ready yet but I got it :)
            }
        }
        //al_clear_to_color(al_map_rgba_f(1, 1, 1, 1));
        app::TileTerrainDisplay(&map, al_get_backbuffer(display), viewWin, displayArea);
        //al_flip_display();

        //al_draw_bitmap(dpyBuffer, 200, 0, 0);
        al_flip_display();
    }




    //bool running = true;
    //float x = 0;
    //int width = al_get_display_width(display);
    
    /*while (running) {
        al_clear_to_color(al_map_rgba_f(1, 1, 1, 1));
        al_draw_bitmap(bitmap, x += 0.01, 0, 0);
        al_flip_display();
        if (x > width) x = -al_get_bitmap_width(bitmap);
        ALLEGRO_EVENT event;
        if (!al_is_event_queue_empty(queue)) {
            al_wait_for_event(queue, &event);
            if (event.type == ALLEGRO_EVENT_KEY_UP || event.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
                running = false;
        }
    }
    
    al_destroy_display(display);
    al_uninstall_keyboard();
    al_destroy_bitmap(bitmap);*/
        std::cin.get();
    return 0;
}
