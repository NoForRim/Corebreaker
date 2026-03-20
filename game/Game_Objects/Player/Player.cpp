#include "Player.hpp"
#include <allegro5/allegro_primitives.h>
#include <allegro5/keyboard.h>
#include <allegro5/mouse.h>
#include "../Projectiles/Bullet.hpp" 
#include "../../world.hpp" 

// Переменная для задержки стрельбы
int fire_cooldown = 0;

Player::Player(double x, double y) : Game_Object(x, y) {
    pos_x = x;
    pos_y = y;
    width = 32; 
    height = 32;
    speed = 2.5; 
}

void Player::move(double move_x, double move_y) {
    pos_x += move_x;
    pos_y += move_y;

    // Ограничение движения рамками экрана (WIDTH=640, HEIGHT=360)
    if (pos_x < 0) pos_x = 0;
    if (pos_y < 0) pos_y = 0;
    if (pos_x > 640 - 20) pos_x = 640 - 20;
    if (pos_y > 360 - 20) pos_y = 360 - 20;
}


bool Player::is_alive() {
    return HP > 0;
}

void Player::physics() {
   ALLEGRO_KEYBOARD_STATE key_state;
    al_get_keyboard_state(&key_state);

    //Плавный разгон 
    double acc = 0.5; 
    if (al_key_down(&key_state, ALLEGRO_KEY_W)) vel_y -= acc;
    if (al_key_down(&key_state, ALLEGRO_KEY_S)) vel_y += acc;
    if (al_key_down(&key_state, ALLEGRO_KEY_A)) vel_x -= acc;
    if (al_key_down(&key_state, ALLEGRO_KEY_D)) vel_x += acc;

    //Трение
    double friction = 0.85; 
    vel_x *= friction;
    vel_y *= friction;

    //Ограничение максимальной скорости
    double max_speed = 4.5;
    if (vel_x > max_speed)  vel_x = max_speed;
    if (vel_x < -max_speed) vel_x = -max_speed;
    if (vel_y > max_speed)  vel_y = max_speed;
    if (vel_y < -max_speed) vel_y = -max_speed;

    //Движение
    move(vel_x, vel_y);


    // Логика стрельбы
    ALLEGRO_MOUSE_STATE mouse;
    al_get_mouse_state(&mouse);

    if (fire_cooldown > 0) fire_cooldown--;

    if ((mouse.buttons & 1) && fire_cooldown <= 0) {
        //Считаем коэффициент масштабирования (
        float dw = (float)al_get_display_width(al_get_current_display());
        float dh = (float)al_get_display_height(al_get_current_display());
        float sw = dw / 640.0; // WIDTH
        float sh = dh / 360.0; // HEIGHT
        float scale = (sw < sh) ? sw : sh;

        //Переводим координаты мыши в игровое пространство
        float tx = mouse.x / scale;
        float ty = mouse.y / scale;

        //Создаем пулю
        Screen::spawn(new Bullet(pos_x + 10, pos_y + 10, tx, ty));
        
        fire_cooldown = 15; 
}
}

void Player::render() {
    al_draw_filled_rectangle(pos_x, pos_y, pos_x + 20, pos_y + 20, 
                             al_map_rgb(0, 255, 100));
}