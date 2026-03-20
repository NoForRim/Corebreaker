#ifndef Scope_Player
#define Scope_Player
#include "../Game_Object.hpp"
#include "../../engine.hpp"
#include <allegro5/allegro_primitives.h>
#include <allegro5/keyboard.h>
#include <allegro5/keycodes.h>

#include <cstddef>
#include <iostream>
#include <vector>

class Player : public Game_Object {
    public:
        double HP = 100.0;
        double speed = 1.0;
        double vel_x;
        double vel_y;
        

        Player(double x, double y);

        void move(double move_x, double move_y);
        bool is_alive();

        void physics() override;
        void render() override;
};

#endif