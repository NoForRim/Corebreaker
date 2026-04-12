#ifndef UI_Utils_hpp
#define UI_Utils_hpp

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_color.h>
#include <algorithm>

class UI_Utils {
public:
    //Текст с тенью
    static void draw_text_shadow(ALLEGRO_FONT* font, ALLEGRO_COLOR color, float x, float y, int flags, const char* text) {
        if (!font || !text) return;
        al_draw_text(font, al_map_rgb(0, 0, 0), x + 1, y + 1, flags, text);
        al_draw_text(font, color, x, y, flags, text);
    }

    static void draw_health_bar_rally(float x, float y, float w, float h, float hp_perc, float rally_perc) {
        // Фон
        al_draw_filled_rectangle(x, y, x + w, y + h, al_map_rgb(30, 30, 30));
        
        // Оранжевый слой (Rally) — считаем общую длину (HP + Recoverable)
        if (rally_perc > 0) {
            float rw = w * std::min(1.0f, rally_perc);
            al_draw_filled_rectangle(x, y, x + rw, y + h, al_map_rgb(200, 100, 0));
        }
        
        // Зеленый слой (HP)
        if (hp_perc > 0) {
            float hw = w * std::min(1.0f, hp_perc);
            al_draw_filled_rectangle(x, y, x + hw, y + h, al_map_rgb(50, 200, 80));
        }
        
        // Рамка
        al_draw_rectangle(x - 1, y - 1, x + w + 1, y + h + 1, al_map_rgb(100, 100, 100), 1);
    }

    static void draw_corruption_bar(float x, float y, float w, float h, float sig_perc, float noi_perc) {
        ALLEGRO_COLOR bg_color = al_map_rgba(20, 20, 20, 150);
        ALLEGRO_COLOR signal_color = al_map_rgb(110, 0, 255); 
        ALLEGRO_COLOR noise_color  = al_map_rgb(100, 100, 110);
        ALLEGRO_COLOR notch_color  = al_map_rgb(120, 120, 120);

        // Подложка
        al_draw_filled_rectangle(x, y, x + w, y + h, bg_color);

        float sig_w = w * std::min(1.0f, sig_perc);
        float noi_w = w * std::min(1.0f, noi_perc);

        // Чтобы шум не вылезал за границы при сумме > 100%
        if (sig_w + noi_w > w) noi_w = w - sig_w;

        if (sig_w > 0) al_draw_filled_rectangle(x, y, x + sig_w, y + h, signal_color);
        if (noi_w > 0) al_draw_filled_rectangle(x + sig_w, y, x + sig_w + noi_w, y + h, noise_color);

        // Рамка
        al_draw_rectangle(x - 0.5, y - 0.5, x + w + 0.5, y + h + 0.5, al_map_rgb(80, 80, 80), 1);

        // Засечки (5 секций)
        for (int i = 1; i <= 4; ++i) {
            float notch_x = x + (w / 5.0f) * i;
            al_draw_line(notch_x, y, notch_x, y + h, notch_color, 1);
        }
    }

    static void draw_custom_crosshair(float mx, float my, ALLEGRO_COLOR color, float t) {
        // --- 1. КОНСТАНТЫ (Максимальный размер) ---
        const float MAX_BSZ = 7.0f;           // Угол квадрата
        const float MAX_GAP = 2.5f;           // Прорезь у центра
        const float th = 1.2f;                // Толщина

        // --- 2. ФАЗЫ (0.0 - 1.0) ---
        float anim_mult = 1.0f; // 1.0 = Квадрат, 0.0 = Точка в центре

        if (t < 0.6f) {
            anim_mult = 1.0f; // Замирание (Квадрат)
        } 
        else if (t < 0.8f) {
            float phase_t = (t - 0.6f) / 0.2f; 
            anim_mult = 1.0f - phase_t; // Сжатие в центр
        } 
        else {
            float phase_t = (t - 0.8f) / 0.2f;
            anim_mult = phase_t; // Возврат в квадрат
        }

        // --- 3. РАСЧЕТ ТЕКУЩИХ КООРДИНАТ ---
        // И углы, и прорези множатся на anim_mult.
        // Когда anim_mult = 0, все координаты станут mx/my (точка).
        float bsz = MAX_BSZ * anim_mult;
        float gap = MAX_GAP * anim_mult;

        if (bsz < 0.2f) return; // Не рисуем невидимую точку

        // --- 4. ОТРИСОВКА ---
        // Верхний левый
        al_draw_line(mx - bsz, my - bsz, mx - gap, my - bsz, color, th);
        al_draw_line(mx - bsz, my - bsz, mx - bsz, my - gap, color, th);
        
        // Верхний правый
        al_draw_line(mx + bsz, my - bsz, mx + gap, my - bsz, color, th);
        al_draw_line(mx + bsz, my - bsz, mx + bsz, my - gap, color, th);
        
        // Нижний левый
        al_draw_line(mx - bsz, my + bsz, mx - gap, my + bsz, color, th);
        al_draw_line(mx - bsz, my + bsz, mx - bsz, my + gap, color, th);
        
        // Нижний правый
        al_draw_line(mx + bsz, my + bsz, mx + gap, my + bsz, color, th);
        al_draw_line(mx + bsz, my + bsz, mx + bsz, my + gap, color, th);
    }

    static void draw_simple_bar(float x, float y, float w, float h, float perc, ALLEGRO_COLOR color) {
    // Фон (тёмная подложка)
    al_draw_filled_rectangle(x, y, x + w, y + h, al_map_rgb(40, 20, 20)); 
    if (perc > 0) {
        // Ограничиваем perc через std::min, чтобы полоска не вылетала за рамки
        al_draw_filled_rectangle(x, y, x + (w * std::min(1.0f, perc)), y + h, color);
    }
    // Опционально: тонкая рамка
    al_draw_rectangle(x - 1, y - 1, x + w + 1, y + h + 1, al_map_rgb(80, 80, 80), 1);
}
};

#endif