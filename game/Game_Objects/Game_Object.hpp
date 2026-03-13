#ifndef Scope_Game_Object
#define Scope_Game_Object

class Game_Object {
    public:
        double pos_x;
        double pos_y;
        virtual void physics() = 0;
        virtual void render() = 0;
        virtual ~Game_Object() = default;
};
/*
class Enemy{
    public:
        double pos_x;
        double pos_y;
        double HP;
        void move(double move_x, double move_y);
        bool is_alive(double HP);
}

class Consumables{
    public:
        double pos_x;
        double pos_y;
}
*/

#endif