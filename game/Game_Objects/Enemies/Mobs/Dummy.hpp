#ifndef Dummy_hpp
#define Dummy_hpp

#include "../Enemy.hpp"
#include <allegro5/allegro_primitives.h>

class Dummy : public Enemy {
private:
    static constexpr float INITIAL_HP = 100.0f; // Переменная здоровья здесь

public:
    Dummy(float x, float y) : Enemy(x, y, INITIAL_HP) {
        this->width = 32;
        this->height = 32;
    }

    void physics() override; 
    void render() override;
};
#endif