#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <vector>
#include <algorithm>
#include "../Screen.hpp"
#include "../Game_Objects/Game_Object.hpp"
#include "../Game_Objects/Player/Player.hpp"
#include "../Game_Objects/Enemies/Bosses/Boss.hpp"
#include "../World/Room.hpp"

class Renderer {
public:
    // 1. Отрисовка игровых объектов (мир, сущности)
    static void draw_world(Room& room, const std::vector<Game_Object*>& objects) {
        room.render();
        for (auto obj : objects) {
            if (obj && obj->is_alive) obj->render();
        }
    }

    // 2. Отрисовка интерфейса игрока (HP Bar с системой Rally)
    static void draw_player_hud(Player* player, ALLEGRO_FONT* font) {
        if (!player || !font) return;

        float x = 20; 
        float y = 320; 
        float bar_width = 100;
        float bar_height = 12;

        // Если HP изменилось, перерисовываем текстуру текста
        if (player->hp_needs_update && font) {
            player->update_hp_cache(font);
            player->hp_needs_update = false; // Сбрасываем флаг
        }

        // Рисуем кэшированный битмап (теперь он всегда актуальный)
        if (player->get_hp_cache()) {
            al_draw_bitmap(player->get_hp_cache(), x, y - 18, 0);
        }
        
        //2. ВСЁ ОСТАЛЬНОЕ (Рамка и бары)
        al_draw_rectangle(x - 1, y - 1, x + bar_width + 1, y + bar_height + 1, al_map_rgb(150, 150, 150), 1);
        al_draw_filled_rectangle(x, y, x + bar_width, y + bar_height, al_map_rgb(20, 20, 20));

        //Отрисовка полосок
        float g_w = (player->hp / player->max_hp) * bar_width;
        float o_w = ((player->hp + player->recoverable_hp) / player->max_hp) * bar_width;

        if (o_w > 0) al_draw_filled_rectangle(x, y, x + std::min(o_w, bar_width), y + bar_height, al_map_rgb(200, 100, 0));
        if (g_w > 0) al_draw_filled_rectangle(x, y, x + std::min(g_w, bar_width), y + bar_height, al_map_rgb(50, 200, 80));
    }

    // 3. Отрисовка UI босса
    static void draw_boss_hud(Boss* boss) {
        if (boss && boss->is_alive) {
            boss->draw_boss_ui();
        }
    }

    // 4. Отрисовка курсора (прицела)
    static void draw_crosshair(float mx, float my, const std::vector<Game_Object*>& objects, float anim_tick) {
        ALLEGRO_COLOR color = al_map_rgb(255, 255, 255);
        
        // Проверка наведения на врага
        for (auto obj : objects) {
            if (obj && obj->is_enemy && obj->is_alive &&
                mx >= obj->pos_x && mx <= obj->pos_x + obj->width &&
                my >= obj->pos_y && my <= obj->pos_y + obj->height) {
                color = al_map_rgb(255, 50, 50);
                break;
            }
        }

        float bsz = 7.0f; // CH_BASE_SIZE
        float gap = bsz * 0.4f; // CH_GAP_FACTOR
        float th = 1.2f; // CH_THICKNESS

        // Рисуем уголки
        // Верх-лево
        al_draw_line(mx - bsz, my - bsz, mx - gap, my - bsz, color, th);
        al_draw_line(mx - bsz, my - bsz, mx - bsz, my - gap, color, th);
        // Верх-право
        al_draw_line(mx + bsz, my - bsz, mx + gap, my - bsz, color, th);
        al_draw_line(mx + bsz, my - bsz, mx + bsz, my - gap, color, th);
        // Низ-лево
        al_draw_line(mx - bsz, my + bsz, mx - gap, my + bsz, color, th);
        al_draw_line(mx - bsz, my + bsz, mx - bsz, my + gap, color, th);
        // Низ-право
        al_draw_line(mx + bsz, my + bsz, mx + gap, my + bsz, color, th);
        al_draw_line(mx + bsz, my + bsz, mx + bsz, my + gap, color, th);
    }
};

#endif