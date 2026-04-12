#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <iostream>
#include <vector>
#include <ctime>
#include <algorithm>
#include <string>
#include <memory>
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
#include "Binds.hpp"
#include "Visuals/Renderer.hpp"
#include "Game_Objects/Game_Object.hpp"
#include "Game_Objects/Player/Player.hpp"
#include "World/Room.hpp"
#include "World/Floor_Manager.hpp"
#include "World/Room_Templates.hpp"
#include "Visuals/Assets.hpp"
#include "Visuals/Map_Renderer.hpp"
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
        std::function<std::unique_ptr<Game_Object>(float, float)> create;
    };

    std::vector<EnemyType> pool;

    EnemyDirector() {
        pool.push_back({0, 10, 1, [](float x, float y) { 
            auto e = std::make_unique<Dummy>(x, y);
            e->is_enemy = true;
            return e;
        }});
        pool.push_back({1, 15, 1, [](float x, float y) { 
            auto e = std::make_unique<Crawling_Dummy>(x, y);
            e->is_enemy = true;
            return e;
        }});
        pool.push_back({2, 25, 1, [](float x, float y) { 
            auto e = std::make_unique<Stalker>(x, y);
            e->is_enemy = true;
            return e;
        }});
    }

    EnemyType* get_random(int budget, int floor) {
        std::vector<EnemyType*> possible;
        for (auto& et : pool) {
            if (et.cost <= budget && floor >= et.min_floor) possible.push_back(&et);
        }
        if (possible.empty()) return nullptr;
        return possible[rand() % possible.size()];
    }

    EnemyType* get_by_id(int id) {
        for (auto& et : pool) if (et.id == id) return &et;
        return nullptr;
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
        for (auto& obj : Screen::objects) {
            if (obj && obj->is_alive && obj->is_enemy) return false;
        }
        return true;
    }

    void clear_non_player_objects() {
        // unique_ptr автоматически удалит удалены объекты размещённые
        active_boss = nullptr;
        
        Screen::objects.erase(
            std::remove_if(Screen::objects.begin(), Screen::objects.end(), 
            [&](const std::unique_ptr<Game_Object>& obj) {
                // Удаляем всё, что НЕ является игроком
                return obj.get() != Screen::player;
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
                auto boss = std::make_unique<Mega_Dummy>(320 - 40, 180 - 40);
                boss->is_enemy = true;
                boss->is_boss = true;
                boss->is_alive = true;
                boss->init_ui(); 
                
                active_boss = boss.get();
                Screen::spawn(std::move(boss));
            }
            return;
        }

        // Логика для обычных врагов
        EnemyDirector director;

        // Если в шаблоне уже прописаны конкретные враги
        if (!data.preset_enemies.empty()) {
            for (auto& p : data.preset_enemies) {
                auto et = director.get_by_id(p.type_id);
                if (et && et->create) {
                    Screen::spawn(et->create(p.x, p.y));
                }
            }
            return;
        }

        // Случайная генерация по бюджету
        int floor = floor_manager.get_current_floor(); 
        int budget = 15 + (floor * 10) + (rand() % 15);

        // Пытаемся заспавнить до 50 врагов, пока хватает бюджета
        for(int i = 0; i < 50 && budget >= 10; i++) {
            auto et = director.get_random(budget, floor);
            if (!et) break;

            // Выбираем случайную плитку (tx: 2-18, ty: 2-9), чтобы не спавнить в стенах
            int tx = rand() % 16 + 2; 
            int ty = rand() % 7 + 2;
            
            float wx = tx * 32.0f; 
            float wy = ty * 32.0f;

            // Считаем дистанцию до игрока, чтобы не спавнить врага прямо на голову
            float dist = hypot(wx - Screen::player->pos_x, wy - Screen::player->pos_y);

            // Проверяем: плитка пустая (0) И игрок далеко (150 пикселей)
            if (current_room.tiles[ty][tx] == 0 && dist > 150.0f) {
                Screen::spawn(et->create(wx, wy));
                budget -= et->cost;
            }
        }
    }
public:
    GameScreen() {
        if(!al_init_font_addon()) std::cout << "ERROR: Font addon failed" << std::endl;
        if(!al_init_ttf_addon()) std::cout << "ERROR: TTF addon failed" << std::endl;
        if(!al_init_primitives_addon()) std::cout << "ERROR: Primitives addon failed" << std::endl;

        Binds::init_defaults();

        gui_font = al_create_builtin_font();
        if (!gui_font) {
            std::cout << "ERROR: Failed to create builtin font!" << std::endl;
            return;
        }
        Assets::main_font = gui_font; 
        al_hide_mouse_cursor(al_get_current_display());

        floor_manager.generate_floor(1); 
        current_room.load_template(floor_manager.get_current_room_data());
        
        auto player = std::make_unique<Player>(WIDTH / 2, HEIGHT / 2);
        Screen::player = player.get();
        Screen::spawn(std::move(player));
        
        spawn_enemies();
    }

    ~GameScreen() {
        if (gui_font) al_destroy_font(gui_font);
    }

    void physics_process() override {
        if (!Screen::player) return;

        // 1. Добавление и обновление объектов
        if (!Screen::to_add.empty()) {
            // Перемещаем объекты из to_add в objects
            for (auto& obj : Screen::to_add) {
                Screen::objects.push_back(std::move(obj));
            }
            Screen::to_add.clear();
        }

        // 2. Физика для каждого объекта
        for (auto& obj : Screen::objects) {
            if (!obj || !obj->is_alive) continue;

            if (obj.get() == Screen::player) {
                // Явно вызываем физику игрока с комнатой
                Screen::player->physics(current_room); 
            } else {
                // Все остальные (враги, пули)
                obj->physics();
            }
        }

        // 3. Удаление мертвых объектов
        //unique_ptr удалит объекты автоматически при erase
        Screen::objects.erase(
            std::remove_if(Screen::objects.begin(), Screen::objects.end(), 
            [&](const std::unique_ptr<Game_Object>& obj) {
                if (!obj->is_alive) {
                    if (obj.get() == Screen::player) Screen::player = nullptr;
                    if (obj.get() == active_boss) active_boss = nullptr;
                    return true;
                }
                return false;
            }),
            Screen::objects.end()
        );

        // 4. Логика зачистки комнаты
        RoomData& current_data = floor_manager.get_current_room_data();
        bool clear = is_room_clear();
        if (!current_data.is_cleared && clear) {
            current_data.is_cleared = true;
        }

        // 5. Координаты игрока на сетке тайлов
        int tx = (int)(Screen::player->pos_x + 16) / 32; 
        int ty = (int)(Screen::player->pos_y + 16) / 32;

        // Если игрок больше не стоит на тайле двери (2), разрешаем переход снова
        if (tx >= 0 && tx < 20 && ty >= 0 && ty < 11) {
            if (current_room.tiles[ty][tx] != 2) {
                is_transitioning = false;
            }
        }

        // 6. Логика перехода в другую комнату
        if (tx >= 0 && tx < 20 && ty >= 0 && ty < 11) {
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

    // --- 4. КУРСОР ---
    ALLEGRO_MOUSE_STATE ms;
    al_get_mouse_state(&ms);
    float mx = ms.x / scale;
    float my = ms.y / scale;

    crosshair_anim += CH_SPEED;
    if (crosshair_anim > 1.0f) crosshair_anim = 0.0f;

    Renderer::draw_crosshair(mx, my, Screen::objects, crosshair_anim);

    MapRenderer::draw(floor_manager, al_key_down(&ks, ALLEGRO_KEY_TAB), WIDTH, HEIGHT);
}

int main(int argc, char **argv) {
    srand(time(NULL));
    if(!al_init()) return -1;

    Templates::Init();
    run_room_validation(); 
    
    std::cout << "DEBUG: Launching Game..." << std::endl;
    auto game = std::make_unique<GameScreen>();
    game->start();
    
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
    
    auto game = std::make_unique<GameScreen>();
    game->start();
    
    return 0;
}   