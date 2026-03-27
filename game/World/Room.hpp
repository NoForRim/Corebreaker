#ifndef Room_hpp
#define Room_hpp

#include <vector>
#include "Room_Templates.hpp"

class Room {
public:
    int tiles[11][20];

    // Загрузка данных из шаблона
    void load_template(const RoomData& data) {
        for(int y = 0; y < 11; y++) {
            for(int x = 0; x < 20; x++) {
                tiles[y][x] = data.tiles[y][x];
            }
        }
    }

    void render();
    bool is_wall(float x, float y);
};

#endif