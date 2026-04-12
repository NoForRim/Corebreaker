#include "Bullet.hpp"
#include "../../Screen.hpp"
#include "../Player/Player.hpp"
#include "../Weapon_System/Combat_System.hpp"
#include <allegro5/allegro_primitives.h>
#include <cmath>

Bullet::Bullet(float x, float y, float tx, float ty, BulletProps props) : Game_Object(x, y) {
    this->damage = props.damage;
    this->penetration = props.penetration;
    this->type = props.type;
    this->size   = props.size;
    this->color  = props.color;
    
    // Хитбокс на основе радиуса
    this->width  = props.size * 2;
    this->height = props.size * 2;

    float dx_dist = tx - x;
    float dy_dist = ty - y;
    float dist = std::sqrt(dx_dist * dx_dist + dy_dist * dy_dist);

    if (dist > 0) {
        this->vel_x = (dx_dist / dist) * props.speed;
        this->vel_y = (dy_dist / dist) * props.speed;
    } else {
        this->vel_x = props.speed;
        this->vel_y = 0;
    }
}

void Bullet::physics() {
    pos_x += vel_x;
    pos_y += vel_y;

    // Удаление за границами
    if (pos_x < -50 || pos_x > 700 || pos_y < -50 || pos_y > 410) {
        is_alive = false;
        return;
    }

    for (auto obj : Screen::objects) {
        if (obj && obj->is_enemy && obj->is_alive) {
            // Проверка коллизии AABB (пересечение прямоугольников)
            if (pos_x < obj->pos_x + obj->width && pos_x + width > obj->pos_x &&
                pos_y < obj->pos_y + obj->height && pos_y + height > obj->pos_y) 
            {
                float speed_total = std::sqrt(vel_x * vel_x + vel_y * vel_y);
                
                Damage_Event ev;
                ev.attacker = (Game_Object*)Screen::player;
                ev.victim = obj;
                ev.damage = this->damage;
                ev.penetration = this->penetration; // ДОБАВИТЬ ЭТО
                ev.type = this->type;               // И ЭТО
                ev.kx = (speed_total > 0) ? vel_x / speed_total : 0;
                ev.ky = (speed_total > 0) ? vel_y / speed_total : 0;
                ev.is_projectile = true;

                Combat_System::handle_attack(ev);

                this->is_alive = false; 
                break; 
            }
        }
    }
}

void Bullet::render() {
    al_draw_filled_circle(pos_x + size, pos_y + size, size, color);
}