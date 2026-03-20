#ifndef Dummy_hpp
#define Dummy_hpp

#include "../Game_Object.hpp"
#include <allegro5/allegro_primitives.h>

class Dummy : public Game_Object {
public:
    Dummy(double x, double y) : Game_Object(x, y) {
        width = 24;
        height = 24;
    }

    void physics() override; // Здесь будет логика получения урона
    void render() override;
};

#endif