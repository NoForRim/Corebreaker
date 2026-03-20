#include <vector>
#include "Game_Objects/Enemies/Dummy.hpp"
#include "Game_Objects/Enemies/Crawling_Dummy.hpp"
#include "../engine.hpp"
#include "World.hpp"
#include "Game_Objects/Game_Object.hpp"
#include "Game_Objects/Player/Player.hpp"

// Инициализация вектора 
std::vector<Game_Object*> Screen::objects;

class GameScreen : public Engine, public Screen { 
public:
    void physics_process() override {
        for (int i = 0; i < objects.size(); i++) {
            objects[i]->physics();
            if (!objects[i]->is_alive) {
                delete objects[i];
                objects.erase(objects.begin() + i);
                i--;
            }
        }
    }

    void render_process() override {
    // 1. Настраиваем масштабирование (один раз)
    ALLEGRO_TRANSFORM t;
    al_identity_transform(&t);
    
    float dw = (float)al_get_display_width(al_get_current_display());
    float dh = (float)al_get_display_height(al_get_current_display());
    float sw = dw / WIDTH;
    float sh = dh / HEIGHT;
    float scale = (sw < sh) ? sw : sh;
    
    al_scale_transform(&t, scale, scale);
    al_use_transform(&t);

    // 2. Сначала рисуем все игровые объекты
    for (auto obj : objects) {
        obj->render();
    }

    // 3. Работаем с мышью
    ALLEGRO_MOUSE_STATE mouse;
    al_get_mouse_state(&mouse);

    // Переводим координаты мыши из экранных (напр. 1920) в игровые (напр. 640)
    float cx = mouse.x / scale;
    float cy = mouse.y / scale;

    // 4. Рисуем прицел (используем cx и cy)
    float half_size = 5; 
    float depth = 3;    
    ALLEGRO_COLOR white = al_map_rgb(255, 255, 255);

    float x1 = cx - half_size; float y1 = cy - half_size;
    float x2 = cx + half_size; float y2 = cy + half_size;

    // Левый верхний
    al_draw_line(x1, y1, x1 + depth, y1, white, 1.0);
    al_draw_line(x1, y1, x1, y1 + depth, white, 1.0);
    // Правый верхний
    al_draw_line(x2, y1, x2 - depth, y1, white, 1.0);
    al_draw_line(x2, y1, x2, y1 + depth, white, 1.0);
    // Левый нижний
    al_draw_line(x1, y2, x1 + depth, y2, white, 1.0);
    al_draw_line(x1, y2, x1, y2 - depth, white, 1.0);
    // Правый нижний
    al_draw_line(x2, y2, x2 - depth, y2, white, 1.0);
    al_draw_line(x2, y2, x2, y2 - depth, white, 1.0);
}
    GameScreen() {
        al_hide_mouse_cursor(NULL);
        
        spawn(new Player(WIDTH / 2, HEIGHT / 2));

        spawn(new Dummy(100, 100));
        spawn(new Dummy(500, 150));
        spawn(new Dummy(300, 50));
        spawn(new Dummy(200, 300));

        spawn(new Crawling_Dummy(50, 50));
        spawn(new Crawling_Dummy(600, 300));
    }
};

int main(int argc, char **argv) {
    GameScreen game;
    game.start();
    return 0;
}