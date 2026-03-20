#include "Dummy.hpp"
#include "../../World.hpp" // Чтобы видеть все объекты (пули)

void Dummy::physics() {
    // Проверяем все объекты в мире
    for (auto obj : Screen::objects) {
        // Если объект — это не мы сами и он жив
        if (obj != this && obj->is_alive) {
            
            // Простейшая проверка: находится ли центр пули внутри нашего квадрата
            // (Для начала этого хватит, позже сделаем полноценный хитбокс)
            if (obj->pos_x > pos_x && obj->pos_x < pos_x + width &&
                obj->pos_y > pos_y && obj->pos_y < pos_y + height) {
                
                // ПОПАДАНИЕ!
                this->is_alive = false; // Враг исчезает
                obj->is_alive = false;  // Пуля исчезает
            }
        }
    }
}

void Dummy::render() {
    al_draw_filled_rectangle(pos_x, pos_y, pos_x + width, pos_y + height, 
                             al_map_rgb(255, 50, 50)); // Красный квадрат
}