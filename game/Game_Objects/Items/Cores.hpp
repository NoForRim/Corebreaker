#ifndef CORES_HPP
#define CORES_HPP

#include <string>
#include <vector>
#include "../Weapon_System/Damage_Event.hpp" 

enum class ShootType { SEMI, AUTO, BURST, SPREAD, RAIL, BEAM };

struct WeaponStats {
    float damage = 0;
    float fire_rate = 0;
    float bullet_speed = 0;
    float penetration = 0;
    float crit_chance = 0.05f;
    float spread = 0.0f;
    DamageType d_type = DamageType::PHYSICAL; 
};

struct WeaponCore {
    std::string name;
    ShootType s_type;
    float base_damage;
    float base_fire_rate;
    float base_speed;
    std::vector<DamageType> compatible_types;

    WeaponCore(std::string n, ShootType st, float d, float fr, float spd) 
        : name(n), s_type(st), base_damage(d), base_fire_rate(fr), base_speed(spd) {
        compatible_types = { DamageType::PHYSICAL, DamageType::MAGICAL };
    }
};

struct EnergyCore {
    std::string name;
    DamageType type;
    float damage_mult;

    EnergyCore(std::string n, DamageType t, float mult = 1.0f) 
        : name(n), type(t), damage_mult(mult) {}
};

#endif