#ifndef World_hpp
#define World_hpp

#include <vector>
#include "Game_Objects/Game_Object.hpp"

class Screen {
public:
    static std::vector<Game_Object*> objects;
    static void spawn(Game_Object* obj) {
        objects.push_back(obj);
    }
};

#endif