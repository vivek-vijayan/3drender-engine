#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <ctime>
#include <stdlib.h>
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

class Cloud {
private:
    sf::RectangleShape cloud;
    sf::Texture cloudTexture;

public:
    Cloud() = default;
    Cloud(sf::Vector2f init_position, sf::Vector2f init_size) {
        cloud.setPosition(init_position);
        cloud.setSize(init_size);

        // Loading the cloud image
        cloudTexture.loadFromFile("assets/img/cloud.png");
        cloud.setTexture(&cloudTexture);
    }

    sf::RectangleShape &getObj() {
        return cloud;
    }
};


class Gomma {
private:
    sf::RectangleShape gomma;
    sf::Texture gommaTexture;
    std::vector<sf::Texture> gifTextures;
    size_t currentFrame = 0;                                                // Current frame for animation
    sf::Clock animationClock, clock;
    float frameDuration = 0.01f;

public:
    Gomma() = default;
    Gomma(sf::Vector2f init_position, sf::Vector2f init_size) {
        gomma.setPosition(init_position);
        gomma.setSize(init_size);

        for (int i = 1; i < 3; i++) {
            gommaTexture.loadFromFile("assets/img/gomma/gomma-" + std::to_string(i) + ".png");
            gifTextures.push_back(std::move(gommaTexture));
        }
    }

    sf::RectangleShape& getObj() {
        return gomma;
    }

    void auto_move() {
        if (animationClock.getElapsedTime().asSeconds() > frameDuration) {
            currentFrame = (currentFrame + 1) % gifTextures.size();
            if (gomma.getPosition().x > -200.f) {
                gomma.setPosition(gomma.getPosition().x - 10.f, gomma.getPosition().y);
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(30));
            gomma.setTexture(&gifTextures[currentFrame]);
            animationClock.restart();
        }
    }
};

class Bush {
private:
    sf::RectangleShape bush;
    sf::Texture bushTexture;

public:
    Bush() = default;
    Bush(sf::Vector2f init_position, sf::Vector2f init_size) {
        bush.setPosition(init_position);
        bush.setSize(init_size);

        // Loading the cloud image
        bushTexture.loadFromFile("assets/img/bush.png");
        bush.setTexture(&bushTexture);
    }

    sf::RectangleShape& getObj() {
        return bush;
    }
};
// ──────────────────────────────────────────
// Mario Class
// Represents the Mario character including rendering, animation, and jumping.
// ─────────────────────────────────────────────
class Mario {
private:
    sf::RectangleShape mario;                                               // Mario's visible rectangle
    sf::Texture mario_texture, mario_jump_texture, mario_backward_texture, mario_jump_texture_backward, gomma_texture;  // Texture for running and jumping
    std::vector<sf::Texture> gifTextures, gifTextureBackward;               // Animation frames
    sf::Sprite sprite1;
    unsigned int id;                                                        // Object identifier
    size_t currentFrame = 0;                                                // Current frame for animation
    sf::Clock animationClock, clock;                                        // Clocks for timing animations and jumping
    float frameDuration = 0.01f;                                            // Duration of each animation frame (in seconds)
    float vy = 0.1f;       
    float fall_vy = 0.1f;// Vertical velocity
    float gravity = 980.0f;                                                 // Gravitational acceleration
    bool isJumping = false;                                                 // Jump state flag
    bool isHighJump = false;                                                // Whether a high jump is active
    float jumpForce = -500.0f;             
    
    // Global or class member variables
    float vx = 0.0f;
    float backwardJumpSpeed = -150.0f;
    bool isJumpingBackward = false;

public:
    bool isFalling = false;

public:
    Mario() = default;

    // Initialize Mario with a unique ID and prepare textures
    Mario(unsigned int obj_id) {
        id = obj_id;

        mario.setSize(sf::Vector2f(75.f, 75.f));
        mario.setPosition(sf::Vector2f(390.f, 480.f));
        mario_jump_texture.loadFromFile("assets/img/mario/mario-jump.png");
        mario_jump_texture_backward.loadFromFile("assets/img/mario/mario-jump-rev.png");

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

    // Falling down
    void updateFall() {
        float dt = clock.restart().asSeconds();
        std::cout << fall_vy << " Mario Y: " << mario.getPosition().y << std::endl;
        fall_vy += (float) gravity * dt * 20.f;                        // Accelerate due to gravity
        mario.move(0.f, (fall_vy * dt));
        // Move down
        //std::this_thread::sleep_for(std::chrono::milliseconds(30));
        // std::this_thread::sleep_for(std::chrono::milliseconds(30));
        fall_vy = (float) gravity * dt;
        const float maxFallSpeed = 1500.f;
        if (fall_vy > maxFallSpeed)
            fall_vy = maxFallSpeed;

        // Clamp to floor (adjusted for height)
        if (mario.getPosition().y >= 1000.f) {
            mario.setPosition(mario.getPosition().x, 1000.f);
            fall_vy = 0.0f;
            isFalling = false;
        }
    }



    // Play running animation frames
    void run() {
        if (animationClock.getElapsedTime().asSeconds() > frameDuration) {
            currentFrame = (currentFrame + 1) % gifTextures.size();
            if (mario.getPosition().x < 390.f) {
                mario.setPosition(mario.getPosition().x + 20.f, mario.getPosition().y);
            }
            if (!isJumping) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                mario.setTexture(&gifTextures[currentFrame]);
            }
            else {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
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
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                mario.setTexture(&gifTextureBackward[currentFrame]);
            }
            else {
                sf::Vector2f curr_pos = mario.getPosition();
                mario.setPosition(sf::Vector2f(curr_pos.x - 20.f, curr_pos.y));
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                mario.setTexture(&mario_jump_texture_backward);
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
    sf::SoundBuffer jumpBuffer, dieBuffer;
    sf::Sound jumpSound, dieSound;
    sf::Clock soundCooldownClock;          // Prevents spamming jump sounds

    std::vector<std::vector<std::shared_ptr<sf::RectangleShape>>> brickslab; // Brick grid

    GroundBrick brick;                      // Brick generator
    Mario mario;                            // Mario object
    int defaultpose = 0;                    // 0 -> forward , 1 -> Backward
    sf::Vector2f running_pos;
    float total_length = 0.f;
    bool fallen;
    Cloud cloud1, cloud2, cloud3, cloud4, cloud5, cloud6, cloud7;
    Bush bush1, bush2, bush3, bush4, bush5, bush6, bush7, bush8, bush9, bush10, bush11, bush12;
    Gomma gomma1;
public:
    // Game constructor: initializes window, loads assets, builds level
    SuperMarioGamePlay() : window(sf::VideoMode(1600, 900), "Super Mario Bros"), mario(100),
        cloud1(sf::Vector2f(100.f, 100.f), sf::Vector2f(120.f, 80.f)),
        cloud2(sf::Vector2f(300.f, 200.f), sf::Vector2f(120.f, 80.f)),
        cloud3(sf::Vector2f(600.f, 100.f), sf::Vector2f(120.f, 80.f)),
        cloud4(sf::Vector2f(1000.f, 400.f), sf::Vector2f(120.f, 80.f)),
        cloud5(sf::Vector2f(1200.f, 100.f), sf::Vector2f(120.f, 80.f)),
        cloud6(sf::Vector2f(1600.f, 100.f), sf::Vector2f(120.f, 80.f)),
        cloud7(sf::Vector2f(2500.f, 100.f), sf::Vector2f(120.f, 80.f)),

        bush1(sf::Vector2f(0.f, 500.f), sf::Vector2f(120.f, 80.f)),
        bush2(sf::Vector2f(150.f, 500.f), sf::Vector2f(120.f, 80.f)),
        bush3(sf::Vector2f(600.f, 500.f), sf::Vector2f(120.f, 80.f)),
        bush4(sf::Vector2f(900.f, 500.f), sf::Vector2f(120.f, 80.f)),
        bush5(sf::Vector2f(1700.f, 500.f), sf::Vector2f(120.f, 80.f)),
        bush6(sf::Vector2f(2500.f, 500.f), sf::Vector2f(120.f, 80.f)),
        bush7(sf::Vector2f(4500.f, 500.f), sf::Vector2f(120.f, 80.f)),
        bush8(sf::Vector2f(6500.f, 500.f), sf::Vector2f(120.f, 80.f)),
        bush9(sf::Vector2f(6900.f, 500.f), sf::Vector2f(120.f, 80.f)),
        bush10(sf::Vector2f(8000.f, 500.f), sf::Vector2f(120.f, 80.f)),
        bush11(sf::Vector2f(9000.f, 500.f), sf::Vector2f(120.f, 80.f)),
        bush12(sf::Vector2f(9500.f, 500.f), sf::Vector2f(120.f, 80.f)),
        gomma1(sf::Vector2f(2000.f, 480.f), sf::Vector2f(70.f, 70.f))

        {
        window.setFramerateLimit(100);
        std::vector<std::shared_ptr<sf::RectangleShape>> clouds;

        running_pos = sf::Vector2f(sf::Vector2f(390.f, 480.f));
        fallen = false;

        

        // Load level layout from file
        std::ifstream level0_brick("assets/level/Level0-brick.dat");
        if (!level0_brick) {
            std::cerr << "Could not open file\n";
        }

        brickslab.resize(10);  // Create 10 rows

        char ch;
        int index = 0;
        while (level0_brick.get(ch)) {
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
            total_length = 50.f * index;
        }

        level0_brick.close();

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

        
        // Load jump sound effect
        if (!dieBuffer.loadFromFile("assets/audio/mariodie.wav")) {
            std::cerr << "Failed to load jump sound\n";
        }
        dieSound.setBuffer(dieBuffer);

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
            gomma1.auto_move();
            render();
        }
    }

private:
    float moving_curr_brixk_x = 390.f;
    float moving_curr_brixk_y = 450.f;
    bool hasPlayedDieSound = false;
    // Handles input, sound logic, and character movement
    void processEvents() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        // Handle jump input with LShift modifier for high jump
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
            if (!fallen)
            {
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
        }

        // Move scene to left when right key is pressed
        
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) && !fallen) {
            
            mario.run();
            float moveSpeed = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ? 15.f : 8.f;
            if (mario.getPosition().x > 380.f)
            {
                for (auto& row : brickslab) {
                    for (auto& brick : row) {
                        sf::Vector2f pos = brick->getPosition();
                        moving_curr_brixk_x = pos.x;
                        moving_curr_brixk_y = pos.y;
                        brick->setPosition(pos.x - moveSpeed, pos.y);
                        running_pos = sf::Vector2f(total_length - moving_curr_brixk_x + moveSpeed + mario.getPosition().x, mario.getPosition().y);
                    }   
                }

                // Parallel moving objects
                cloud1.getObj().setPosition((sf::Vector2f(cloud1.getObj().getPosition().x - 2.f, 100.f)));
                cloud2.getObj().setPosition((sf::Vector2f(cloud2.getObj().getPosition().x - 2.f, 200.f)));
                cloud3.getObj().setPosition((sf::Vector2f(cloud3.getObj().getPosition().x - 2.f, 100.f)));
                cloud4.getObj().setPosition((sf::Vector2f(cloud4.getObj().getPosition().x - 2.f, 400.f)));
                cloud5.getObj().setPosition((sf::Vector2f(cloud5.getObj().getPosition().x - 2.f, 100.f)));
                cloud6.getObj().setPosition((sf::Vector2f(cloud6.getObj().getPosition().x - 2.f, 100.f)));
                cloud7.getObj().setPosition((sf::Vector2f(cloud7.getObj().getPosition().x - 2.f, 100.f)));

                bush1.getObj().setPosition((sf::Vector2f(bush1.getObj().getPosition().x - moveSpeed, 500.f)));
                bush2.getObj().setPosition((sf::Vector2f(bush2.getObj().getPosition().x - moveSpeed, 500.f)));
                bush3.getObj().setPosition((sf::Vector2f(bush3.getObj().getPosition().x - moveSpeed, 500.f)));
                bush4.getObj().setPosition((sf::Vector2f(bush4.getObj().getPosition().x - moveSpeed, 500.f)));
                bush5.getObj().setPosition((sf::Vector2f(bush5.getObj().getPosition().x - moveSpeed, 500.f)));
                bush6.getObj().setPosition((sf::Vector2f(bush6.getObj().getPosition().x - moveSpeed, 500.f)));
                bush7.getObj().setPosition((sf::Vector2f(bush7.getObj().getPosition().x - moveSpeed, 500.f)));
                bush8.getObj().setPosition((sf::Vector2f(bush8.getObj().getPosition().x - moveSpeed, 500.f)));
                bush9.getObj().setPosition((sf::Vector2f(bush9.getObj().getPosition().x - moveSpeed, 500.f)));
                bush10.getObj().setPosition((sf::Vector2f(bush10.getObj().getPosition().x - moveSpeed, 500.f)));
                bush11.getObj().setPosition((sf::Vector2f(bush11.getObj().getPosition().x - moveSpeed, 500.f)));
                bush12.getObj().setPosition((sf::Vector2f(bush12.getObj().getPosition().x - moveSpeed, 500.f)));

            }
            else {
                running_pos = sf::Vector2f(total_length - moving_curr_brixk_x + moveSpeed + mario.getPosition().x, mario.getPosition().y);    
                
            }
            
            defaultpose = 0;
        } 
        // Move mario to left when left key is pressed, but brick stays at same position
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) && !fallen) {
            mario.runBackward();
            running_pos = sf::Vector2f(total_length + mario.getPosition().x - moving_curr_brixk_x, mario.getPosition().y);
            defaultpose = 1;
        }
        

        else {
            // Show standing frame if not moving
            if (!fallen) {
                running_pos = sf::Vector2f(total_length - moving_curr_brixk_x + mario.getPosition().x, mario.getPosition().y);
                if (defaultpose == 1)
                    mario.standBack();
                else
                    mario.stand();
            }
        }

        if(!fallen)
            mario.updateJump();
    }

    // Placeholder for additional logic updates
    void update() {}

    // check some confditions
    void check() {
        sf::Vector2f curr_pos =  running_pos;
        float start = curr_pos.x - 30.f;
        float end = curr_pos.x + 0.f;
        //std::cout << "Reached the blank space - x : " << start << " - y : " << end << std::endl;
        if (brick.empty.count(start) || brick.empty.count(end)) {
            if (curr_pos.y > 450.f) {
                if (!hasPlayedDieSound && soundCooldownClock.getElapsedTime().asMilliseconds() > 3) {
                    ground_play_bg_audio.stop();
                    dieSound.play();
                    soundCooldownClock.restart();
                    hasPlayedDieSound = true;  // Ensure it only runs once
                }
               fallen = true;
               mario.isFalling = true;
               mario.updateFall();

            }
        }
    }

    // Renders background, bricks, and Mario to screen
    void render() {
        window.clear(sf::Color(222, 161, 161));
        window.draw(background);

        

        window.draw(cloud1.getObj());
        window.draw(cloud2.getObj());
        window.draw(cloud3.getObj());
        window.draw(cloud4.getObj());
        window.draw(cloud5.getObj());
        window.draw(cloud6.getObj());
        window.draw(cloud7.getObj());

        window.draw(bush1.getObj());
        window.draw(bush2.getObj());
        window.draw(bush3.getObj());
        window.draw(bush4.getObj());
        window.draw(bush5.getObj());
        window.draw(bush6.getObj());
        window.draw(bush7.getObj());
        window.draw(bush8.getObj());
        window.draw(bush9.getObj());
        window.draw(bush10.getObj());
        window.draw(bush11.getObj());
        window.draw(bush12.getObj());

        for (const auto& row : brickslab) {
            for (const auto& brick : row) {
                window.draw(*brick);
            }
        }
        window.draw(gomma1.getObj());
        window.draw(mario.getObj());
        window.display();
    }
};
