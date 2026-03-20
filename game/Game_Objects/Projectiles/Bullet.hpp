#ifndef Bullet_hpp
#define Bullet_hpp

#include "../Game_Object.hpp"
#include <cmath>

class Bullet : public Game_Object {
public:
    double dx, dy;
    double speed = 5.0;

    Bullet(double x, double y, double target_x, double target_y) : Game_Object(x, y) {
        double diff_x = target_x - x;
        double diff_y = target_y - y;
        double distance = std::sqrt(diff_x * diff_x + diff_y * diff_y);
        
        dx = (diff_x / distance) * speed;
        dy = (diff_y / distance) * speed;
    }

    void physics() override {
        pos_x += dx;
        pos_y += dy;
        // Удаляем пулю, если она улетела далеко за экран
        if (pos_x < -100 || pos_x > 800 || pos_y < -100 || pos_y > 500) {
            is_alive = false;
        }
    }

    void render() override {
        al_draw_filled_circle(pos_x, pos_y, 3, al_map_rgb(255, 255, 0));
    }
};

#endif