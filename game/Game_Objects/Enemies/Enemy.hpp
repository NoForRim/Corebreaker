#ifndef ENEMY_HPP
#define ENEMY_HPP

#include "../Game_Object.hpp"

class Enemy : public Game_Object {
public:
    Enemy(float x, float y, float m_hp) : Game_Object(x, y) {
        this->max_hp = m_hp;
        this->hp = m_hp;
        this->is_enemy = true;
    }

    virtual void init_ui() {}

    void take_damage(float damage, float kx, float ky) override {
        hp -= damage;
        // Импульс от удара
        pos_x += kx * 4;
        pos_y += ky * 4;

        if (hp <= 0) {
            hp = 0;
            is_alive = false;
        }
    }
};

#endif