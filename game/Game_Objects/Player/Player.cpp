#include "Player.hpp"

Player player;

void Player::move(double move_x, double move_y) {
    pos_x += move_x;
    pos_y += move_y;
}

class Screen : public Engine {
    public:
        void physics_process() override {
            //keyboard init
            ALLEGRO_KEYBOARD_STATE state;
            al_get_keyboard_state(&state);


            if (al_key_down(&state, ALLEGRO_KEY_W)){player.move(0, -10);}
            if (al_key_down(&state, ALLEGRO_KEY_A)){player.move(-10, 0);}
            if (al_key_down(&state, ALLEGRO_KEY_S)){player.move(0, 10);}
            if (al_key_down(&state, ALLEGRO_KEY_D)){player.move(10, 0);}
        }
        void render_process() override {
            al_draw_filled_rectangle(Player_Size-(x_pos/2), Player_Size-(y_pos/2),
                                 125, 125,
                                 {255, 0, 0, 0});
        }
};
