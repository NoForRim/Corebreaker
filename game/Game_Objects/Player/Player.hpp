#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "../Game_Object.hpp"
#include <string>

// Предварительное объявление класса, чтобы не было ошибок "not declared"
class Room; 

class Player : public Game_Object {
private:
    ALLEGRO_BITMAP* hp_cache = nullptr;
public:
    // --- НАСТРОЙКИ БАЛАНСА ---
    const float PLAYER_MAX_HP = 100.0f;
    const float PLAYER_SPEED  = 4.5f;
    const float DEFAULT_DAMAGE = 20.0f;
    const int   FIRE_COOLDOWN = 15;

    // Rally System переменные
    float hp = 100.0f;
    float max_hp = 100.0f;         // Добавляем максимум
    float recoverable_hp = 0.0f;   
    float rally_timer = 0.0f;      
    float invul_timer = 0.0f;
    
    float current_damage;
    int fire_cooldown = 0;

    //Dash

    std::string hp_string = "";
    bool hp_needs_update = true; // По умолчанию true, чтобы отрисовать первый раз

    void update_hp_cache(ALLEGRO_FONT* font); // Метод для обновления кэша
    ALLEGRO_BITMAP* get_hp_cache() { return hp_cache; }


    Player(double x, double y);

   virtual void take_damage(float damage, float kx, float ky) override;
    
    void physics(Room &room);
    void move(double move_x, double move_y, Room &room);
    
    void render() override;
    bool is_alive_check(){
        return hp > 0 && is_alive;
    }; 
};

#endif