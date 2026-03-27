#ifndef Floor_Manager_hpp
#define Floor_Manager_hpp

#include "Room_Templates.hpp"
#include "Room.hpp"
#include <map>
#include <vector>
#include <ctime>
#include <iostream>
#include <algorithm>
#include <allegro5/allegro_primitives.h>

struct RoomLocation {
    int x, y;
    bool operator<(const RoomLocation& other) const {
        return x != other.x ? x < other.x : y < other.y;
    }
    bool operator==(const RoomLocation& other) const { return x == other.x && y == other.y; }
};

class FloorManager {
private:
    int current_floor = 1;
    std::map<RoomLocation, RoomData> floor_map;
    RoomLocation current_pos = {5, 5};
    float map_fade = 0.0f;
    const float fade_speed = 0.08f;

    Side get_opposite_side(Side dir) {
        if (dir == UP) return DOWN;
        if (dir == DOWN) return UP;
        if (dir == LEFT) return RIGHT;
        if (dir == RIGHT) return LEFT;
        return NONE;
    }

    RoomLocation get_next_loc(RoomLocation loc, Side dir) {
        if (dir == UP) loc.y--;
        else if (dir == DOWN) loc.y++;
        else if (dir == LEFT) loc.x--;
        else if (dir == RIGHT) loc.x++;
        return loc;
    }

    RoomData select_room_variation(int mask, RoomType type) {
        if (Templates::Table.count(mask)) {
            auto& vars = Templates::Table[mask];
            std::vector<RoomData*> filtered;
            for (auto& v : vars) if (v.type == type) filtered.push_back(&v);
            if (filtered.empty()) {
                for (auto& v : vars) if (v.type == COMMON) filtered.push_back(&v);
            }
            return filtered.empty() ? RoomData() : *filtered[rand() % filtered.size()];
        }
        return RoomData();
    }

    void render_map_content(float ox, float oy, float sz, float m, unsigned char alpha) {
        for (auto const& [loc, data] : floor_map) {
            bool is_revealed = data.visited;
            bool neighbor_is_visited = false;
            
            if (!is_revealed) {
                Side dirs[] = {UP, DOWN, LEFT, RIGHT};
                for (Side d : dirs) {
                    RoomLocation adj = get_next_loc(loc, d);
                    if (floor_map.count(adj) && floor_map[adj].visited && (floor_map[adj].mask & (int)get_opposite_side(d))) {
                        is_revealed = true;
                        neighbor_is_visited = true;
                        break;
                    }
                }
            }

            if (!is_revealed) continue;

            float x1 = ox + loc.x * (sz + m);
            float y1 = oy + loc.y * (sz + m);
            float cx = x1 + sz / 2.0f;
            float cy = y1 + sz / 2.0f;

            ALLEGRO_COLOR c_line = al_map_rgba(100, 100, 100, alpha);
            if (data.visited) {
                if (data.mask & UP)    al_draw_line(cx, y1, cx, y1 - m, c_line, 1);
                if (data.mask & DOWN)  al_draw_line(cx, y1 + sz, cx, y1 + sz + m, c_line, 1);
                if (data.mask & LEFT)  al_draw_line(x1, cy, x1 - m, cy, c_line, 1);
                if (data.mask & RIGHT) al_draw_line(x1 + sz, cy, x1 + sz + m, cy, c_line, 1);
            } else if (neighbor_is_visited) {
                Side dirs[] = {UP, DOWN, LEFT, RIGHT};
                for (Side d : dirs) {
                    RoomLocation adj = get_next_loc(loc, d);
                    if (floor_map.count(adj) && floor_map[adj].visited && (data.mask & (int)d)) {
                        if (d == UP)    al_draw_line(cx, y1, cx, y1 - m, c_line, 1);
                        if (d == DOWN)  al_draw_line(cx, y1 + sz, cx, y1 + sz + m, c_line, 1);
                        if (d == LEFT)  al_draw_line(x1, cy, x1 - m, cy, c_line, 1);
                        if (d == RIGHT) al_draw_line(x1 + sz, cy, x1 + sz + m, cy, c_line, 1);
                    }
                }
            }

            ALLEGRO_COLOR c_room;
            if (loc == current_pos) c_room = al_map_rgba(0, 255, 150, alpha);
            else if (data.visited)  c_room = al_map_rgba(60, 60, 90, alpha);
            else                    c_room = al_map_rgba(40, 40, 45, alpha);

            al_draw_filled_rectangle(x1, y1, x1 + sz, y1 + sz, c_room);

            float p = sz * 0.25f;
            if (data.type == TREASURE) al_draw_filled_rectangle(x1 + p, y1 + p, x1 + sz - p, y1 + sz - p, al_map_rgba(255, 200, 0, alpha));
            else if (data.type == SHOP) al_draw_filled_rectangle(x1 + p, y1 + p, x1 + sz - p, y1 + sz - p, al_map_rgba(0, 100, 255, alpha));
            else if (data.type == BOSS) al_draw_filled_rectangle(x1 + p, y1 + p, x1 + sz - p, y1 + sz - p, al_map_rgba(255, 0, 0, alpha));
        }
    }

public:
    FloorManager() { Templates::Init(); }

    int get_current_floor() const { 
        return current_floor; 
    }

    void generate_floor(int floor_level) {
    floor_map.clear();
    std::map<RoomLocation, int> skeleton;
    std::vector<RoomLocation> room_list;
    RoomLocation start = {5, 5};
    skeleton[start] = 0; 
    room_list.push_back(start);

    

    // 1. Генерируем основной путь (обычные комнаты)
    int target_rooms = 6 + floor_level; 
    while ((int)room_list.size() < target_rooms) {
        RoomLocation parent = room_list[rand() % room_list.size()];
        Side dir = (Side)(1 << (rand() % 4));
        RoomLocation child = get_next_loc(parent, dir);
        
        if (child.x < 1 || child.x > 9 || child.y < 1 || child.y > 9) continue;
        if (skeleton.find(child) == skeleton.end()) {
            skeleton[parent] |= (int)dir;
            skeleton[child] |= (int)get_opposite_side(dir);
            room_list.push_back(child);
        }
    }

    // 2. Функция для добавления спец-тупика
    auto add_special_room = [&](RoomType type) {
        // Ищем комнату, у которой есть свободное соседнее место
        for (int i = 0; i < 50; i++) { // 50 попыток
            RoomLocation parent = room_list[rand() % room_list.size()];
            Side dir = (Side)(1 << (rand() % 4));
            RoomLocation child = get_next_loc(parent, dir);

            // Если место свободно
            if (child.x >= 1 && child.x <= 9 && child.y >= 1 && child.y <= 9 && skeleton.find(child) == skeleton.end()) {
                skeleton[parent] |= (int)dir;           // Добавляем дверь в родителе
                skeleton[child] = (int)get_opposite_side(dir); // В новой комнате только одна дверь назад
                
                // Сразу создаем данные комнаты
                floor_map[child] = select_room_variation(skeleton[child], type);
                floor_map[child].type = type;
                floor_map[child].visited = false;
                floor_map[child].is_cleared = true; // Спец-комнаты зачищены
                
                floor_map[child].is_cleared = (type != BOSS);
                return;
            }
        }
    };

    // Добавляем спец-комнаты как отдельные "отростки"
    add_special_room(SHOP);
    add_special_room(TREASURE);
    add_special_room(BOSS);

    // 3. Заполняем остальные комнаты COMMON
    for (auto const& [loc, mask] : skeleton) {
        if (floor_map.find(loc) == floor_map.end()) { // Если еще не заполнено спец-логикой
            RoomType t = (loc == start) ? START : COMMON;
            floor_map[loc] = select_room_variation(mask, t);
            floor_map[loc].type = t;
            floor_map[loc].visited = false;
            floor_map[loc].is_cleared = (t == START);
        }
    }

    current_pos = start;
    floor_map[current_pos].visited = true;
}

    void draw_minimap(bool is_tab_pressed, float sw, float sh) {
        if (is_tab_pressed) { if (map_fade < 1.0f) map_fade += fade_speed; }
        else { if (map_fade > 0.0f) map_fade -= fade_speed; }
        if (map_fade > 1.0f) map_fade = 1.0f;
        if (map_fade < 0.0f) map_fade = 0.0f;
        if (map_fade > 0.01f) draw_interface_overlay(sw, sh, map_fade);
        if (map_fade < 0.9f) draw_compact_minimap(1.0f - map_fade);
    }

    void draw_interface_overlay(float sw, float sh, float alpha_mult) {
    float sz = 24.0f, m = 6.0f;
    int min_x = 100, max_x = -100, min_y = 100, max_y = -100;
    bool any_visible = false;

    for (auto const& [loc, data] : floor_map) {
        // Логика раскрытия (как в render_map_content)
        bool is_revealed = data.visited;
        if (!is_revealed) {
            Side dirs[] = {UP, DOWN, LEFT, RIGHT};
            for (Side d : dirs) {
                RoomLocation adj = get_next_loc(loc, d);
                if (floor_map.count(adj) && floor_map[adj].visited && (floor_map[adj].mask & (int)get_opposite_side(d))) {
                    is_revealed = true;
                    break;
                }
            }
        }

        if (is_revealed) {
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
    render_map_content(ox, oy, sz, m, 255 * alpha_mult);
}

    void draw_compact_minimap(float alpha_mult) {
        float sz = 10.0f, m = 2.0f, ox = 20.0f, oy = 20.0f;
        render_map_content(ox, oy, sz, m, 150 * alpha_mult);
    }

    bool can_move(Side dir) { return (bool)(floor_map[current_pos].mask & (int)dir); }
    
    void move(Side dir, Room& room_obj) {
        RoomLocation next = get_next_loc(current_pos, dir);
        if (floor_map.count(next)) {
            current_pos = next;
            floor_map[current_pos].visited = true;
            room_obj.load_template(floor_map[current_pos]);
        }
    }

    // Возвращаем ссылку, чтобы можно было менять is_cleared
    RoomData& get_current_room_data() { return floor_map[current_pos]; }
    
    int get_current_x() { return current_pos.x; }
    int get_current_y() { return current_pos.y; }
};

#endif