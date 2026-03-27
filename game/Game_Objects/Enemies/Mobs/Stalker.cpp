#include "Stalker.hpp"
#include "../../../Screen.hpp"
#include "../../Player/Player.hpp"
#include <allegro5/allegro_primitives.h>
#include <cmath>

Stalker::Stalker(float x, float y) : Game_Object(x, y) {
    hp = 60.0f;
    attack_damage = 15.0f;
    width = 24;
    height = 24;
    is_enemy = true;
    state = HUNT;
    state_timer = 0;
}

void Stalker::take_damage(float damage, float kx, float ky) {
    this->hp -= damage;
    
    // Используем константу отброса
    this->pos_x += kx * KNOCKBACK_FORCE; 
    this->pos_y += ky * KNOCKBACK_FORCE;

    // Прерывание атаки при получении урона
    if (state == PREPARE || state == DASH) {
        state = COOLDOWN;
        state_timer = -0.2f; // Оглушение
    }
}

void Stalker::physics() {
    if (hp <= 0) {
        is_alive = false; 
        return; 
    }

    if (!Screen::player || !is_alive) return;

    float dx = Screen::player->pos_x - pos_x;
    float dy = Screen::player->pos_y - pos_y;
    float dist = std::sqrt(dx*dx + dy*dy);
    
    // Стандартный шаг времени для 60 FPS
    const float DT = 0.016f;
    state_timer += DT;

    // Потеря цели
    if (dist > DIST_LOST && state != HUNT) {
        state = HUNT;
    }

    switch (state) {
        case HUNT:
            // Движение к игроку
            pos_x += (dx / dist) * SPEED_HUNT; 
            pos_y += (dy / dist) * SPEED_HUNT;
            
            if (dist < DIST_AGGRO) { 
                state = PREPARE; 
                state_timer = 0; 
            }
            break;

        case PREPARE:
            // Подготовка (замирание)
            if (state_timer > TIME_PREPARE) { 
                state = DASH; 
                state_timer = 0; 
                angle = std::atan2(dy, dx); 
            }
            break;

        case DASH:
            // Рывок
            pos_x += std::cos(angle) * SPEED_DASH;
            pos_y += std::sin(angle) * SPEED_DASH;
            
            // Проверка попадания по игроку
            if (dist < DIST_HIT) {
                Screen::player->take_damage(attack_damage, std::cos(angle), std::sin(angle));
                state = COOLDOWN;
                state_timer = 0;
            }
            
            // Окончание рывка по времени
            if (state_timer > TIME_DASH) { 
                state = COOLDOWN; 
                state_timer = 0; 
            }
            break;

        case COOLDOWN:
            // Отдых
            if (state_timer > TIME_COOLDOWN) {
                state = HUNT;
            }
            break;
    }
}

void Stalker::render() {
    ALLEGRO_COLOR color_body;
    
    // Выбор цвета в зависимости от состояния
    if (state == PREPARE) 
        color_body = al_map_rgb(255, 200, 0); // Желтый — замах
    else if (state == DASH) 
        color_body = al_map_rgb(255, 255, 255); // Белый — рывок
    else 
        color_body = al_map_rgb(180, 40, 40); // Красный — обычное состояние

    al_draw_filled_rectangle(pos_x, pos_y, pos_x + width, pos_y + height, color_body);
    al_draw_rectangle(pos_x, pos_y, pos_x + width, pos_y + height, al_map_rgb(0,0,0), 1);
}