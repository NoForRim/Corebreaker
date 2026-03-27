#include "Dummy.hpp"
#include "../../Projectiles/Bullet.hpp" 
#include "../../Player/Player.hpp"      
#include "../../../Screen.hpp"

void Dummy::physics() {
    for (auto obj : Screen::objects) {
        if (!obj || obj == this || !obj->is_alive) continue;

        if (obj->pos_x > pos_x && obj->pos_x < pos_x + width &&
            obj->pos_y > pos_y && obj->pos_y < pos_y + height) {
            
            Bullet* bullet = dynamic_cast<Bullet*>(obj);
            if (bullet) {
                this->is_alive = false; 
                bullet->is_alive = false; 
                return; 
            }
        }
    }
}

void Dummy::render() {
    al_draw_filled_rectangle(pos_x, pos_y, pos_x + width, pos_y + height, 
                             al_map_rgb(255, 50, 50));
}