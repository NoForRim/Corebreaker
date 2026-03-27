#ifndef BOSS_HPP
#define BOSS_HPP

#include "../Enemy.hpp"
#include <string>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>

class Boss : public Enemy {
public:
    std::string boss_name;
    ALLEGRO_BITMAP* cached_name_bmp = nullptr; // Кэш для текста

    Boss(float x, float y, std::string name, float m_hp) : Enemy(x, y, m_hp) {
        this->boss_name = name;
        
        // Явно прописываем значения в поля Game_Object
        this->hp = m_hp; 
        this->max_hp = m_hp;

        this->is_boss = true;
        this->width = 64;
        this->height = 64;
    }

    // Удаляем битмап при смерти босса
    virtual ~Boss() {
        if (cached_name_bmp) {
            al_destroy_bitmap(cached_name_bmp);
        }
    }

    // Создаем картинку из текста один раз
    void init_ui(ALLEGRO_FONT* font) override {
        if (!font || boss_name.empty()) return;

        int tw = al_get_text_width(font, boss_name.c_str());
        int th = al_get_font_line_height(font);

        cached_name_bmp = al_create_bitmap(tw, th);
        if (!cached_name_bmp) return;

        ALLEGRO_BITMAP* prev_target = al_get_target_bitmap();
        al_set_target_bitmap(cached_name_bmp);
        al_clear_to_color(al_map_rgba(0, 0, 0, 0)); // Прозрачный фон
        al_draw_text(font, al_map_rgb(255, 255, 255), 0, 0, 0, boss_name.c_str());
        al_set_target_bitmap(prev_target);
    }

    void draw_boss_ui() {
        if (!is_alive) return;
        
        float bar_width = 400;
        float current_bar_width = (hp / max_hp) * bar_width;
        
        // Полоска здоровья (рисуется быстро)
        al_draw_filled_rectangle(120, 20, 120 + bar_width, 40, al_map_rgb(50, 0, 0));
        al_draw_filled_rectangle(120, 20, 120 + current_bar_width, 40, al_map_rgb(200, 0, 0));
        
        // Рисуем заранее подготовленную картинку с текстом
        if (cached_name_bmp) {
            int tw = al_get_bitmap_width(cached_name_bmp);
            al_draw_bitmap(cached_name_bmp, 320 - tw / 2, 45, 0);
        }
    }
};
#endif