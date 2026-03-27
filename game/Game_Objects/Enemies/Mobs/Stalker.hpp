#ifndef STALKER_HPP
#define STALKER_HPP

#include "../../Game_Object.hpp"
#include <cmath>

enum StalkerState { HUNT, PREPARE, DASH, COOLDOWN };

class Stalker : public Game_Object {
public:
    StalkerState state = HUNT;
    float state_timer = 0;
    float angle = 0;

    // Константы баланса
    const float SPEED_HUNT    = 2.2f;
    const float SPEED_DASH    = 9.0f;
    const float DIST_AGGRO    = 110.0f; // Дистанция начала атаки
    const float DIST_LOST     = 250.0f; // Дистанция потери цели
    const float DIST_HIT      = 20.0f;  // Радиус нанесения урона
    const float TIME_PREPARE  = 0.4f;   // Замах
    const float TIME_DASH     = 0.35f;  // Длительность рывка
    const float TIME_COOLDOWN = 1.2f;   // Отдых после атаки
    const float KNOCKBACK_FORCE = 8.0f; // Сила отлета моба от пули

    float attack_damage = 15.0f; // Урон

    

    Stalker(float x, float y);

    void take_damage(float damage, float kx, float ky) override;
    
    void physics() override;
    void render() override;
};

#endif