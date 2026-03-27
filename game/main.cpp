#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <iostream>
#include <vector>
#include <ctime>
#include <algorithm>
#include <string> 
#include <allegro5/allegro_font.h>
#include <functional>
#include <cmath>

#include "Game_Objects/Enemies/Mobs/Dummy.hpp"
#include "Game_Objects/Enemies/Mobs/Crawling_Dummy.hpp"
#include "Game_Objects/Enemies/Mobs/Stalker.hpp"
#include "Game_Objects/Enemies/Bosses/Boss.hpp"
#include "Game_Objects/Enemies/Bosses/Mega_Dummy.hpp"
#include "engine.hpp"
#include "Screen.hpp"
#include "Visuals/Renderer.hpp"
#include "Game_Objects/Game_Object.hpp"
#include "Game_Objects/Player/Player.hpp"
#include "World/Room.hpp"
#include "World/Floor_Manager.hpp"
#include "World/Room_Templates.hpp"

/*
#ifdef main
#undef main
#endif
*/
// --- 1. Вспомогательные классы и функции ---

void run_room_validation() {
    std::cout << "\n=== STARTING ROOM TEMPLATES VALIDATION ===\n" << std::endl;
    for (auto const& [mask, variations] : Templates::Table) {
        for (size_t i = 0; i < variations.size(); ++i) {
            const auto& room = variations[i];
            bool has_error = false;
            
            bool tile_up    = (room.tiles[0][9] == 2 || room.tiles[0][10] == 2);
            bool tile_down  = (room.tiles[10][9] == 2 || room.tiles[10][10] == 2);
            bool tile_left  = (room.tiles[5][0] == 2);
            bool tile_right = (room.tiles[5][19] == 2);

            if (tile_up != (bool)(mask & UP)) has_error = true;
            if (tile_down != (bool)(mask & DOWN)) has_error = true;
            if (tile_left != (bool)(mask & LEFT)) has_error = true;
            if (tile_right != (bool)(mask & RIGHT)) has_error = true;

            if (has_error) {
                std::cout << "[!] ERROR Mask [" << mask << "] Var [" << i << "]" << std::endl;
            }
        }
    }
    std::cout << "=== VALIDATION FINISHED ===\n" << std::endl;
}

class EnemyDirector {
public:
    struct EnemyType {
        int id;
        int cost;
        int min_floor;
        std::function<Game_Object*(float, float)> create;
    };

    std::vector<EnemyType> pool;

    EnemyDirector() {
        pool.push_back({0, 10, 1, [](float x, float y) { 
            auto e = new Dummy(x, y); e->is_enemy = true; return e; 
        }});
        pool.push_back({1, 15, 1, [](float x, float y) { 
            auto e = new Crawling_Dummy(x, y); e->is_enemy = true; return e; 
        }});
        pool.push_back({2, 25, 1, [](float x, float y) { 
            auto e = new Stalker(x, y); e->is_enemy = true; return e;
        }});
    }

    EnemyType get_random(int budget, int floor) {
        std::vector<EnemyType*> possible;
        for (auto& et : pool) {
            if (et.cost <= budget && floor >= et.min_floor) possible.push_back(&et);
        }
        if (possible.empty()) return {-1, 0, 0, nullptr};
        return *possible[rand() % possible.size()];
    }

    EnemyType get_by_id(int id) {
        for (auto& et : pool) if (et.id == id) return et;
        return {-1, 0, 0, nullptr};
    }
};

// --- 2. Основной класс игры ---

class GameScreen : public Engine, public Screen {
private: 
    Room current_room; 
    FloorManager floor_manager;
    ALLEGRO_FONT* gui_font = nullptr; 
    Boss* active_boss = nullptr; 
    bool is_transitioning = false;

    float crosshair_anim = 0.0f;
    const float CH_SPEED      = 0.02f;  
    const float CH_BASE_SIZE  = 7.0f;   
    const float CH_GAP_FACTOR = 0.4f;   
    const float CH_THICKNESS  = 1.2f;   

    bool is_room_clear() {
        for (auto obj : Screen::objects) {
            if (obj && obj->is_alive && obj->is_enemy) return false;
        }
        return true;
    }

    void clear_non_player_objects() {
    active_boss = nullptr; // Сбрасываем указатель на босса
    
    Screen::objects.erase(
        std::remove_if(Screen::objects.begin(), Screen::objects.end(), [&](Game_Object* obj) {
            // Удаляем всё, что НЕ является игроком
            if (obj != Screen::player) {
                delete obj;
                return true;
            }
            return false;
        }), 
        Screen::objects.end()
    );
}
    void spawn_enemies() {
        RoomData& data = floor_manager.get_current_room_data();
        if (data.is_spawned || data.type == START || data.type == SHOP || data.type == TREASURE) return;

        data.is_spawned = true;

        if (data.type == BOSS) {
            if (!data.is_cleared) {
                active_boss = new Mega_Dummy(320 - 40, 180 - 40);
                active_boss->is_enemy = true;
                active_boss->is_boss = true;
                active_boss->is_alive = true;
                
                // ВАЖНО: готовим текст сразу после создания
                active_boss->init_ui(gui_font); 
                
                spawn(active_boss);
            }
            return;
        }

    // 2. ЛОГИКА ДЛЯ ОБЫЧНЫХ ВРАГОВ (Те самые 20+ строчек)
    EnemyDirector director; 

    // Если в шаблоне уже прописаны конкретные враги
    if (!data.preset_enemies.empty()) {
        for (auto& p : data.preset_enemies) {
            auto et = director.get_by_id(p.type_id);
            if (et.create) spawn(et.create(p.x, p.y));
        }
        return;
    }

    // Случайная генерация по бюджету
    int floor = floor_manager.get_current_floor(); 
    int budget = 15 + (floor * 10) + (rand() % 15);

    // Пытаемся заспавнить до 50 врагов, пока хватает бюджета
    for(int i = 0; i < 50 && budget >= 10; i++) {
        auto et = director.get_random(budget, floor);
        if (et.id == -1) break;

        // Выбираем случайную плитку (tx: 2-18, ty: 2-9), чтобы не спавнить в стенах
        int tx = rand() % 16 + 2; 
        int ty = rand() % 7 + 2;
        
        float wx = tx * 32.0f; 
        float wy = ty * 32.0f;

        // Считаем дистанцию до игрока, чтобы не спавнить врага прямо на голову
        float dist = hypot(wx - Screen::player->pos_x, wy - Screen::player->pos_y);

        // Проверяем: плитка пустая (0) И игрок далеко (150 пикселей)
        if (current_room.tiles[ty][tx] == 0 && dist > 150) {
            spawn(et.create(wx, wy));
            budget -= et.cost;
        }
    }
}
public:
    GameScreen() {
        if(!al_init_font_addon()) std::cout << "ERROR: Font addon failed" << std::endl;
        if(!al_init_primitives_addon()) std::cout << "ERROR: Primitives addon failed" << std::endl;

        gui_font = al_create_builtin_font(); 
        al_hide_mouse_cursor(al_get_current_display());

        floor_manager.generate_floor(1); 
        current_room.load_template(floor_manager.get_current_room_data());
        
        Player* p = new Player(WIDTH / 2, HEIGHT / 2);
        Screen::player = p; 
        spawn(p);
        spawn_enemies();
    }

    ~GameScreen() {
        if (gui_font) al_destroy_font(gui_font);
    }

    void physics_process() override {
        if (!Screen::player) return;

        // 1. Добавление и обновление объектов
        if (!Screen::to_add.empty()) {
            Screen::objects.insert(Screen::objects.end(), Screen::to_add.begin(), Screen::to_add.end());
            Screen::to_add.clear();
        }

        for (auto obj : Screen::objects) {
            if (!obj || !obj->is_alive) continue;

            if (obj == Screen::player) {
                // Явно вызываем физику игрока с комнатой
                Screen::player->physics(current_room); 
            } else {
                // Все остальные (враги, пули)
                obj->physics();
            }
        }
        // Удаление мертвых объектов
        auto it = Screen::objects.begin();
        while (it != Screen::objects.end()) {
            if (!(*it)->is_alive) {
                if (*it == Screen::player) Screen::player = nullptr;
                if (*it == active_boss) active_boss = nullptr;
                delete *it;
                it = objects.erase(it);
            } else ++it;
        }

        // 2. Логика зачистки комнаты
        RoomData& current_data = floor_manager.get_current_room_data();
        bool clear = is_room_clear();
        if (!current_data.is_cleared && clear) {
            current_data.is_cleared = true;
        }

        // 3. Координаты игрока на сетке тайлов
        int tx = (int)(Screen::player->pos_x + 16) / 32; 
        int ty = (int)(Screen::player->pos_y + 16) / 32;

        // Если игрок больше не стоит на тайле двери (2), разрешаем переход снова
        if (tx >= 0 && tx < 20 && ty >= 0 && ty < 11) {
            if (current_room.tiles[ty][tx] != 2) {
                is_transitioning = false;
            }
        }

        // 4. Логика перехода в другую комнату
        if (tx >= 0 && tx < 20 && ty >= 0 && ty < 11) {
            // Добавляем проверку !is_transitioning
            if (current_room.tiles[ty][tx] == 2 && clear && !is_transitioning) { 
                
                Side door_side = NONE;
                if (ty == 0) door_side = UP;
                else if (ty == 10) door_side = DOWN;
                else if (tx == 0) door_side = LEFT;
                else if (tx == 19) door_side = RIGHT;

                if (door_side != NONE && floor_manager.can_move(door_side)) {
                    // Блокируем повторный вход до выхода из зоны двери
                    is_transitioning = true; 

                    floor_manager.move(door_side, current_room);
                    
                    if (door_side == UP) Screen::player->pos_y = 9 * 32;
                    else if (door_side == DOWN) Screen::player->pos_y = 1 * 32;
                    else if (door_side == LEFT) Screen::player->pos_x = 18 * 32;
                    else if (door_side == RIGHT) Screen::player->pos_x = 1 * 32;

                    clear_non_player_objects();
                    spawn_enemies();
                }
            }
        }
    }
    void render_process() override {
    al_clear_to_color(al_map_rgb(15, 15, 20));
    
    // --- 1. РАСЧЕТ МАСШТАБА ---
    float sw = (float)al_get_display_width(al_get_current_display());
    float sh = (float)al_get_display_height(al_get_current_display());
    float scale = std::min(sw / WIDTH, sh / HEIGHT);

    ALLEGRO_TRANSFORM t_trans;
    al_identity_transform(&t_trans);
    al_scale_transform(&t_trans, scale, scale);
    al_use_transform(&t_trans);

    // --- 2. ИГРОВОЙ МИР ---
    Renderer::draw_world(current_room, Screen::objects);

    // --- 3. ИНТЕРФЕЙС (UI) ---
    Renderer::draw_player_hud(Screen::player, gui_font);
    Renderer::draw_boss_hud(active_boss);
    
    // Миникарта (оставляем вызов через floor_manager, так как она завязана на логику этажа)
    ALLEGRO_KEYBOARD_STATE ks;
    al_get_keyboard_state(&ks);
    floor_manager.draw_minimap(al_key_down(&ks, ALLEGRO_KEY_TAB), (float)WIDTH, (float)HEIGHT);

    // --- 4. КУРСОР ---
    ALLEGRO_MOUSE_STATE ms;
    al_get_mouse_state(&ms);
    float mx = ms.x / scale;
    float my = ms.y / scale;

    crosshair_anim += CH_SPEED;
    if (crosshair_anim > 1.0f) crosshair_anim = 0.0f;

    Renderer::draw_crosshair(mx, my, Screen::objects, crosshair_anim);
}

int main(int argc, char **argv) {
    srand(time(NULL));
    if(!al_init()) return -1;

    Templates::Init();
    run_room_validation(); 
    
    std::cout << "DEBUG: Launching Game..." << std::endl;
    GameScreen* game = new GameScreen();
    game->start();
    
    delete game;
    return 0;
}
};
#ifdef main
#undef main
#endif

// extern "C" помогает линковщику найти функцию в C++ проекте
extern "C" int main(int argc, char **argv) {
    srand(time(NULL));
    
    if(!al_init()) {
        std::cout << "Failed to init Allegro!" << std::endl;
        return -1;
    }

    Templates::Init();
    run_room_validation(); 
    
    std::cout << "DEBUG: Launching Game..." << std::endl;
    
    // Создаем игру в куче, чтобы не переполнять стек
    GameScreen* game = new GameScreen();
    game->start();
    
    delete game;
    return 0;
}   