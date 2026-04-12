#ifndef BOSS_HPP
#define BOSS_HPP

#include "../Enemy.hpp"
#include <string>
#include <allegro5/allegro.h>
#include "../../../Visuals/Assets.hpp"

class Boss : public Enemy {
public:
    std::string boss_name;
    ALLEGRO_BITMAP* cached_name_bmp = nullptr;

    Boss(float x, float y, std::string name, float m_hp) : Enemy(x, y, m_hp) {
        this->boss_name = name;
        this->hp = m_hp; 
        this->max_hp = m_hp;
        this->is_boss = true;
        this->width = 64;
        this->height = 64;
    }

    virtual ~Boss() {
        if (cached_name_bmp) al_destroy_bitmap(cached_name_bmp);
    }

    // Инициализация кэша текста (вызывается один раз при спавне)
    void init_ui() override { 
        if (!Assets::main_font || boss_name.empty()) return;

        int tw = al_get_text_width(Assets::main_font, boss_name.c_str());
        int th = al_get_font_line_height(Assets::main_font);

        if (cached_name_bmp) al_destroy_bitmap(cached_name_bmp);

        cached_name_bmp = al_create_bitmap(tw, th);
        if (!cached_name_bmp) return;

        ALLEGRO_BITMAP* prev_target = al_get_target_bitmap();
        al_set_target_bitmap(cached_name_bmp);
        al_clear_to_color(al_map_rgba(0, 0, 0, 0)); 
        al_draw_text(Assets::main_font, al_map_rgb(255, 255, 255), 0, 0, 0, boss_name.c_str());
        al_set_target_bitmap(prev_target);
    }

    // Геттер для кэша (нужен рендереру)
    ALLEGRO_BITMAP* get_name_bmp() const { return cached_name_bmp; }
};
#endif