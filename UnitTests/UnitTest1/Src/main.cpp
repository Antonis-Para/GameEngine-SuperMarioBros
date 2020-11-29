#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>

int main() {
    ALLEGRO_DISPLAY *display;
    ALLEGRO_EVENT_QUEUE *queue;
    ALLEGRO_BITMAP *bitmap = NULL;
    al_init();
    display = al_create_display(640, 480);
    queue = al_create_event_queue();
    al_install_keyboard();
    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_display_event_source(display));
    al_init_image_addon();
    bitmap = al_load_bitmap(".\\UnitTests\\UnitTest1\\Media\\Overworld_GrassBiome\\overworld_tileset_grass.png");
    assert(bitmap != NULL);
    bool running = true;
    float x = 0;
    int width = al_get_display_width(display);
    
    while (running) {
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
    al_destroy_bitmap(bitmap);
    return 0;
}
