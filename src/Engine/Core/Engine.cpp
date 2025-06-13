#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <fstream>
#include <unordered_set>
#include <vector>
#include <string>
#include <memory>
#include <chrono>
#include <thread>

// ─────────────────────────────────────────────
// GroundBrick Class
// Handles creation of brick objects using shared textures for memory efficiency.
// ─────────────────────────────────────────────
class GroundBrick {
private:
    inline static std::shared_ptr<sf::Texture> shared_brick_texture; // Shared texture among all bricks

public:
    std::unordered_set<int> empty;
public:
    GroundBrick() {
        // Load texture only once using static shared pointer
        if (!shared_brick_texture) {
            shared_brick_texture = std::make_shared<sf::Texture>();
            if (!shared_brick_texture->loadFromFile("assets/img/brick-1.png")) {
                std::cerr << "Failed to load brick texture\n";
            }
        }
    }

    // Returns a shared_ptr to a brick rectangle shape with texture applied
    std::shared_ptr<sf::RectangleShape> getObj() {
        auto brick_frame = std::make_shared<sf::RectangleShape>(sf::Vector2f(50.f, 50.f));
        brick_frame->setTexture(shared_brick_texture.get());
        return brick_frame;
    }
};


// ─────────────────────────────────────────────
// Mario Class
// Represents the Mario character including rendering, animation, and jumping.
// ─────────────────────────────────────────────
class Mario {
private:
    sf::RectangleShape mario;                                               // Mario's visible rectangle
    sf::Texture mario_texture, mario_jump_texture, mario_backward_texture, mario_jump_texture_backward;  // Texture for running and jumping
    std::vector<sf::Texture> gifTextures, gifTextureBackward;               // Animation frames
    sf::Sprite sprite1;
    unsigned int id;                                                        // Object identifier
    size_t currentFrame = 0;                                                // Current frame for animation
    sf::Clock animationClock, clock;                                        // Clocks for timing animations and jumping
    float frameDuration = 0.01f;                                            // Duration of each animation frame (in seconds)
    float vy = 0.1f;                                                        // Vertical velocity
    float gravity = 980.0f;                                                 // Gravitational acceleration
    bool isJumping = false;                                                 // Jump state flag
    bool isHighJump = false;                                                // Whether a high jump is active
    float jumpForce = -500.0f;             
    
    // Global or class member variables
    float vx = 0.0f;
    float backwardJumpSpeed = -150.0f;
    bool isJumpingBackward = false;

public:
    sf::Vector2f running_pos;


public:
    Mario() = default;

    // Initialize Mario with a unique ID and prepare textures
    Mario(unsigned int obj_id) {
        id = obj_id;
        mario.setSize(sf::Vector2f(75.f, 75.f));
        mario.setPosition(sf::Vector2f(390.f, 480.f));
        mario_jump_texture.loadFromFile("assets/img/mario/mario-jump.png");
        mario_jump_texture_backward.loadFromFile("assets/img/mario/mario-jump-rev.png");
        running_pos = sf::Vector2f(390.f, 480.f);

        // Load running animation frames
        for (int i = 0; i < 5; i++) {
            mario_texture.loadFromFile("assets/img/mario-char/mario-" + std::to_string(i) + "_resized.png");
            gifTextures.push_back(std::move(mario_texture));
        }

        // loading backward running animation frames
        for (int i = 0; i < 5; i++) {
            mario_backward_texture.loadFromFile("assets/img/mario-char/mario-" + std::to_string(i) + "-rev_resized.png");
            gifTextureBackward.push_back(std::move(mario_backward_texture));
        }

        stand(); // Set initial standing frame
    }

    // Set Mario to standing frame (idle or jumping texture)
    void stand() {
        if (!isJumping) {
            mario.setTexture(&gifTextures[0]);
        }
        else {
            mario.setTexture(&mario_jump_texture);
        }
    }

    void standBack() {
        if (!isJumping) {
            mario.setTexture(&gifTextureBackward[0]);
        }
        else {
            mario.setTexture(&mario_jump_texture_backward);
        }
    }

    // Play running animation frames
    void run() {
        if (animationClock.getElapsedTime().asSeconds() > frameDuration) {
            currentFrame = (currentFrame + 1) % gifTextures.size();
            if (mario.getPosition().x < 390.f) {
                mario.setPosition(mario.getPosition().x + 20.f, mario.getPosition().y);
                running_pos = sf::Vector2f(mario.getPosition().x + 20.f, mario.getPosition().y);
            }
            if (!isJumping) {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                mario.setTexture(&gifTextures[currentFrame]);
            }
            else {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                mario.setTexture(&mario_jump_texture);
            }
            animationClock.restart();
            
        }
    }

    void runBackward() {
        if (animationClock.getElapsedTime().asSeconds() > frameDuration) {
            currentFrame = (currentFrame + 1) % gifTextureBackward.size();
            if (!isJumping) {
                sf::Vector2f curr_pos = mario.getPosition();
                mario.setPosition(sf::Vector2f(curr_pos.x - 20.f, curr_pos.y));
                std::this_thread::sleep_for(std::chrono::milliseconds(30));
                mario.setTexture(&gifTextureBackward[currentFrame]);
                running_pos = sf::Vector2f(curr_pos.x - 20.f, curr_pos.y);
            }
            else {
                sf::Vector2f curr_pos = mario.getPosition();
                mario.setPosition(sf::Vector2f(curr_pos.x - 20.f, curr_pos.y));
                std::this_thread::sleep_for(std::chrono::milliseconds(30));
                mario.setTexture(&mario_jump_texture_backward);
                running_pos = sf::Vector2f(curr_pos.x - 20.f, curr_pos.y);
            }
            animationClock.restart();

        }
    }

    // Return the Mario object for rendering
    sf::RectangleShape getObj() {
        return mario;
    }

    sf::Vector2f getPosition() {
        return mario.getPosition();
    }

    // Initiates a jump. Accepts flags to determine if it's a high jump and/or backward jump.
    void startJump(bool highJump, bool jumpBackward) {
        if (!isJumping) {
            vy = jumpForce;
            vx = jumpBackward ? backwardJumpSpeed : 0.0f;
            isHighJump = highJump;
            isJumpingBackward = jumpBackward;
            isJumping = true;
        }
    }

    // Update Mario's movement and state during a jump
    void updateJump() {
        float dt = clock.restart().asSeconds();

        // Gravity affects vertical velocity
        vy += gravity * dt;

        float moveY = isHighJump ? (vy - 150) * dt : vy * dt;
        float moveX = isJumpingBackward ? vx * dt : 0.0f;

        // Move Mario diagonally if jumping backward
        mario.move(moveX, moveY);

        // Ground collision check
        if (mario.getPosition().y >= 475.f) {
            mario.setPosition(mario.getPosition().x, 475.f);
            vy = 0.0f;
            vx = 0.0f;
            isJumping = false;
            isHighJump = false;
            isJumpingBackward = false;
        }
    }

    // Destructor logs the object's destruction
    ~Mario() {
        std::cout << "Mario obj id : " << id << " removed" << std::endl;
    }
};

// ─────────────────────────────────────────────
// SuperMarioGamePlay Class
// Main game loop handler: rendering, audio, input, and updates
// ─────────────────────────────────────────────
class SuperMarioGamePlay {
private:
    sf::RenderWindow window;                // Main game window
    sf::Texture skyTexture;                 // Background texture
    sf::RectangleShape background;          // Background shape

    // Audio resources
    sf::Music ground_play_bg_audio;
    sf::SoundBuffer jumpBuffer;
    sf::Sound jumpSound;
    sf::Clock soundCooldownClock;          // Prevents spamming jump sounds

    std::vector<std::vector<std::shared_ptr<sf::RectangleShape>>> brickslab; // Brick grid
    GroundBrick brick;                      // Brick generator
    Mario mario;                            // Mario object
    int defaultpose = 0;                    // 0 -> forward , 1 -> Backward

public:
    // Game constructor: initializes window, loads assets, builds level
    SuperMarioGamePlay() : window(sf::VideoMode(1600, 900), "3D Game Engine"), mario(100) {
        window.setFramerateLimit(100);

        // Load level layout from file
        std::ifstream file("assets/level/Level0.dat");
        if (!file) {
            std::cerr << "Could not open file\n";
        }

        brickslab.resize(10);  // Create 10 rows

        char ch;
        int index = 0;
        while (file.get(ch)) {
            if (ch == '1') {
                for (int h = 0; h < 10; h++) {
                    auto new_brick_obj = brick.getObj();
                    new_brick_obj->setPosition(sf::Vector2f{ 50.f * index, 550.f + (50.f * h) });
                    brickslab[h].push_back(new_brick_obj);
                    
                }
            }
            else {
                float rad = 50.f * index;
                for (float i = rad; i <= rad + 50; i++) brick.empty.insert(i);
            }

            index += 1;
        }

        for (auto ff : brick.empty) {
            std::cout << ff << std::endl;
        }
        file.close();

        // Load background music
        if (!ground_play_bg_audio.openFromFile("assets/audio/gameplay-ground.ogg")) {
            std::cerr << "Failed to load music\n";
        }
        ground_play_bg_audio.setLoop(true);
        ground_play_bg_audio.play();

        // Load jump sound effect
        if (!jumpBuffer.loadFromFile("assets/audio/jump-small.wav")) {
            std::cerr << "Failed to load jump sound\n";
        }
        jumpSound.setBuffer(jumpBuffer);

        // Load sky background
        if (!skyTexture.loadFromFile("assets/img/main_bg.png")) {
            std::cerr << "Failed to load sky texture.\n";
        }
        background.setTexture(&skyTexture);
        background.setSize(sf::Vector2f(1600, 900));
    }

    // Main game loop
    void run() {
        while (window.isOpen()) {
            processEvents();
            update();
            check();
            render();
        }
    }

private:
    // Handles input, sound logic, and character movement
    void processEvents() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        // Handle jump input with LShift modifier for high jump
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
            float cooldown = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ? 1.1f : 1.0f;
            if (soundCooldownClock.getElapsedTime().asSeconds() > cooldown) {
                jumpSound.play();
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
                    mario.startJump(true, false);
                }
                else {
                    mario.startJump(false, false);
                }
                soundCooldownClock.restart();
            }
        }

        // Move scene to left when right key is pressed
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
            mario.run();
            if (mario.getPosition().x > 380.f)
            {
                float moveSpeed = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ? 15.f : 8.f;

                for (auto& row : brickslab) {
                    for (auto& brick : row) {
                        sf::Vector2f pos = brick->getPosition();
                        brick->setPosition(pos.x - moveSpeed, pos.y);
                        mario.running_pos = sf::Vector2f(mario.getPosition().x + (pos.x - moveSpeed), pos.y);
                    }
                }
            }
            defaultpose = 0;
        } 
        // Move mario to left when left key is pressed, but brick stays at same position
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
            mario.runBackward();
            defaultpose = 1;
        }
        else {
            // Show standing frame if not moving
            if (defaultpose == 1)
                mario.standBack();
            else
                mario.stand();
        }

        mario.updateJump();
    }

    // Placeholder for additional logic updates
    void update() {}

    // check some confditions
    void check() {
        sf::Vector2f curr_pos =  mario.running_pos;
        std::cout << "Running position - x : " << curr_pos.x << " - y : " << curr_pos.y << std::endl;
        float start = curr_pos.x;
        float end = curr_pos.x + 75.f;
        std::cout << "Reached the blank space - x : " << start << " - y : " << end << std::endl;
        if (brick.empty.count(start) || brick.empty.count(end)) {
            std::cout << "Reached the blank space - x : " << start << " - y : " << end << std::endl;
            if (curr_pos.y > 450.f) {
                std::cout << "Reached the blank space" << std::endl;
            }
        }
    }

    // Renders background, bricks, and Mario to screen
    void render() {
        window.clear(sf::Color(222, 161, 161));
        window.draw(background);

        for (const auto& row : brickslab) {
            for (const auto& brick : row) {
                window.draw(*brick);
            }
        }

        window.draw(mario.getObj());
        window.display();
    }
};
