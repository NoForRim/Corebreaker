#ifndef CONSOLE_HPP
#define CONSOLE_HPP

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>

#include "../../Game_Objects/Player/Player.hpp"
#include "../../Game_Objects/Items/Chips.hpp"

enum class CommandType { SET, GIVE, KILL, UNKNOWN };
enum class PlayerParam { HP, MAX_HP, SPEED, ARMOR, DAMAGE, FIRE_RATE, UNKNOWN };
enum class EffectType  { DAMAGE, PEN, SPEED, ACC, CRIT, UNKNOWN };

class GameConsole {
    std::string input_buffer;
    std::vector<std::string> log;
    bool active = false;
    Player* player = nullptr;

    // Вспомогательные функции маппинга (строка -> Enum)
    std::string toLower(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return std::tolower(c); });
        return s;
    }

    CommandType strToCmd(std::string s) {
        s = toLower(s);
        if (s == "set") return CommandType::SET;
        if (s == "give") return CommandType::GIVE;
        if (s == "kill") return CommandType::KILL;
        return CommandType::UNKNOWN;
    }

    PlayerParam strToParam(std::string s) {
        s = toLower(s);
        if (s == "hp") return PlayerParam::HP;
        if (s == "max_hp") return PlayerParam::MAX_HP;
        if (s == "speed") return PlayerParam::SPEED;
        if (s == "armor") return PlayerParam::ARMOR;
        if (s == "damage") return PlayerParam::DAMAGE;
        if (s == "fire_rate") return PlayerParam::FIRE_RATE;
        return PlayerParam::UNKNOWN;
    }

    EffectType strToEffect(std::string s) {
        s = toLower(s);
        if (s == "damage" || s == "dmg") return EffectType::DAMAGE;
        if (s == "pen") return EffectType::PEN;
        if (s == "speed") return EffectType::SPEED;
        if (s == "acc") return EffectType::ACC;
        if (s == "crit") return EffectType::CRIT;
        return EffectType::UNKNOWN;
    }

public:
    void init(Player* p) { player = p; }
    void toggle() { active = !active; }
    bool is_active() const { return active; }

    void print(const std::string& msg) {
        log.push_back(msg);
        if (log.size() > 14) log.erase(log.begin());
    }

    // Обработка событий ввода для Allegro
    void handle_event(ALLEGRO_EVENT& ev) {
        if (!active) return;

        if (ev.type == ALLEGRO_EVENT_KEY_CHAR) {
            if (ev.keyboard.keycode == ALLEGRO_KEY_ENTER) {
                if (!input_buffer.empty()) {
                    process_command(input_buffer);
                    input_buffer.clear();
                }
            } else if (ev.keyboard.keycode == ALLEGRO_KEY_BACKSPACE) {
                if (!input_buffer.empty()) input_buffer.pop_back();
            } else if (ev.keyboard.unichar >= 32 && ev.keyboard.unichar <= 126) {
                input_buffer += (char)ev.keyboard.unichar;
            }
        }
    }

    void draw(ALLEGRO_FONT* font) {
        if (!active) return;

        // Рисуем плашку консоли (используем константы из Engine для ширины)
        al_draw_filled_rectangle(0, 0, 640, 200, al_map_rgba(15, 15, 15, 230));
        
        for (int i = 0; i < (int)log.size(); ++i) {
            al_draw_text(font, al_map_rgb(180, 180, 180), 10, 10 + i * 14, 0, log[i].c_str());
        }

        std::string prompt = "> " + input_buffer + "_";
        al_draw_text(font, al_map_rgb(0, 255, 0), 10, 180, 0, prompt.c_str());
    }

private:
    void process_command(const std::string& line) {
        log.push_back("] " + line);
        std::stringstream ss(line);
        std::string cmd;
        std::vector<std::string> args;
        while (ss >> cmd) args.push_back(cmd);

        if (args.empty()) return;

        switch (strToCmd(args[0])) {
            case CommandType::SET:  cmd_set(args); break;
            case CommandType::GIVE: cmd_give(args); break;
            case CommandType::KILL: if(player) player->take_damage(9999); break;
            default: print("Unknown command."); break;
        }
    }

    void cmd_set(const std::vector<std::string>& args) {
        if (args.size() < 3 || !player) return;
        try {
            float val = std::stof(args[2]);
            switch (strToParam(args[1])) {
                case PlayerParam::HP: player->hp = val; break;
                case PlayerParam::MAX_HP: player->max_hp = val; break;
                case PlayerParam::SPEED: player->move_speed = val; break;
                case PlayerParam::DAMAGE: player->stats.base_damage = val; break;
                default: print("Param not found."); break;
            }
        } catch (...) { print("Invalid value."); }
    }

    void cmd_give(const std::vector<std::string>& args) {
        if (args.size() < 5 || args[1] != "chip" || !player) return;
        try {
            int slot = std::stoi(args[2]);
            int lvl  = std::stoi(args[3]);
            ChipEffect* eff = nullptr;

            switch (strToEffect(args[4])) {
                case EffectType::DAMAGE: eff = new DamageEffect(); break;
                case EffectType::PEN:    eff = new PenEffect(); break;
                case EffectType::SPEED:  eff = new SpeedEffect(); break;
                case EffectType::ACC:    eff = new AccuracyEffect(); break;
                case EffectType::CRIT:   eff = new CritEffect(); break;
                default: break;
            }

            if (eff && slot >= 0 && slot < 4) {
                if (player->inventory.chips[slot]) delete player->inventory.chips[slot];
                player->inventory.chips[slot] = new Chip(eff);
                player->inventory.chips[slot]->level = lvl;
                print("Granted " + player->inventory.chips[slot]->get_dynamic_name());
            }
        } catch (...) { print("Invalid args."); }
    }
};

#endif