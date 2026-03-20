#ifndef Game_Object_hpp
#define Game_Object_hpp

class Game_Object {
public:
    double pos_x, pos_y;
    double width, height;
    bool is_alive = true;

    Game_Object(double x, double y) : pos_x(x), pos_y(y), width(0), height(0) {}
    virtual ~Game_Object() {}

    virtual void physics() = 0;
    virtual void render() = 0;
};

#endif