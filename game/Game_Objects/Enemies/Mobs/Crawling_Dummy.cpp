#include "Crawling_Dummy.hpp"
#include "../../../Screen.hpp"
#include "../../Player/Player.hpp"
#include "../../Projectiles/Bullet.hpp" 
#include <cmath>

void Crawling_Dummy::physics() {
    // 1. Погоня за игроком (через ссылку Screen::player)
    if (Screen::player) {
        double diff_x = Screen::player->pos_x - pos_x;
        double diff_y = Screen::player->pos_y - pos_y;
        double dist = std::sqrt(diff_x * diff_x + diff_y * diff_y);
        if (dist > 2.0) {
            pos_x += (diff_x / dist) * speed;
            pos_y += (diff_y / dist) * speed;
        }
    }

    // 2. Коллизия ТОЛЬКО с пулями
    for (auto obj : Screen::objects) {
        if (!obj->is_alive) continue;

        // Используем dynamic_cast, чтобы отличить пулю от игрока
        Bullet* b = dynamic_cast<Bullet*>(obj);
        if (b) {
            // Простая проверка: попала ли пуля (точка) в квадрат врага
            if (b->pos_x > pos_x && b->pos_x < pos_x + width &&
                b->pos_y > pos_y && b->pos_y < pos_y + height) {
                
                this->is_alive = false; // Враг умер
                b->is_alive = false;    // Пуля исчезла
                break;
            }
        }
    }
}
void Crawling_Dummy::render() {
    // Рисуем врага оранжевым прямоугольником
    al_draw_filled_rectangle(pos_x, pos_y, pos_x + width, pos_y + height, 
                             al_map_rgb(255, 150, 0));
}