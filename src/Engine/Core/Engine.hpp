#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "configs/components/Wall.hpp"

namespace RectangleObj {
    std::vector<sf::Vector2i> getRectObjVector(sf::Vector2i new_A, sf::Vector2i new_B, sf::Vector2i new_C, sf::Vector2i new_D);
}

namespace WallObjActions {
    void moveWall(Wall wall, std::vector<sf::Vector2i> newPosition);
    void scaleWall(Wall wall, float scale_size);
}

class Game {
public:
    Game();
    void run();

private:
    void moveWall(sf::Vector2i mouse_current_position);
    void processEvents();
    void update();
    void render();

    sf::RenderWindow window;
    sf::ConvexShape convex;
    sf::Vector2i localPosition;
    sf::Texture skyTexture;
    sf::RectangleShape background;
    std::vector<sf::ConvexShape> wall_objects;
    Wall onewall;
};
