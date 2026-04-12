#ifndef SCREEN_HPP
#define SCREEN_HPP

#include <vector>
#include <memory>

// Forward declaration, чтобы не тащить лишние инклуды
class Game_Object;
class Player;

class Screen {
public:
    // Использую unique_ptr для автоматического управления памятью объектов
    inline static std::vector<std::unique_ptr<Game_Object>> objects;
    inline static std::vector<std::unique_ptr<Game_Object>> to_add;
    inline static Player* player = nullptr;  // Управляется отдельно в GameScreen

    // Спавним новый объект, передавая ownership
    static void spawn(std::unique_ptr<Game_Object> obj) {
        to_add.push_back(std::move(obj));
    }
};

#endif