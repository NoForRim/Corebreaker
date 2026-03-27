#include "Player.hpp"
#include <allegro5/allegro_primitives.h>
#include <allegro5/keyboard.h>
#include <allegro5/mouse.h>
#include <string>
#include "../Projectiles/Bullet.hpp" 
#include "../../World/Room.hpp" 
#include "../../Screen.hpp"

Player::Player(double x, double y) : Game_Object(x, y) {
    pos_x = x;
    pos_y = y;
    width = 20;
    height = 20;
    
    // Инициализация HP и урона
    this->hp = PLAYER_MAX_HP;
    this->current_damage = DEFAULT_DAMAGE; 
    this->fire_cooldown = 0;
}

void Player::update_hp_cache(ALLEGRO_FONT* font) {
    if (!font) return;

    // ОПРЕДЕЛЯЕМ РАЗМЕРЫ
    int b_width = 120;
    int b_height = 20;

    // Если битмапа еще нет — создаем. Если есть — используем старый (так быстрее)
    if (!hp_cache) {
        hp_cache = al_create_bitmap(b_width, b_height);
    }

    if (!hp_cache) return; // Если всё еще null — значит графика не инициализирована

    ALLEGRO_BITMAP* prev_target = al_get_target_bitmap();
    al_set_target_bitmap(hp_cache);
    
    // Очищаем прозрачным цветом
    al_clear_to_color(al_map_rgba(0, 0, 0, 0)); 

    char buf[32];
    snprintf(buf, sizeof(buf), "HP: %d / %d", (int)hp, (int)max_hp);
    
    // Рисуем текст в начало битмапа
    al_draw_text(font, al_map_rgb(255, 255, 255), 0, 0, 0, buf);

    al_set_target_bitmap(prev_target);
}

void Player::move(double move_x, double move_y, Room &room) {
    // 1. Проверка по горизонтали (X)
    float next_x = pos_x + move_x;
    if (!room.is_wall(next_x, pos_y) && 
        !room.is_wall(next_x + width, pos_y) && 
        !room.is_wall(next_x, pos_y + height) && 
        !room.is_wall(next_x + width, pos_y + height)) 
    {
        pos_x = next_x;
    }

    // 2. Проверка по вертикали (Y)
    float next_y = pos_y + move_y;
    if (!room.is_wall(pos_x, next_y) && 
        !room.is_wall(pos_x + width, next_y) && 
        !room.is_wall(pos_x, next_y + height) && 
        !room.is_wall(pos_x + width, next_y + height)) 
    {
        pos_y = next_y;
    }

    // Ограничение экраном
    if (pos_x < 0) pos_x = 0;
    if (pos_y < 0) pos_y = 0;
    if (pos_x > 640 - width) pos_x = 640 - width;
    if (pos_y > 360 - height) pos_y = 360 - height;
}

void Player::take_damage(float damage, float kx, float ky) {
    // 1. Проверка на неуязвимость (I-frames)
    if (this->invul_timer > 0) return;

    printf("DAMAGE RECEIVED: %f | CURRENT HP BEFORE: %f\n", damage, this->hp);
    // 2. Наносим ВЕСЬ урон сразу
    this->hp -= damage;
    if (this->hp < 0) this->hp = 0;

    // 3. Устанавливаем Rally (оранжевое здоровье)
    // Весь полученный урон теперь можно "отбить"
    this->recoverable_hp = damage; 

    // 4. Таймеры
    this->invul_timer = 0.6f; // Неуязвимость на полсекунды
    this->rally_timer = 2.0f; // Оранжевое здоровье не тает 2 секунды

    // 5. Отброс игрока (Knockback)
    this->vel_x = kx * 8.0f;
    this->vel_y = ky * 8.0f;

    // 6. Обновляем текстовый кэш HP
    this->hp_needs_update = true;
}

void Player::physics(Room &room) {
    ALLEGRO_KEYBOARD_STATE key_state;
    al_get_keyboard_state(&key_state);


    // 1. Уменьшаем таймер неуязвимости
    if (invul_timer > 0) invul_timer -= 0.016f;

    // 2. Логика Rally (оранжевого здоровья)
    if (rally_timer > 0) {
        rally_timer -= 0.016f; // Ждем, пока игрок может вернуть HP
    } else {
        // Если время вышло, оранжевое здоровье начинает быстро исчезать
        if (recoverable_hp > 0) {
            recoverable_hp -= 0.5f; // Скорость "таяния"
            if (recoverable_hp < 0) recoverable_hp = 0;

            this->hp_needs_update = true;
        }
    }

    // Ускорение
    double acc = 0.5; 
    if (al_key_down(&key_state, ALLEGRO_KEY_W)) vel_y -= acc;
    if (al_key_down(&key_state, ALLEGRO_KEY_S)) vel_y += acc;
    if (al_key_down(&key_state, ALLEGRO_KEY_A)) vel_x -= acc;
    if (al_key_down(&key_state, ALLEGRO_KEY_D)) vel_x += acc;

    // Трение
    double friction = 0.85; 
    vel_x *= friction;
    vel_y *= friction;

    // Максимальная скорость
    double max_speed = 4.5;
    if (vel_x > max_speed)  vel_x = max_speed;
    if (vel_x < -max_speed) vel_x = -max_speed;
    if (vel_y > max_speed)  vel_y = max_speed;
    if (vel_y < -max_speed) vel_y = -max_speed;

    // Передаем комнату в move
    move(vel_x, vel_y, room);

    // Стрельба
    ALLEGRO_MOUSE_STATE mouse;
    al_get_mouse_state(&mouse);

    if (fire_cooldown > 0) fire_cooldown--;

    if ((mouse.buttons & 1) && fire_cooldown <= 0) {
        float dw = (float)al_get_display_width(al_get_current_display());
        float dh = (float)al_get_display_height(al_get_current_display());
        float sw = dw / 640.0;
        float sh = dh / 360.0;
        float scale = (sw < sh) ? sw : sh;

        float tx = mouse.x / scale;
        float ty = mouse.y / scale;

        Screen::spawn(new Bullet(pos_x + width/2, pos_y + height/2, tx, ty, current_damage));
        
        fire_cooldown = FIRE_COOLDOWN;
    }
}

void Player::render() {
    al_draw_filled_rectangle(pos_x, pos_y, pos_x + width, pos_y + height, 
                             al_map_rgb(0, 255, 100));
}