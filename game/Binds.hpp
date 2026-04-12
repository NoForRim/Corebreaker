#ifndef BINDS_HPP
#define BINDS_HPP

#include <allegro5/allegro.h>
#include <map>

enum class Action {
    MOVE_UP,
    MOVE_DOWN,
    MOVE_LEFT,
    MOVE_RIGHT,
    DASH,         
    ATTACK,        // Стрельба (клавиатурный дублёр)
    PLAYER_STATS, 
    OPEN_MAP,
    DEBUG_ADD_SIGNAL,
    DEBUG_REM_SIGNAL,
    DEBUG_ADD_NOISE,
    DEBUG_REM_NOISE,
    DEBUG_FLUSH_ALL
};

class Binds {
public:
    static inline std::map<Action, int> keys;

    static void init_defaults() {
        keys[Action::MOVE_UP]    = ALLEGRO_KEY_W;
        keys[Action::MOVE_DOWN]  = ALLEGRO_KEY_S;
        keys[Action::MOVE_LEFT]  = ALLEGRO_KEY_A;
        keys[Action::MOVE_RIGHT] = ALLEGRO_KEY_D;

        keys[Action::DASH]         = ALLEGRO_KEY_LSHIFT; // Левый Шифт
        keys[Action::ATTACK]       = ALLEGRO_KEY_SPACE;  // Пробел как альтернатива мышке
        keys[Action::OPEN_MAP]     = ALLEGRO_KEY_TAB;
        keys[Action::PLAYER_STATS] = ALLEGRO_KEY_C;

        keys[Action::DEBUG_ADD_SIGNAL] = ALLEGRO_KEY_1;
        keys[Action::DEBUG_REM_SIGNAL] = ALLEGRO_KEY_2;
        keys[Action::DEBUG_ADD_NOISE]  = ALLEGRO_KEY_3;
        keys[Action::DEBUG_REM_NOISE]  = ALLEGRO_KEY_4;
        keys[Action::DEBUG_FLUSH_ALL]  = ALLEGRO_KEY_5;
    }

    static bool is_pressed(Action action) {
        if (keys.find(action) == keys.end()) return false;
        ALLEGRO_KEYBOARD_STATE ks;
        al_get_keyboard_state(&ks);
        return al_key_down(&ks, keys[action]);
    }
};

#endif