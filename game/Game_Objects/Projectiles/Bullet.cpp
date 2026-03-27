#include "Bullet.hpp"
#include "../../Screen.hpp"
#include "../Player/Player.hpp"
#include <allegro5/allegro_primitives.h>
#include <cmath>

Bullet::Bullet(float x, float y, float tx, float ty, float dmg) : Game_Object(x, y) {
    this->damage = dmg;
    this->width = 4;
    this->height = 4;

    float dx_dist = tx - x;
    float dy_dist = ty - y;
    float dist = std::sqrt(dx_dist * dx_dist + dy_dist * dy_dist);
    float b_speed = 8.0f; // Можно вынести в константу

    if (dist > 0) {
        this->vel_x = (dx_dist / dist) * b_speed;
        this->vel_y = (dy_dist / dist) * b_speed;
    } else {
        this->vel_x = b_speed;
        this->vel_y = 0;
    }
}

void Bullet::physics() {
    pos_x += vel_x;
    pos_y += vel_y;

    // Удаление за экраном
    if (pos_x < -100 || pos_x > 800 || pos_y < -100 || pos_y > 500) {
        is_alive = false;
        return;
    }

    // Проверка столкновений
    for (auto obj : Screen::objects) {
        if (obj && obj->is_enemy && obj->is_alive) {
            if (pos_x >= obj->pos_x && pos_x <= obj->pos_x + obj->width &&
                pos_y >= obj->pos_y && pos_y <= obj->pos_y + obj->height) 
            {
                // Считаем направление отброса
                float kx = (vel_x > 0) ? 1.0f : -1.0f;
                float ky = (vel_y > 0) ? 1.0f : -1.0f;

                obj->take_damage(this->damage, kx, ky);
                if (Screen::player && Screen::player->recoverable_hp > 0) {
                    // Сколько лечим за одно попадание. 
                    // Можно сделать зависимым от урона пули, например: heal = damage * 0.25f;
                    float heal = 3.0f; 
                    
                    if (Screen::player->recoverable_hp < heal) {
                        heal = Screen::player->recoverable_hp;
                    }

                    Screen::player->hp += heal;
                    Screen::player->recoverable_hp -= heal;
                    
                    // Ограничитель, чтобы не вылезти за MAX_HP
                    if (Screen::player->hp > Screen::player->PLAYER_MAX_HP) {
                        Screen::player->hp = Screen::player->PLAYER_MAX_HP;
                    }
                }
                this->is_alive = false; 
                break; 
            }
        }
    }
}

void Bullet::render() {
    al_draw_filled_circle(pos_x, pos_y, 3, al_map_rgb(255, 255, 0));
}