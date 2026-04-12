#ifndef COMBAT_SYSTEM_HPP
#define COMBAT_SYSTEM_HPP

#include "Damage_Event.hpp"
#include "../Game_Object.hpp"
#include "../Player/Player.hpp"
#include "../../Screen.hpp"
#include <algorithm>
#include <cmath>

class Combat_System {
public:
    static void handle_attack(const Damage_Event& event) {
        if (!event.victim || !event.victim->is_alive) return;

        // 1. Рассчитываем финальный урон с учетом брони, резистов и пробития
        float final_damage = calculate_final_damage(event);

        // 2. Наносим урон цели (вызывает take_damage у моба или игрока)
        event.victim->take_damage(final_damage, event.kx, event.ky);

        // 3. Если атакует игрок, обрабатываем механику Rally (лечение)
        if (event.attacker == (Game_Object*)Screen::player) {
            apply_rally(event, final_damage);
        }
    }

private:
    static float calculate_final_damage(const Damage_Event& event) {
        float raw = event.damage;
        Game_Object* target = event.victim;

        switch (event.type) {
            case DamageType::PHYSICAL: {
                // Пробитие вычитается из брони напрямую
                // effective_armor не может быть меньше 0
                float effective_armor = std::max(0.0f, target->armor - event.penetration);

                // Коэффициент снижения (EHP)
                // Формула: Damage * (1 / (1 + 0.06 * Armor))
                // Каждая единица брони дает +6% к "выносливости" против физ. урона
                float armor_constant = 0.06f;
                float multiplier = 1.0f / (1.0f + (armor_constant * effective_armor));
                
                return raw * multiplier;
            }

            case DamageType::MAGICAL: {
                // Для магии используем прямое мультипликативное снижение
                return raw * (1.0f - target->magic_res_base);
            }

            case DamageType::PURE: {
                // Чистый урон игнорирует все типы защит
                return raw;
            }
        }
        return raw;
    }

    static void apply_rally(const Damage_Event& event, float actual_damage) {
        Player* p = Screen::player;
        // Если игрока нет или восстанавливать нечего — выходим
        if (!p || p->recoverable_hp <= 0) return;

        // Фиксированное лечение
        float heal_amount = 3.0f; 
        
        // Лечим не больше, чем накоплено в оранжевом слое (recoverable_hp)
        float actual_heal = std::min(heal_amount, p->recoverable_hp);
        
        p->hp += actual_heal;
        p->recoverable_hp -= actual_heal;
        
        // Ограничиваем текущей практической капой (V2). 
        // Это не дает Rally "пробить" шрамы от взлома.
        if (p->hp > p->max_hp) {
            p->hp = p->max_hp;
        }
        
        // Сигнал HUD'у обновить числа и кэш
        p->hp_needs_update = true;
    }
};

#endif