// Данный пример работает на engine,
// поэтому надо изменить build скрипт
// чтобы он подключал engine.cpp

#include "engine.hpp"
#include <allegro5/allegro_primitives.h>
#include <cstddef>
#include <iostream>
#include <vector>

using namespace std;

struct Point {
    float x, y;
};

class IObject {
  public:
    virtual void physics_process() = 0;
    virtual void render_process() = 0;
    virtual ~IObject() = default;
};

class Screen : public Engine {
  private:
    vector<IObject *> objects = {};

  public:
    void add_object(IObject *obj) { objects.push_back(obj); }
    void physics_process() override {
        for (size_t i = 0; i < objects.size(); i++) {
            objects[i]->physics_process();
        }
    }
    void render_process() override {
        for (size_t i = 0; i < objects.size(); i++) {
            objects[i]->render_process();
        }
    }
};


int main() {
    Screen screen;

    try {
        screen.start();
    } catch (char const *error) {
        cout << error << endl;
    }
    return 0;
}