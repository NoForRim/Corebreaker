#ifndef PAUSE_MENU_HPP
#define PAUSE_MENU_HPP

#include <vector>
#include <string>
#include <iostream>

class PauseMenu {
    int selected_index = 0;
    std::vector<std::string> options = { "RESUME", "SETTINGS", "MAIN MENU", "EXIT" };
    bool active = false;

public:
    void toggle() { active = !active; selected_index = 0; }
    bool is_active() const { return active; }

    void update(InputManager& input) {
        if (!active) return;

        if (input.is_key_pressed(KEY_UP)) 
            selected_index = (selected_index - 1 + options.size()) % options.size();
        
        if (input.is_key_pressed(KEY_DOWN)) 
            selected_index = (selected_index + 1) % options.size();

        if (input.is_key_pressed(KEY_ENTER)) handle_selection();
    }

    void draw(Renderer& ren) {
        if (!active) return;
        // Полупрозрачный фон на весь экран
        ren.draw_rect_filled(0, 0, ren.width, ren.height, {0, 0, 0, 180});

        for (int i = 0; i < (int)options.size(); ++i) {
            Color color = (i == selected_index) ? Color::YELLOW : Color::WHITE;
            ren.draw_text(ren.width / 2 - 60, 250 + i * 45, options[i], color);
        }
    }

private:
    void handle_selection() {
        switch (selected_index) {
            case 0: active = false; break;
            case 3: exit(0); break;
            default: break; 
        }
    }
};

#endif