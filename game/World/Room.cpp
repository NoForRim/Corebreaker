#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include "Room.hpp"

void Room::render() {
    for (int y = 0; y < 11; y++) {
        for (int x = 0; x < 20; x++) {
            float x1 = x * 32;
            float y1 = y * 32;
            float x2 = (x + 1) * 32;
            float y2 = (y + 1) * 32;

            int tile = tiles[y][x];

            if (tile == 1) { // СТЕНА
                al_draw_filled_rectangle(x1, y1, x2, y2, al_map_rgb(45, 45, 50));
                al_draw_rectangle(x1, y1, x2, y2, al_map_rgb(70, 70, 80), 1.0);
            } 
            else if (tile == 2) { // ДВЕРЬ
                al_draw_filled_rectangle(x1, y1, x2, y2, al_map_rgb(200, 150, 50));
                al_draw_rectangle(x1 + 4, y1 + 4, x2 - 4, y2 - 4, al_map_rgb(255, 220, 100), 2.0);
            }
            else if (tile == 3) { // ПРОПАСТЬ (PIT)
                // Рисуем глубокую черную дыру
                al_draw_filled_rectangle(x1, y1, x2, y2, al_map_rgb(10, 10, 15));
                // Можно добавить легкий градиент по краям для эффекта глубины
                al_draw_rectangle(x1 + 2, y1 + 2, x2 - 2, y2 - 2, al_map_rgb(20, 20, 30), 1.0);
            }
            else if (tile == 4 || tile == 5) { // ПЬЕДЕСТАЛЫ (4 - Магазин, 5 - Сокровищница)
                // Основание пьедестала
                al_draw_filled_rectangle(x1 + 4, y1 + 4, x2 - 4, y2 - 4, al_map_rgb(100, 100, 110));
                // Верхняя крышка (разные цвета для типа)
                ALLEGRO_COLOR top_color = (tile == 5) ? al_map_rgb(255, 215, 0) : al_map_rgb(150, 150, 160);
                al_draw_filled_rectangle(x1 + 6, y1 + 6, x2 - 6, y2 - 10, top_color);
            }
            else { // ПОЛ (0)
                al_draw_pixel(x1 + 16, y1 + 16, al_map_rgb(30, 30, 35));
            }
        }
    }
}

bool Room::is_wall(float x, float y) {
    int gx = (int)(x / 32);
    int gy = (int)(y / 32);
    
    if (gx < 0 || gx >= 20 || gy < 0 || gy >= 11) return true;
    
    int tile = tiles[gy][gx];
    // Теперь стена (1), пропасть (3) и пьедесталы (4, 5) блокируют движение
    return (tile == 1 || tile == 3 || tile == 4 || tile == 5);
}