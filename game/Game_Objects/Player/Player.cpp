#include "Player.hpp"
#include <allegro5/allegro_primitives.h>
#include <allegro5/mouse.h>
#include <cmath>
#include <string>
#include "../Projectiles/Bullet.hpp" 
#include "../../World/Room.hpp" 
#include "../../Screen.hpp"
#include "../Weapon_System/Combat_System.hpp"
#include "../../Visuals/UI/UI_Utils.hpp"
#include "../../Visuals/Assets.hpp"
#include "../../Binds.hpp"

Player::Player(double x, double y) : Game_Object(x, y) {
    pos_x = x;
    pos_y = y;
    width = 20;
    height = 20;
    
    // Инициализация HP и урона
    this->hp = base_max_hp = 100.0f;
    this->current_damage = DEFAULT_DAMAGE; 
    this->fire_cooldown = 0;


    // Инициализация защиты
    this->max_hp = 100.0f;
    this->hp = this->max_hp;
    this->armor = 5.0f;             // Начальная броня (физ. защита)
    this->magic_res_base = 0.25f;   // 25% защиты от магии
    
    this->current_damage = 15.0f;   // Базовый урон
    this->fire_cooldown = 0;
    this->is_enemy = false;         // Игрок — не враг

    // Цвета интерфейса
    COLOR_DASH_BG    = al_map_rgb(40, 40, 40);      // Очень тёмно-серый
    COLOR_DASH_FRAME = al_map_rgb(255, 255, 255);   // Белый
    COLOR_DASH_FILL  = al_map_rgb(0, 190, 255);     // Яркий Циан
    COLOR_DASH_REGEN = al_map_rgb(0, 100, 150);     // Тёмно-синий/голубой
}

Player::~Player() {
    if (hp_cache) {
        al_destroy_bitmap(hp_cache);
        hp_cache = nullptr;
    }
}

void Player::update_hp_cache(ALLEGRO_FONT* font) {
    if (!font) return;

    int b_w = 180; // Ширина кэша
    int b_h = 32;  // Высота с запасом

    if (!hp_cache) hp_cache = al_create_bitmap(b_w, b_h);
    if (!hp_cache) return;

    float mid_x = b_w / 2.0f;

    ALLEGRO_BITMAP* prev_target = al_get_target_bitmap();
    al_set_target_bitmap(hp_cache);
    al_clear_to_color(al_map_rgba(0, 0, 0, 0)); 

    char buf_p[16], buf_v[32];
    snprintf(buf_p, sizeof(buf_p), "%d%%", (int)((hp / max_hp) * 100));
    snprintf(buf_v, sizeof(buf_v), "%.0f/%.0f", hp, max_hp);

    if (current_hud_style == HUDStyle::COMPACT) {
        UI_Utils::draw_text_shadow(font, al_map_rgb(255, 255, 255), mid_x, 0, ALLEGRO_ALIGN_CENTER, buf_p);
        UI_Utils::draw_text_shadow(font, al_map_rgb(200, 200, 200), b_w - 5, 0, ALLEGRO_ALIGN_RIGHT, buf_v);
    } else {
        UI_Utils::draw_text_shadow(font, al_map_rgb(255, 255, 255), mid_x, 1, ALLEGRO_ALIGN_CENTER, buf_p);
        UI_Utils::draw_text_shadow(font, al_map_rgb(220, 220, 220), mid_x, 15, ALLEGRO_ALIGN_CENTER, buf_v);
    }

    al_set_target_bitmap(prev_target);
}

void Player::update_stats_cache(ALLEGRO_FONT* font) {
    if (!font) return;

    int w = 200; 
    int h = 140; 

    if (!stats_cache) stats_cache = al_create_bitmap(w, h);
    if (!stats_cache) return;

    ALLEGRO_BITMAP* prev_target = al_get_target_bitmap();
    al_set_target_bitmap(stats_cache);
    al_clear_to_color(al_map_rgba(0, 0, 0, 0)); 

    // 1. Фон окна (темный полупрозрачный прямоугольник с рамкой)
    al_draw_filled_rounded_rectangle(0, 0, w, h, 5, 5, al_map_rgba(20, 20, 25, 200));
    al_draw_rounded_rectangle(0, 0, w, h, 5, 5, al_map_rgb(255, 255, 255), 1.2);

    // 2. Расчет процентов защиты
    float phys_reduct = (1.0f - (1.0f / (1.0f + 0.06f * armor))) * 100.0f;
    float mag_reduct = magic_res_base * 100.0f;

    // 3. Заголовок
    al_draw_text(font, al_map_rgb(0, 0, 0), w/2 + 1, 11, ALLEGRO_ALIGN_CENTER, "STATS");
    al_draw_text(font, al_map_rgb(255, 255, 0), w/2, 10, ALLEGRO_ALIGN_CENTER, "STATS");

    // 4. Отрисовка строк
    auto draw_stat_row = [&](const char* name, const char* val, int y) {
        // Тень для названий
        al_draw_text(font, al_map_rgb(0, 0, 0), 16, y + 1, 0, name);
        al_draw_text(font, al_map_rgb(200, 200, 200), 15, y, 0, name);
        // Тень для значений
        al_draw_text(font, al_map_rgb(0, 0, 0), w - 14, y + 1, ALLEGRO_ALIGN_RIGHT, val);
        al_draw_text(font, al_map_rgb(255, 255, 255), w - 15, y, ALLEGRO_ALIGN_RIGHT, val);
    };

    char b[32];
    snprintf(b, 32, "%.1f", current_damage);    draw_stat_row("Damage:", b, 40);
    snprintf(b, 32, "%.1f", armor);             draw_stat_row("Armor:", b, 60);
    snprintf(b, 32, "%d%%", (int)phys_reduct);  draw_stat_row("P. Reduct:", b, 80);
    snprintf(b, 32, "%d%%", (int)mag_reduct);   draw_stat_row("M. Resist:", b, 100);

    al_set_target_bitmap(prev_target);
}

void Player::move(double move_x, double move_y, Room &room) {
    // 1. Проверка по горизонтали (X)
    float next_x = pos_x + move_x;
    if (!room.is_wall(next_x, pos_y) && 
        !room.is_wall(next_x + width, pos_y) && 
        !room.is_wall(next_x, pos_y + height) && 
        !room.is_wall(next_x + width, pos_y + height)) 
    {
        pos_x = next_x;
    }

    // 2. Проверка по вертикали (Y)
    float next_y = pos_y + move_y;
    if (!room.is_wall(pos_x, next_y) && 
        !room.is_wall(pos_x + width, next_y) && 
        !room.is_wall(pos_x, next_y + height) && 
        !room.is_wall(pos_x + width, next_y + height)) 
    {
        pos_y = next_y;
    }

    // Ограничение экраном (используем константы)
    if (pos_x < 0) pos_x = 0;
    if (pos_y < 0) pos_y = 0;
    if (pos_x > SCREEN_WIDTH - width) pos_x = SCREEN_WIDTH - width;
    if (pos_y > SCREEN_HEIGHT - height) pos_y = SCREEN_HEIGHT - height;
}

void Player::take_damage(float damage, float kx, float ky) {
    // 1. Проверка на неуязвимость (I-frames)
    if (this->invul_timer > 0) return;

    // 2. Наносим ВЕСЬ урон сразу
    this->hp -= damage;
    if (this->hp < 0) this->hp = 0;

    // 3. Устанавливаем Rally (оранжевое здоровье)
    // Весь полученный урон теперь можно "отбить"
    this->recoverable_hp += damage; 
    if (this->recoverable_hp > this->max_hp) this->recoverable_hp = this->max_hp;
    
    // 4. Таймеры
    this->invul_timer = 0.6f; // Неуязвимость на полсекунды
    this->rally_timer = 2.0f; // Оранжевое здоровье не тает 2 секунды

    // 5. Отброс игрока (Knockback)
    this->vel_x = kx * 8.0f;
    this->vel_y = ky * 8.0f;

    // 6. Обновляем текстовый кэш HP
    this->hp_needs_update = true;
}

void Player::update_corruption_logic() {
    const float ABSOLUTE_MIN = 1.0f;
    const float GLOBAL_MAX = 500.0f;

    // 1. Ограничиваем базу (V1)
    if (this->base_max_hp < ABSOLUTE_MIN) this->base_max_hp = ABSOLUTE_MIN;
    if (this->base_max_hp > GLOBAL_MAX) this->base_max_hp = GLOBAL_MAX;

    // 2. Ограничиваем капу (V2)
    if (this->max_hp > this->base_max_hp) this->max_hp = this->base_max_hp;
    if (this->max_hp < ABSOLUTE_MIN) this->max_hp = ABSOLUTE_MIN;

    // 3. Лимитируем сами переменные взлома (чтобы не росли в бесконечность)
    // Сигнал и Шум не должны в сумме превышать 100% (условно 100 единиц)
    float total_cap = 100.0f; 
    if (this->signal_corruption > total_cap) this->signal_corruption = total_cap;
    if (this->noise_corruption > total_cap) this->noise_corruption = total_cap;

    // 4. Синхронизируем ХП
    if (this->hp > this->max_hp) {
        this->hp = this->max_hp;
        this->hp_needs_update = true;
    }

    this->total_corruption_percent = std::clamp((signal_corruption + noise_corruption) / 100.0f, 0.0f, 1.0f);
}

void Player::add_signal(float amount) {
    const float MAX_CAPACITY = 100.0f;
    
    // Проверяем, есть ли вообще свободное место в контейнере
    float current_total = this->signal_corruption + this->noise_corruption;
    float free_space = MAX_CAPACITY - current_total;

    if (free_space > 0) {
        // Добавляем сигнал только в пределах свободного места
        float addition = std::min(amount, free_space);
        this->signal_corruption += addition;
    }
    // Если свободного места нет (free_space <= 0), сигнал просто игнорируется
    
    update_corruption_logic();
}

void Player::add_noise(float amount) {
    const float MAX_CAPACITY = 100.0f;
    
    float current_total = this->signal_corruption + this->noise_corruption;
    float free_space = MAX_CAPACITY - current_total;

    if (free_space >= amount) {
        // Случай 1: Места достаточно, просто добавляем шум
        this->noise_corruption += amount;
    } 
    else {
        // Случай 2: Места мало или нет. Нужно вытеснять сигнал.
        
        // Сначала забираем всё оставшееся свободное место
        float amount_to_add = amount;
        if (free_space > 0) {
            this->noise_corruption += free_space;
            amount_to_add -= free_space;
        }

        // Теперь вытесняем сигнал тем, что осталось от amount
        if (this->signal_corruption > 0) {
            float displaced = std::min(amount_to_add, this->signal_corruption);
            this->signal_corruption -= displaced;
            this->noise_corruption += displaced;
        }
        
        // Итоговая проверка, чтобы не вылезти за 100 из-за погрешностей float
        if (this->noise_corruption > MAX_CAPACITY) this->noise_corruption = MAX_CAPACITY;
    }

    update_corruption_logic();
}

void Player::flush_corruption(float amount) {
    // 1. Пытаемся сначала очистить Шум
    if (this->noise_corruption > 0) {
        float drain = std::min(amount, this->noise_corruption);
        this->noise_corruption -= drain;
        amount -= drain; // Вычитаем потраченный "ресурс очистки"
    }

    // 2. Если после очистки Шума еще остался заряд (amount > 0), чистим Сигнал
    if (amount > 0 && this->signal_corruption > 0) {
        this->signal_corruption -= std::min(amount, this->signal_corruption);
    }

    // Для надежности фиксируем нули
    if (this->noise_corruption < 0) this->noise_corruption = 0;
    if (this->signal_corruption < 0) this->signal_corruption = 0;

    update_corruption_logic();
}

void Player::update_timers(float dt) {
    if (dash_charges < MAX_DASH_CHARGES) {
        dash_regen_timer += dt;
        if (dash_regen_timer >= DASH_REGEN_TIME) {
            dash_charges++;
            dash_regen_timer = 0;
        }
    }
    if (dash_timer > 0) dash_timer -= dt;
    if (invul_timer > 0) invul_timer -= dt;
}

void Player::recalculate_stats() {
    // 1. Сброс к базовым значениям
    this->max_hp = this->base_max_hp;
    this->current_damage = this->DEFAULT_DAMAGE;
    this->current_penetration = 0.0f;
    this->current_fire_rate_mult = 1.0f;
    this->armor = 0.0f; // Из Game_Object

    // 2. Применяем эффекты каждого чипа
    for (int i = 0; i < 4; ++i) {
        if (chips[i]) {
            // Влияем на тело (HP, Armor)
            chips[i]->apply_to_player(this);
            
            // Влияем на виртуальные статы оружия
            // Создаем временную структуру, чтобы собрать бонусы
            WeaponStats temp_stats;
            temp_stats.damage = 1.0f; // Используем как множители
            temp_stats.penetration = 0.0f;
            temp_stats.fire_rate = 1.0f;

            chips[i]->apply_to_weapon(temp_stats);

            // Складываем бонусы
            this->current_damage *= temp_stats.damage;
            this->current_penetration += temp_stats.penetration;
            this->current_fire_rate_mult *= temp_stats.fire_rate;
        }
    }

    // 3. Обновляем UI
    if (this->hp > this->max_hp) this->hp = this->max_hp;
    this->hp_needs_update = true;
}

bool Player::pick_up_chip(Chip* new_chip) {
    if (!new_chip) return false;
    
    if (inventory.size() < MAX_INV_SIZE) {
        inventory.push_back(new_chip);
        return true;
    }
    return false; // Инвентарь полон
}

void Player::equip_chip(int inv_idx, int slot_idx) {
    if (inv_idx < 0 || inv_idx >= inventory.size()) return;
    if (slot_idx < 0 || slot_idx >= 4) return;

    // Если в слоте уже есть чип, возвращаем его в рюкзак
    Chip* old_active = chips[slot_idx];
    
    // Устанавливаем новый чип из рюкзака
    chips[slot_idx] = inventory[inv_idx];
    
    // Удаляем его из списка рюкзака
    inventory.erase(inventory.begin() + inv_idx);

    // Если мы что-то сняли, кладем это в рюкзак
    if (old_active) {
        inventory.push_back(old_active);
    }

    recalculate_stats();
}

void Player::discard_chip(int inv_idx) {
    if (inv_idx >= 0 && inv_idx < inventory.size()) {
        delete inventory[inv_idx];
        inventory.erase(inventory.begin() + inv_idx);
    }
}

// Реализация методов эффектов (чтобы не было циклической зависимости в .hpp)
void HealthEffect::apply_to_player(Player* p, int level) {
    p->max_hp += level * ChipBalance::HP_FLAT;
}

void ArmorEffect::apply_to_player(Player* p, int level) {
    p->armor += level * ChipBalance::ARMOR_FLAT;
}

void Player::handle_dash_input() {
    // Проверяем: нажат Shift, есть заряды, нет текущего рывка
    // И ГЛАВНОЕ: текущая скорость по любой оси должна быть заметной
    bool is_moving = (std::abs(vel_x) > 0.1f || std::abs(vel_y) > 0.1f);

    if (Binds::is_pressed(Action::DASH) && dash_charges > 0 && dash_timer <= 0 && is_moving) {
        dash_timer = DASH_DURATION;
        dash_charges--;
        this->invul_timer = DASH_DURATION + 0.1f;
    }
}

void Player::handle_movement(Room &room) {
    if (dash_timer > 0) {
        move(vel_x * 3.0, vel_y * 3.0, room);
    } else {
        double acc = 0.5;
        if (Binds::is_pressed(Action::MOVE_UP))    vel_y -= acc;
        if (Binds::is_pressed(Action::MOVE_DOWN))  vel_y += acc;
        if (Binds::is_pressed(Action::MOVE_LEFT))  vel_x -= acc;
        if (Binds::is_pressed(Action::MOVE_RIGHT)) vel_x += acc;

        vel_x *= 0.85; // Трение
        vel_y *= 0.85;

        double max_speed = 4.5;
        if (vel_x > max_speed)  vel_x = max_speed;
        if (vel_x < -max_speed) vel_x = -max_speed;
        if (vel_y > max_speed)  vel_y = max_speed;
        if (vel_y < -max_speed) vel_y = -max_speed;

        move(vel_x, vel_y, room);
    }
}

void Player::handle_rally(float dt) {
    if (rally_timer > 0) {
        rally_timer -= dt;
    } else if (recoverable_hp > 0) {
        recoverable_hp -= 0.5f; // Скорость таяния оранжевой полоски
        if (recoverable_hp < 0) recoverable_hp = 0;
        this->hp_needs_update = true;
    }
}

void Player::handle_shooting() {
    ALLEGRO_MOUSE_STATE mouse;
    al_get_mouse_state(&mouse);

    if (fire_cooldown > 0) fire_cooldown--;

    if (((mouse.buttons & 1) || Binds::is_pressed(Action::ATTACK)) && fire_cooldown <= 0) {
        float dw = (float)al_get_display_width(al_get_current_display());
        float dh = (float)al_get_display_height(al_get_current_display());
        float scale = std::min(dw / 640.0f, dh / 360.0f);

        float tx = mouse.x / scale;
        float ty = mouse.y / scale;

        // Настройка параметров выстрела
        BulletProps props;
        props.damage = this->current_damage;
        props.penetration = 2.0f;           // Пуля игнорирует 2 ед. брони
        props.type = DamageType::PHYSICAL;  // Физический тип урона
        props.speed = 10.0f;
        props.size = 3.0f;
        props.color = al_map_rgb(255, 255, 0);

        /// Спавним пулю, передавая наши настроенные props
        Screen::spawn(new Bullet(pos_x + width/2, 
                                 pos_y + height/2, 
                                 tx, ty, props));
                                 
        fire_cooldown = FIRE_COOLDOWN;
    }
}

void Player::handle_ui_input() {
    bool current_stats_state = Binds::is_pressed(Action::PLAYER_STATS);

    if (current_stats_state && !stats_key_pressed) {
        show_stats = !show_stats;
        if (show_stats) this->hp_needs_update = true; 
    }
    stats_key_pressed = current_stats_state;
}

void Player::physics(Room &room) {
    const float DT = 0.016f; //константа времени (60FPS)

    // 1. Логика времени
    update_timers(DT);        // Регенерация рывков, кулдауны способностей
    handle_rally(DT);         // Таяние "оранжевого" здоровья

    // 2. Логика ввода (теперь Binds сам опрашивает клавиатуру)
    handle_dash_input();      // Рывок
    handle_movement(room);    // Движение и столкновения со стенами
    handle_shooting();        // Мышь + альтернативная кнопка стрельбы
    handle_ui_input();        // Открытие/закрытие окна статистики

    // --- ТОЧЕЧНОЕ УПРАВЛЕНИЕ (1-4) ---
    
    // Добавить Сигнал (Инъекция)
    if (Binds::is_pressed(Action::DEBUG_ADD_SIGNAL)) {
        add_signal(0.5f); 
    }
    // Удалить только Сигнал (Откат апгрейда)
    if (Binds::is_pressed(Action::DEBUG_REM_SIGNAL)) {
        if (signal_corruption > 0) {
            signal_corruption -= 0.5f;
            update_corruption_logic();
        }
    }
    // Добавить Мусор
    if (Binds::is_pressed(Action::DEBUG_ADD_NOISE)) {
        add_noise(0.5f);
    }
    // Удалить только Мусор (Дефрагментация)
    if (Binds::is_pressed(Action::DEBUG_REM_NOISE)) {
        if (noise_corruption > 0) {
            noise_corruption -= 0.5f;
            update_corruption_logic();
        }
    }

    // --- УМНАЯ ОЧИСТКА (5) ---
    if (Binds::is_pressed(Action::DEBUG_FLUSH_ALL)) {
        this->flush_corruption(0.8f); // Сначала шум, потом сигнал
    }
}

void Player::render() {
    draw_player_body();
    draw_dash_bar();

    if (show_stats) {
        // Используем флаг hp_needs_update, чтобы перерисовать статы 
        // только если что-то изменилось (получили урон, открыли окно и т.д.)
        if (this->hp_needs_update) {
            // Передаём шрифт
            this->update_stats_cache(Assets::main_font); 
            // Мы не сбрасываем флаг здесь, если его сбрасывает другой метод, 
            // но важно, чтобы кэш обновился.
        }

        if (stats_cache) {
            al_draw_bitmap(stats_cache, 20, 150, 0); 
        }
    }
}

void Player::draw_player_body() {
    ALLEGRO_COLOR color;

    // Приоритет 1: Состояние рывка (Белый)
    if (dash_timer > 0) {
        color = al_map_rgb(255, 255, 255);
    } 
    // Приоритет 2: Состояние неуязвимости после урона (Мигание)
    else if (invul_timer > 0) {
        // Мигаем каждые 0.1 секунды
        if ((int)(invul_timer * 10) % 2 == 0) {
            color = al_map_rgba(0, 255, 100, 100); // Полупрозрачный зеленый
        } else {
            color = al_map_rgb(0, 255, 100);       // Обычный зеленый
        }
    } 
    // Состояние 3: Обычное (Зеленый)
    else {
        color = al_map_rgb(0, 255, 100);
    }

    // Рисуем основной квадрат игрока
    al_draw_filled_rectangle(pos_x, pos_y, pos_x + width, pos_y + height, color);
    
    // Дополнительно: можно добавить контур, чтобы персонаж лучше читался на темном фоне
    al_draw_rectangle(pos_x, pos_y, pos_x + width, pos_y + height, al_map_rgb(0, 0, 0), 1.0f);
}

void Player::draw_dash_bar() {
    // Настройки позиции (над головой)
    float bar_w = 40.0f; // Общая ширина шкалы
    float bar_h = 6.0f;  // Высота шкалы
    float bx = pos_x + (width / 2) - (bar_w / 2); // Центрируем над игроком
    float by = pos_y - 12; // Смещение вверх

    // Цвета из твоего референса
    ALLEGRO_COLOR col_bg = al_map_rgb(30, 30, 30);      // Темный фон
    ALLEGRO_COLOR col_frame = al_map_rgb(255, 255, 255); // Белая рамка
    ALLEGRO_COLOR col_full = al_map_rgb(0, 191, 255);   // Ярко-голубой (заряжен)
    ALLEGRO_COLOR col_regen = al_map_rgb(0, 80, 120);    // Темно-синий (заряжается)

    // 1. Рисуем подложку
    al_draw_filled_rectangle(bx, by, bx + bar_w, by + bar_h, col_bg);

    // 2. Рисуем сегменты
    float seg_w = bar_w / MAX_DASH_CHARGES;
    
    for (int i = 0; i < MAX_DASH_CHARGES; i++) {
        float x1 = bx + (i * seg_w);
        float x2 = x1 + seg_w;

        if (i < dash_charges) {
            // Полностью готовый заряд
            al_draw_filled_rectangle(x1 + 1, by + 1, x2 - 1, by + bar_h - 1, col_full);
        } 
        else if (i == dash_charges) {
            // Текущий восстанавливающийся заряд
            float progress = dash_regen_timer / DASH_REGEN_TIME;
            float current_fill_w = (seg_w - 2) * progress;
            al_draw_filled_rectangle(x1 + 1, by + 1, x1 + 1 + current_fill_w, by + bar_h - 1, col_regen);
        }
    }

    // 3. Рисуем общую рамку
    al_draw_rectangle(bx, by, bx + bar_w, by + bar_h, col_frame, 1.0f);

    // 4. Рисуем разделители между сегментами
    for (int i = 1; i < MAX_DASH_CHARGES; i++) {
        float dx = bx + (i * seg_w);
        al_draw_line(dx, by, dx, by + bar_h, col_frame, 1.0f);
    }
}