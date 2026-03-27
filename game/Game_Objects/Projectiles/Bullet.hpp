#ifndef Bullet_hpp
#define Bullet_hpp

#include "../Game_Object.hpp"

class Bullet : public Game_Object {
public:
    float damage;
    // Удаляем dx, dy, speed из полей, если используем vel_x, vel_y из базового класса
    Bullet(float x, float y, float target_x, float target_y, float dmg);

    void physics() override;
    void render() override;
};

#endif