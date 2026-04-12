#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include <vector>
#include <algorithm>

#include "../Screen.hpp"
#include "../Game_Objects/Game_Object.hpp"
#include "../Game_Objects/Player/Player.hpp"
#include "../Game_Objects/Enemies/Bosses/Boss.hpp"
#include "../World/Room.hpp"
#include "UI/UI_Utils.hpp"

class Renderer {
public:
    // 1. Отрисовка игрового мира и объектов
    static void draw_world(Room& room, const std::vector<std::unique_ptr<Game_Object>>& objects) {
        room.render(); // Сначала рисуем пол/стены

        // Создаём временный вектор указателей для сортировки
        std::vector<Game_Object*> obj_ptrs;
        for (const auto& obj : objects) {
            if (obj) obj_ptrs.push_back(obj.get());
        }

        // Сортируем объекты: те, у кого Y меньше (выше на экране), 
        // отрисовываются первыми, чтобы нижние объекты их перекрывали.
        std::sort(obj_ptrs.begin(), obj_ptrs.end(), [](Game_Object* a, Game_Object* b) {
            return (a->pos_y + a->height) < (b->pos_y + b->height);
        });

        for (auto obj : obj_ptrs) {
            if (obj && obj->is_alive) {
                obj->render();
            }
        }
    }

    // 2. Отрисовка интерфейса игрока (HUD)
    static void draw_player_hud(Player* player, ALLEGRO_FONT* font) {
        if (!player || !font) return;

        // Координаты и размеры главного модуля здоровья
        float x = 20;
        float y = 330;
        float bw = 180;
        float bh = 22;

        // Обновляем текстовый кэш (цифры HP), если данные изменились
        if (player->hp_needs_update) {
            player->update_hp_cache(font);
            player->hp_needs_update = false;
        }

        // --- Вызовы отрисовки из UI_Utils ---

        // Шкала взлома (чуть выше основного бара)
        UI_Utils::draw_corruption_bar(
            x, y - 10, bw, 4, 
            player->get_signal_part() / 100.0f, 
            player->get_noise_part() / 100.0f
        );

        // Основной бар: Ралли (фон) и текущее ХП (поверх)
        // Рассчитываем проценты здесь, чтобы UI_Utils оставался "глупым" и универсальным
        float hp_perc = (player->max_hp > 0) ? (player->hp / player->max_hp) : 0;
        float rally_perc = (player->max_hp > 0) ? ((player->hp + player->recoverable_hp) / player->max_hp) : 0;
        
        UI_Utils::draw_health_bar_rally(x, y, bw, bh, hp_perc, rally_perc);

        // Отрисовка кэшированного текста (цифры HP внутри бара)
        if (player->get_hp_cache()) {
            al_draw_bitmap(player->get_hp_cache(), x, y + 8, 0);
        }
    }

    // 3. Отрисовка интерфейса босса
    static void draw_boss_hud(Boss* boss) {
        if (!boss || !boss->is_alive) return;

        // Параметры полоски босса (центрируем по экрану)
        float bw = 400; // Ширина
        float bh = 20;  // Высота
        float bx = 320 - (bw / 2); // Центр при разрешении 640 
        float by = 20;

        // 1. Отрисовка полоски через наши утилиты
        UI_Utils::draw_simple_bar(bx, by, bw, bh, boss->hp / boss->max_hp, al_map_rgb(200, 0, 0));

        // 2. Отрисовка имени (кэшированного битмапа)
        ALLEGRO_BITMAP* name_bmp = boss->get_name_bmp();
        if (name_bmp) {
            int tw = al_get_bitmap_width(name_bmp);
            // Рисуем имя чуть ниже полоски (by + bh + отступ)
            al_draw_bitmap(name_bmp, 320 - tw / 2, by + bh + 5, 0);
        }
    }

    // 4. Отрисовка прицела
    static void draw_crosshair(float mx, float my, const std::vector<std::unique_ptr<Game_Object>>& objects, float anim_tick) {
        ALLEGRO_COLOR color = al_map_rgb(255, 255, 255);
        
        // Проверка наведения на врага
        for (const auto& obj : objects) {
            if (obj && obj->is_enemy && obj->is_alive &&
                mx >= obj->pos_x && mx <= obj->pos_x + obj->width &&
                my >= obj->pos_y && my <= obj->pos_y + obj->height) {
                color = al_map_rgb(255, 50, 50); // Красный при наведении
                break;
            }
        }
        
        // Передаем anim_tick в UI_Utils для расчета пульсации
        UI_Utils::draw_custom_crosshair(mx, my, color, anim_tick);
    }
};

#endif