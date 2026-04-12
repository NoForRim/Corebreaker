#ifndef Bullet_hpp
#define Bullet_hpp

#include "../Game_Object.hpp"
#include "../Weapon_System/Damage_Event.hpp"

// Параметры пули, которые задаёт оружие
struct BulletProps {
    float damage;
    float penetration;
    DamageType type;       // Тип урона (физ/маг/чистый)
    float speed;
    float size;
    ALLEGRO_COLOR color;
};

class Bullet : public Game_Object {
public:
    float damage;
    float penetration;
    float size; 
    ALLEGRO_COLOR color;
    DamageType type;

    // Конструктор принимает структуру свойств
    Bullet(float x, float y, float target_x, float target_y, BulletProps props);

    void physics() override;
    void render() override;
};

#endif