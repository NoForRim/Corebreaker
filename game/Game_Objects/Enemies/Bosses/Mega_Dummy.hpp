#ifndef Mega_Dummy_hpp
#define Mega_Dummy_hpp

#include "Boss.hpp"
#include "../../../Screen.hpp"
#include "../../Player/Player.hpp"
#include <allegro5/allegro_primitives.h>
#include <cmath>
#include <cstdio>

class Mega_Dummy : public Boss {
private:
    static constexpr float BOSS_HP = 600.0f;
public:
    Mega_Dummy(float x, float y) : Boss(x, y, "MEGA_DUMMY", BOSS_HP) {
        this->width = 80;
        this->height = 80;
    }

    void physics() override {
        if (!is_alive) return;

        if (Screen::player) {
            float dx = (float)Screen::player->pos_x - this->pos_x;
            float dy = (float)Screen::player->pos_y - this->pos_y;
            float dist = std::sqrt(dx * dx + dy * dy);

            if (dist > 5.0f) {
                this->pos_x += (dx / dist) * 0.4f;
                this->pos_y += (dy / dist) * 0.4f;
            }
        }
    }

    void render() override {
        if (!is_alive) return;
        al_draw_filled_rectangle(pos_x, pos_y, pos_x + width, pos_y + height, al_map_rgb(200, 0, 0));
        float gap = 12.0f;
        al_draw_filled_rectangle(pos_x + gap, pos_y + gap, pos_x + width - gap, pos_y + height - gap, al_map_rgb(255, 60, 60));
        al_draw_rectangle(pos_x, pos_y, pos_x + width, pos_y + height, al_map_rgb(0, 0, 0), 3);
    }

    void take_damage(float damage, float kx, float ky) override {
        Boss::take_damage(damage, kx, ky);
    }
};
#endif