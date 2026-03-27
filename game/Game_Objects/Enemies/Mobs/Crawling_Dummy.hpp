#ifndef Crawling_Dummy_hpp
#define Crawling_Dummy_hpp

#include "../Enemy.hpp" // ОБЯЗАТЕЛЬНО наследуемся от Enemy
#include <allegro5/allegro_primitives.h>

class Crawling_Dummy : public Enemy { // Меняем Game_Object на Enemy
private:
    static constexpr float INITIAL_HP = 60.0f;

public:
    // Передаем INITIAL_HP в конструктор родителя (Enemy)
    Crawling_Dummy(double x, double y) : Enemy(x, y, INITIAL_HP) {
        this->width = 20;
        this->height = 20;
        this->speed = 0.8; 
        // is_enemy = true уже ставится внутри конструктора Enemy
    }

    void physics() override;
    void render() override;
};

#endif