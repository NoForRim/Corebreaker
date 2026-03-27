#ifndef Game_Object_hpp
#define Game_Object_hpp

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>

class Game_Object {
protected: 
    double vel_x = 0, vel_y = 0; 
    double speed = 0; 

public:
    double pos_x, pos_y;
    double width, height;
    
    float hp = 0; 
    float max_hp = 0;

    bool is_alive = true;
    bool is_enemy = false;
    bool is_boss = false; 

    Game_Object(double x, double y) : pos_x(x), pos_y(y), width(0), height(0) {}
    virtual ~Game_Object() {}

    virtual void init_ui(ALLEGRO_FONT* font) {}
    
    // Универсальный метод урона: величина, направление отброса по X и Y
    virtual void take_damage(float damage, float kx, float ky) {}
        
    virtual void physics() {} 
    virtual void render() = 0;
};
#endif