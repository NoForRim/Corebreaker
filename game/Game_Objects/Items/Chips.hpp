#ifndef CHIPS_HPP
#define CHIPS_HPP

#include "Cores.hpp" // Содержит WeaponStats
#include <algorithm>
#include <vector>
#include <string>

// --- БАЛАНС: Коэффициенты скейлинга ---
namespace ChipBalance {
    const float DMG_MULT      = 0.07f;  // +7% за уровень
    const float PEN_FLAT      = 3.0f;   // +3 ед. за уровень
    const float SPEED_MULT    = 0.05f;  // -5% задержки за уровень
    const float SPREAD_FLAT   = 0.5f;   // -0.5 разброса за уровень
    const float CRIT_FLAT     = 0.03f;  // +3% крита за уровень
    
    // Новые параметры для интеграции с Player.hpp
    const float HP_FLAT        = 40.0f;  // +40 к max_hp (v2) за уровень
    const float ARMOR_FLAT     = 2.0f;   // +2 к armor за уровень
}

enum class ChipTarget { SLOT_A, SLOT_B, NONE };

// Базовый класс эффекта
struct ChipEffect {
    int priority; 
    virtual void apply(WeaponStats& stats, int level) = 0;
    // Для совместимости с Player, если эффект влияет не на оружие, а на тело
    virtual void apply_to_player(class Player* p, int level) {} 

    virtual std::string get_root_name() = 0;   
    virtual std::string get_suffix() = 0;      
    virtual std::string get_description(int level) = 0;
    virtual ~ChipEffect() {}
};

// --- ЭФФЕКТЫ ОРУЖИЯ ---

struct DamageEffect : public ChipEffect {
    DamageEffect() { priority = 100; }
    void apply(WeaponStats& stats, int level) override { stats.damage *= (1.0f + ChipBalance::DMG_MULT * level); }
    std::string get_root_name() override { return "Striker"; }
    std::string get_suffix() override { return "of Might"; }
    std::string get_description(int level) override { return "+" + std::to_string((int)(ChipBalance::DMG_MULT * 100 * level)) + "% DMG"; }
};

struct PenEffect : public ChipEffect {
    PenEffect() { priority = 90; }
    void apply(WeaponStats& stats, int level) override { stats.penetration += ChipBalance::PEN_FLAT * level; }
    std::string get_root_name() override { return "Piercer"; }
    std::string get_suffix() override { return "of Void"; }
    std::string get_description(int level) override { return "+" + std::to_string((int)(ChipBalance::PEN_FLAT * level)) + " PEN"; }
};

struct SpeedEffect : public ChipEffect {
    SpeedEffect() { priority = 80; }
    void apply(WeaponStats& stats, int level) override { stats.fire_rate *= (1.0f - ChipBalance::SPEED_MULT * level); }
    std::string get_root_name() override { return "Trigger"; }
    std::string get_suffix() override { return "of Haste"; }
    std::string get_description(int level) override { return "+" + std::to_string((int)(ChipBalance::SPEED_MULT * 100 * level)) + "% Speed"; }
};

// --- ЭФФЕКТЫ ИГРОКА (Body Mods) ---

struct HealthEffect : public ChipEffect {
    HealthEffect() { priority = 110; } // Высокий приоритет для имени
    void apply(WeaponStats& stats, int level) override {} // На оружие не влияет
    void apply_to_player(Player* p, int level) override; // Реализация в .cpp через Player.hpp
    
    std::string get_root_name() override { return "Tank"; }
    std::string get_suffix() override { return "of Life"; }
    std::string get_description(int level) override { return "+" + std::to_string((int)(ChipBalance::HP_FLAT * level)) + " Max HP"; }
};

struct ArmorEffect : public ChipEffect {
    ArmorEffect() { priority = 95; }
    void apply(WeaponStats& stats, int level) override {}
    void apply_to_player(Player* p, int level) override;

    std::string get_root_name() override { return "Guardian"; }
    std::string get_suffix() override { return "of Steel"; }
    std::string get_description(int level) override { return "+" + std::to_string((int)(ChipBalance::ARMOR_FLAT * level)) + " Armor"; }
};

// --- КЛАСС ЧИПА ---

class Chip {
public:
    int level = 1;
    ChipTarget target = ChipTarget::NONE;
    std::vector<ChipEffect*> effects;

    Chip(ChipEffect* initial_effect, int lvl = 1) : level(lvl) { 
        if (initial_effect) effects.push_back(initial_effect); 
    }
    
    ~Chip() { for (auto e : effects) delete e; }

    std::string get_dynamic_name() {
        if (effects.empty()) return "Broken Chip";
        
        // Сортируем по приоритету для формирования имени
        std::sort(effects.begin(), effects.end(), [](ChipEffect* a, ChipEffect* b) {
            return a->priority > b->priority;
        });

        std::string roman = to_roman(level);
        if (effects.size() >= 3) return "[Master] " + effects[0]->get_root_name() + " " + roman;
        if (effects.size() == 2) return effects[0]->get_root_name() + " " + effects[1]->get_suffix() + " " + roman;
        return effects[0]->get_root_name() + " " + roman;
    }

    // Применение к статам оружия (Combat_System)
    void apply_to_weapon(WeaponStats& stats) {
        for (auto e : effects) e->apply(stats, level);
    }

    // Применение к самому игроку (HP, Armor)
    void apply_to_player(class Player* p) {
        for (auto e : effects) e->apply_to_player(p, level);
    }

    void level_up() {
        if (level < 10) level++;
        // Эволюция чипа на 5 уровне
        if (level == 5 && effects.size() == 1) {
            // Добавляем случайный или фиксированный второй эффект
            effects.push_back(new PenEffect()); 
        }
    }

private:
    std::string to_roman(int n) {
        static const std::string table[] = {"", "I", "II", "III", "IV", "V", "VI", "VII", "VIII", "IX", "X"};
        return (n >= 1 && n <= 10) ? table[n] : std::to_string(n);
    }
};

#endif