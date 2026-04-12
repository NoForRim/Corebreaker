#ifndef MAP_RENDERER_HPP
#define MAP_RENDERER_HPP

#include <allegro5/allegro_primitives.h>
#include <map>
#include <algorithm>
#include "../World/Floor_Manager.hpp"

class MapRenderer {
public:
    static void draw(const FloorManager& fm, bool is_tab_pressed, float sw, float sh) {
        static float map_fade = 0.0f;
        const float fade_speed = 0.08f;

        if (is_tab_pressed) { if (map_fade < 1.0f) map_fade += fade_speed; }
        else { if (map_fade > 0.0f) map_fade -= fade_speed; }
        
        if (map_fade > 1.0f) map_fade = 1.0f;
        if (map_fade < 0.0f) map_fade = 0.0f;

        if (map_fade > 0.01f) draw_interface_overlay(fm, sw, sh, map_fade);
        if (map_fade < 0.9f) draw_compact_minimap(fm, 1.0f - map_fade);
    }

private:
    static void draw_interface_overlay(const FloorManager& fm, float sw, float sh, float alpha_mult) {
        auto& floor_map = fm.get_floor_map();
        float sz = 24.0f, m = 6.0f;
        int min_x = 100, max_x = -100, min_y = 100, max_y = -100;
        bool any_visible = false;

        for (auto const& [loc, data] : floor_map) {
            if (is_revealed(fm, loc, data)) {
                min_x = std::min(min_x, loc.x); max_x = std::max(max_x, loc.x);
                min_y = std::min(min_y, loc.y); max_y = std::max(max_y, loc.y);
                any_visible = true;
            }
        }

        if (!any_visible) return;

        float map_w = (max_x - min_x + 1) * (sz + m) - m;
        float map_h = (max_y - min_y + 1) * (sz + m) - m;
        float ox = (sw / 2.0f) - (min_x * (sz + m)) - (map_w / 2.0f);
        float oy = (sh / 2.0f) - (min_y * (sz + m)) - (map_h / 2.0f);

        al_draw_filled_rounded_rectangle((sw-map_w)/2-20, (sh-map_h)/2-20, (sw+map_w)/2+20, (sh+map_h)/2+20, 10, 10, al_map_rgba(0,0,0,200 * alpha_mult));
        render_map_content(fm, ox, oy, sz, m, 255 * alpha_mult);
    }

    static void draw_compact_minimap(const FloorManager& fm, float alpha_mult) {
        auto& floor_map = fm.get_floor_map();
        float sz = 10.0f; 
        float m = 2.0f;
        float ox = 20.0f; 
        float oy = 20.0f;

        // 1. Инициализируем границы экстремальными значениями
        int min_x = 100, max_x = -100, min_y = 100, max_y = -100;
        bool any_visible = false;

        // 2. Проверяем абсолютно все комнаты в floor_map
        for (auto const& [loc, data] : floor_map) {
            // Если комната видна (посещена или соседствует с посещенной)
            if (is_revealed(fm, loc, data)) {
                if (loc.x < min_x) min_x = loc.x;
                if (loc.x > max_x) max_x = loc.x;
                if (loc.y < min_y) min_y = loc.y;
                if (loc.y > max_y) max_y = loc.y;
                any_visible = true;
            }
        }

        // Если ничего не открыто (в теории невозможно, так как START виден сразу)
        if (!any_visible) return;

        // 3. Рассчитываем координаты прямоугольника
        float x1 = ox + min_x * (sz + m) - 4;
        float y1 = oy + min_y * (sz + m) - 4;
        float x2 = ox + max_x * (sz + m) + sz + 4;
        float y2 = oy + max_y * (sz + m) + sz + 4;

        // 4. Отрисовка подложки
        // Цвет чуть светлее чисто черного, чтобы на фоне 15,15,20 он выделялся
        al_draw_filled_rounded_rectangle(x1, y1, x2, y2, 4, 4, al_map_rgba(0, 0, 0, 180 * alpha_mult));

        // 5. Отрисовка контента (false, чтобы не плодить лишние подложки внутри)
        render_map_content(fm, ox, oy, sz, m, 255 * alpha_mult, false); 
    }

    static bool is_revealed(const FloorManager& fm, RoomLocation loc, const RoomData& data) {
        if (data.visited) return true;
        auto& floor_map = fm.get_floor_map();
        Side dirs[] = {UP, DOWN, LEFT, RIGHT};
        for (Side d : dirs) {
            RoomLocation adj = FloorManager::get_next_loc(loc, d);
            if (floor_map.count(adj) && floor_map.at(adj).visited) {
                // Если сосед посещен и у него есть дверь к нам
                if (floor_map.at(adj).mask & (int)FloorManager::get_opposite_side(d)) return true;
            }
        }
        return false;
    }

    static void render_map_content(const FloorManager& fm, float ox, float oy, float sz, float m, unsigned char alpha, bool draw_bg = false) {
        auto& floor_map = fm.get_floor_map();
        auto current_pos = fm.get_current_pos();

        for (auto const& [loc, data] : floor_map) {
            if (!is_revealed(fm, loc, data)) continue;

            float x1 = ox + loc.x * (sz + m);
            float y1 = oy + loc.y * (sz + m);

            // --- 1. ЛОКАЛЬНАЯ ПОДЛОЖКА (ТОЛЬКО ДЛЯ ЭТОЙ КОМНАТЫ) ---
            if (draw_bg) {
                // Рисуем черный квадрат чуть больше самой комнаты, 
                // чтобы он перекрывал и пространство для линий-дверей (m)
                al_draw_filled_rectangle(x1 - m, y1 - m, x1 + sz + m, y1 + sz + m, al_map_rgba(0, 0, 0, alpha));
            }

            float cx = x1 + sz / 2.0f;
            float cy = y1 + sz / 2.0f;

            // --- 2. РИСУЕМ ЛИНИИ (ДВЕРИ) ---
            ALLEGRO_COLOR c_line = al_map_rgba(100, 100, 100, alpha);
            if (data.visited) {
                if (data.mask & UP)    al_draw_line(cx, y1, cx, y1 - m, c_line, 1);
                if (data.mask & DOWN)  al_draw_line(cx, y1 + sz, cx, y1 + sz + m, c_line, 1);
                if (data.mask & LEFT)  al_draw_line(x1, cy, x1 - m, cy, c_line, 1);
                if (data.mask & RIGHT) al_draw_line(x1 + sz, cy, x1 + sz + m, cy, c_line, 1);
            }

            // --- 3. РИСУЕМ САМУ КОМНАТУ ---
            ALLEGRO_COLOR c_room;
            if (loc == current_pos) c_room = al_map_rgba(0, 255, 150, alpha);
            else if (data.visited)  c_room = al_map_rgba(60, 60, 90, alpha);
            else                    c_room = al_map_rgba(40, 40, 45, alpha);

            al_draw_filled_rectangle(x1, y1, x1 + sz, y1 + sz, c_room);

            // --- 4. СПЕЦ-ИКОНКИ ---
            float p = sz * 0.25f;
            if (data.type == TREASURE) al_draw_filled_rectangle(x1 + p, y1 + p, x1 + sz - p, y1 + sz - p, al_map_rgba(255, 200, 0, alpha));
            else if (data.type == SHOP) al_draw_filled_rectangle(x1 + p, y1 + p, x1 + sz - p, y1 + sz - p, al_map_rgba(0, 100, 255, alpha));
            else if (data.type == BOSS) al_draw_filled_rectangle(x1 + p, y1 + p, x1 + sz - p, y1 + sz - p, al_map_rgba(255, 0, 0, alpha));
        }
    }
};

#endif