#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "../Game_Object.hpp"
#include <string>
#include "../Items/Chips.hpp"

class Room; 

enum class HUDStyle {
    COMPACT,      // Тонкая полоска, текст в одну строку
    DETAILED      // Высокий бар, текст в две строки
};

class Player : public Game_Object {
private:
    ALLEGRO_BITMAP* hp_cache = nullptr;
    ALLEGRO_BITMAP* stats_cache = nullptr;

    // --- Внутренняя логика обновления (Таймеры и Отрисовка) ---
    void update_timers(float dt);
    void handle_rally(float dt);
    
    void draw_dash_bar();             // Отрисовка деш-бара
    void draw_player_body();          // Отрисовка тела персонажа

    // --- Параметры взлома ---
    
    // Сигнал (полезный)
    float signal_corruption = 0.0f; 
    // Шум (мусорный взлом от урона/предметов)
    float noise_corruption = 0.0f;  
    
    // Итоговый процент (0.0 - 1.0)
    float total_corruption_percent = 0.0f;

    // Метод для пересчета влияния взлома на статы
    void update_corruption_logic();


    // --- Параметры деша ---
    int   dash_charges = 2;                
    const int   MAX_DASH_CHARGES = 2;      
    float dash_regen_timer = 0.0f;         
    const float DASH_REGEN_TIME = 1.5f;    

    float dash_timer = 0.0f;               
    const float DASH_DURATION = 0.15f;     
    const float DASH_SPEED_MULT = 3.0f;    

    // --- Настройки UI для Дэш-бара ---
    const float DASH_BAR_WIDTH = 50.0f;  
    const float DASH_BAR_HEIGHT = 8.0f;  
    const float DASH_BAR_OFFSET_X = -12.0f; 
    const float DASH_BAR_OFFSET_Y = -15.0f; 

    // --- Граница экрана и размеры ---
    const float SCREEN_WIDTH = 640.0f;
    const float SCREEN_HEIGHT = 360.0f;

    // Цвета 
    ALLEGRO_COLOR COLOR_DASH_BG;      
    ALLEGRO_COLOR COLOR_DASH_FRAME;   
    ALLEGRO_COLOR COLOR_DASH_FILL;    
    ALLEGRO_COLOR COLOR_DASH_REGEN;   

public:
    Player(double x, double y);
    ~Player();
    
    HUDStyle current_hud_style = HUDStyle::COMPACT;

    // --- Настройки баланса ---
    float base_max_hp = 500.0f;     // v1 hp. Максимальная капа хп
    
    const float PLAYER_SPEED  = 4.5f;
    const float DEFAULT_DAMAGE = 20.0f;
    const int   FIRE_COOLDOWN = 15;
    const float TILE_SIZE = 32.0f;
    const float ENEMY_SPAWN_DISTANCE = 150.0f;

    // --- Чипы ---
    Chip* chips[4] = {nullptr, nullptr, nullptr, nullptr};

    // Статы, которые будут "плавать" в зависимости от чипов
    float current_penetration = 0.0f;
    float current_fire_rate_mult = 1.0f; // Множитель задержки выстрела

    // Метод пересчета (вызывать при смене чипа)
    void recalculate_stats();

    // Рюкзак (хранилище)
    bool show_inventory = false;
    std::vector<Chip*> inventory;
    const size_t MAX_INV_SIZE = 12;

    // Методы инвентаря
    bool pick_up_chip(Chip* new_chip);   // Подобрать чип с земли
    void equip_chip(int inv_idx, int slot_idx); // Из рюкзака в слот
    void discard_chip(int inv_idx);      // Выбросить/удалить

    // --- Состояние персонажа ---
    float max_hp = 100.0f;          //v2 hp. Капа хп, до которой игрок может отхилиться
    float hp = 100.0f;              //v3 hp. Текущее хп
    float recoverable_hp = 0.0f;   
    float rally_timer = 0.0f;      
    float invul_timer = 0.0f;
    
    float current_damage;
    int   fire_cooldown = 0;

    // --- Окно статистики ---
    bool show_stats = false;          
    bool stats_key_pressed = false;   
    bool hp_needs_update = true;      

    // --- Методы обработки ввода (БЕЗ key_state) ---
    void handle_movement(Room &room);
    void handle_dash_input();
    void handle_shooting();
    void handle_ui_input();

    // --- Методы изменения взлома ---

    void add_signal(float amount);
    void add_noise(float amount);
    void flush_corruption(float amount); // Очистка

    // Геттеры для UI
    float get_total_corruption() const { return total_corruption_percent; }
    float get_signal_part() const { return signal_corruption; }
    float get_noise_part() const { return noise_corruption; }
    float get_base_max_hp() const { return base_max_hp; } // Геттер для v1 hp

    // --- Кэширование UI ---
    void update_stats_cache(ALLEGRO_FONT* font);
    void update_hp_cache(ALLEGRO_FONT* font); 
    ALLEGRO_BITMAP* get_hp_cache() { return hp_cache; }

    // --- Вспомогательные переменные ---
    float last_dir_x = 1.0f; 
    float last_dir_y = 0.0f;
    float dash_ui_scale = 1.0f;
    std::string hp_string = "";

    // --- Основные игровые методы ---
    virtual void take_damage(float damage, float kx, float ky) override;
    void physics(Room &room);
    void move(double move_x, double move_y, Room &room);
    void render() override;

    bool is_alive_check() {
        return hp > 0 && is_alive;
    }
};

#endif