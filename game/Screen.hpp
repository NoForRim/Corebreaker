#ifndef SCREEN_HPP
#define SCREEN_HPP

#include <vector>

// Forward declaration, чтобы не тащить лишние инклуды
class Game_Object;
class Player;

class Screen {
public:
    inline static std::vector<Game_Object*> objects;
    inline static std::vector<Game_Object*> to_add;
    inline static Player* player = nullptr;

    static void spawn(Game_Object* obj) {
        to_add.push_back(obj);
    }
};

#endif