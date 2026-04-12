#ifndef Floor_Manager_hpp
#define Floor_Manager_hpp

#include "Room_Templates.hpp"
#include "Room.hpp"
#include <map>
#include <vector>
#include <ctime>
#include <iostream>
#include <algorithm>

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

public:
    FloorManager() { Templates::Init(); }

    const std::map<RoomLocation, RoomData>& get_floor_map() const { return floor_map; }
    RoomLocation get_current_pos() const { return current_pos; }
    int get_current_floor() const { return current_floor; }
    
    static Side get_opposite_side(Side dir) {
        if (dir == UP) return DOWN;
        if (dir == DOWN) return UP;
        if (dir == LEFT) return RIGHT;
        if (dir == RIGHT) return LEFT;
        return NONE;
    }

    static RoomLocation get_next_loc(RoomLocation loc, Side dir) {
        if (dir == UP) loc.y--;
        else if (dir == DOWN) loc.y++;
        else if (dir == LEFT) loc.x--;
        else if (dir == RIGHT) loc.x++;
        return loc;
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