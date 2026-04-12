#ifndef ASSETS_HPP
#define ASSETS_HPP

#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <map>
#include <string>

class Assets {
public:
    // Основной шрифт интерфейса
    inline static ALLEGRO_FONT* main_font = nullptr;

    // Метод для инициализации (вызывать в main или engine)
    static void load() {
        main_font = al_load_ttf_font("resources/fonts/main.ttf", -18, 0);
        // Здесь же можно грузить и другие ресурсы
    }

    // Метод для очистки (вызывать при выходе)
    static void cleanup() {
        if (main_font) al_destroy_font(main_font);
    }
};

#endif