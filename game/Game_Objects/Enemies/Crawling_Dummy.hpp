#ifndef Crawling_Dummy_hpp
#define Crawling_Dummy_hpp

#include "../Game_Object.hpp"
#include <allegro5/allegro_primitives.h>

class Crawling_Dummy : public Game_Object {
private:
    double speed = 0.8; // Скорость ползания (медленнее игрока)

public:
    Crawling_Dummy(double x, double y) : Game_Object(x, y) {
        width = 20;
        height = 20;
    }

    void physics() override;
    void render() override;
};

#endif