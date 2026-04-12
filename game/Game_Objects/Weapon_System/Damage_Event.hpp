#ifndef DAMAGE_EVENT_HPP
#define DAMAGE_EVENT_HPP

enum class DamageType {
    PHYSICAL,  // Срезается бронёй
    MAGICAL,   // Срезается магическим сопротивлением
    PURE       // Не срезается ничем
};

class Game_Object;

struct Damage_Event {
    Game_Object* attacker;
    Game_Object* victim;
    
    float damage;           // Базовое значение урона
    float penetration;      // Пробитие брони (игнорирует X единиц брони)
    DamageType type;        // Тип урона (физ/маг/чистый)
    
    float kx, ky;           // Вектор отброса
    bool is_projectile;     // Флаг снаряда

    bool is_crit = false;       // Флаг для визуальных эффектов
    float crit_mult = 1.5f;     // Множитель (можно вынести в чипы)
};

#endif