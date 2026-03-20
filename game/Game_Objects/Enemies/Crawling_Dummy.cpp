#include "Crawling_Dummy.hpp"
#include "../../World.hpp"
#include "../Player/Player.hpp" // Чтобы знать, где игрок
#include <cmath>

void Crawling_Dummy::physics() {
    // 1. Ищем игрока в списке объектов
    Game_Object* player = nullptr;
    for (auto obj : Screen::objects) {
        // Предположим, первый объект в списке — всегда игрок, 
        // или проверим по размеру (Player обычно 32x32)
        if (obj->width == 32) { 
            player = obj;
            break;
        }
    }

    // 2. Если нашли игрока — ползем к нему
    if (player) {
        double diff_x = player->pos_x - pos_x;
        double diff_y = player->pos_y - pos_y;
        double distance = std::sqrt(diff_x * diff_x + diff_y * diff_y);

        if (distance > 1.0) { // Чтобы не "дрожал", когда стоит вплотную
            pos_x += (diff_x / distance) * speed;
            pos_y += (diff_y / distance) * speed;
        }
    }

    // 3. Проверка попадания пули (Коллизия)
    for (auto obj : Screen::objects) {
        if (obj->is_alive && obj->width < 10) { // Если это пуля
            if (obj->pos_x > pos_x && obj->pos_x < pos_x + width &&
                obj->pos_y > pos_y && obj->pos_y < pos_y + height) {
                
                this->is_alive = false;
                obj->is_alive = false;
            }
        }
    }
}

void Crawling_Dummy::render() {
    // Отрисуем его оранжевым, чтобы отличать от обычных Dummy
    al_draw_filled_rectangle(pos_x, pos_y, pos_x + width, pos_y + height, 
                             al_map_rgb(255, 150, 0)); 
}