#include "Bullet.hpp"
#include <allegro5/allegro_primitives.h>
#include <cmath>

Bullet::Bullet(double x, double y, double tx, double ty) : Game_Object(x, y) {
    // Вычисляем вектор направления от игрока к мышке
    double diff_x = tx - x;
    double diff_y = ty - y;
    double distance = std::sqrt(diff_x * diff_x + diff_y * diff_y);

    // Нормализуем вектор (делаем длину 1) и умножаем на скорость
    dx = (diff_x / distance) * speed;
    dy = (diff_y / distance) * speed;
}

void Bullet::physics() {
    pos_x += dx;
    pos_y += dy;

    // Простая проверка выхода за границы (чтобы не перегружать память)
    if (pos_x < 0 || pos_x > 640 || pos_y < 0 || pos_y > 360) {
        is_alive = false;
    }
}

void Bullet::render() {
    al_draw_filled_circle(pos_x, pos_y, 3, al_map_rgb(255, 255, 0)); // Желтая точка
}