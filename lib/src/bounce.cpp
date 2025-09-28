
#include <SFML/Graphics.hpp>
#include <SFML/Window/WindowStyle.hpp>

#include "simlab/simlab.hpp"

#include <array>
#include <random>
#include <unordered_map>

namespace Game {
    class CollisionGame : public simlab::Game {
      private:

        sf::RenderTexture renderTex;
        sf::Sprite        sprite;
        sf::CircleShape   ball;
        sf::Text          text;
        sf::Font          font;
        sf::Vector2f      ballSpeed;  // pixels per second
        sf::Vector2f      velocity;
        float             acceleration{};

        const int                    nBalls = 5;
        std::vector<sf::CircleShape> balls;
        std::vector<sf::Vector2f>    ballSpeeds;

        static auto createContextSettings() -> sf::ContextSettings {
            sf::ContextSettings settings;
            settings.sRgbCapable       = true;
            settings.antialiasingLevel = 8;
            return settings;
        }

      public:

        explicit CollisionGame(std::string title = "SFML Window")
            : simlab::Game(title, sf::Style::Fullscreen,
                           createContextSettings()),
              ball(50.F),
              acceleration(100.F),
              nBalls(10),
              balls(nBalls),
              ballSpeeds(nBalls) {
            setFramerateLimit(120);
            enablePhysicsEngine();
            // m_physicsManager->setFixedTimeStep(false);
            // m_window.setVerticalSyncEnabled(true);

            renderTex.create(m_window.getSize().x, m_window.getSize().y);
            sprite.setTexture(renderTex.getTexture());

            ball.setFillColor(sf::Color::Black);
            ball.setOutlineColor(sf::Color::Green);
            ball.setOutlineThickness(3);
            ball.setPosition({250, 250});
            ball.setOrigin(ball.getRadius(), ball.getRadius());
            log.info("Circle has {} points\n", ball.getPointCount());
            log.info("Circle Origin: {}\n", ball.getOrigin());
            log.info("Circle Position: {}\n", ball.getPosition());

            std::random_device rd;
            std::mt19937       gen(rd());

            // Radius range
            float minRadius = 20.F;
            float maxRadius = 50.F;

            // Distribution for radius
            std::uniform_real_distribution<float> distRadius(minRadius,
                                                             maxRadius);
            std::uniform_real_distribution<float> distColor(0.F, 255.F);
            for (auto& ball : balls) {
                float radius = distRadius(gen);
                ball.setRadius(radius);
                ball.setOrigin(ball.getRadius(), ball.getRadius());

                // Random position within window bounds (account for radius)
                std::uniform_real_distribution<float> distX(
                    radius, WindowWidth - radius);
                std::uniform_real_distribution<float> distY(
                    radius, WindowHeight - radius);

                float x = distX(gen);
                float y = distY(gen);
                ball.setPosition(x, y);

                // Random fill and outline colors
                sf::Color fillColor(distColor(gen), distColor(gen),
                                    distColor(gen));
                ball.setFillColor(fillColor);
                // ball.setOutlineColor(outlineColor);

                // ball.setOutlineThickness(2.F);
            }

            float minSpeed = -200.F;
            float maxSpeed = 200.F;

            // Distribution for speed
            std::uniform_real_distribution<float> distSpeed(minSpeed, maxSpeed);

            for (auto& speed : ballSpeeds) {
                speed = {distSpeed(gen), distSpeed(gen)};
            }

            font.loadFromFile("assets/Fonts/DancingScript-Regular.ttf");

            text.setFont(font);
            text.setOutlineColor(sf::Color::Green);
            text.setOutlineThickness(3);
            text.setCharacterSize(30);
            text.setFillColor(sf::Color::Black);
            text.setString("Hello, world!");
            text.setPosition({1000, 500});

            ballSpeed = {250.F, 250.F};
            velocity  = {500, 500};
        }

      private:

        void Update(float dt) override {
            int                counter  = 0;
            static const float cellSize = ball.getRadius() * 2.F;

            std::unordered_map<sf::Vector2i, std::vector<int>, Vector2Hash<int>>
                gridBucket;

            auto ballDir = simlab::normalize(ballSpeed);

            ballSpeed += ballDir * acceleration / 2.F * dt;

            // 2. Predict next position
            predictNextPosition(ball, ballSpeed, dt);
            windowCollision(m_window, ball, ballSpeed);

            for (int i = 0; i < nBalls; i++) {
                auto& ball  = balls[i];
                auto& speed = ballSpeeds[i];

                predictNextPosition(ball, speed, dt);
                windowCollision(m_window, ball, speed);
                auto cell = simlab::toVector2i(ball.getPosition() / cellSize);
                gridBucket[cell].push_back(i);

                simlab::Collision::elasticCollisionAdvanced(
                    this->ball, balls[i], ballSpeed, ballSpeeds[i]);
            }

            // Offsets for 8 neighbors + current cell
            static const std::array<sf::Vector2i, 9> neighborOffsets = {
                sf::Vector2i(0, 0),  sf::Vector2i(1, 0),  sf::Vector2i(-1, 0),
                sf::Vector2i(0, 1),  sf::Vector2i(0, -1), sf::Vector2i(1, 1),
                sf::Vector2i(-1, 1), sf::Vector2i(1, -1), sf::Vector2i(-1, -1)};

            for (auto& [cell, ballIdx] : gridBucket) {
                for (int idx : ballIdx) {
                    auto& ball1 = balls[idx];

                    // Neighbor cells
                    for (auto offset : neighborOffsets) {
                        sf::Vector2i neighbor = cell + offset;
                        if (gridBucket.count(neighbor) == 0) {
                            continue;
                        }

                        for (auto& j : gridBucket[neighbor]) {
                            auto& ball2 = balls[j];
                            if (&ball1 == &ball2) {
                                continue;
                            }
                            counter++;
                            simlab::Collision::elasticCollisionAdvanced(
                                ball1, ball2, ballSpeeds[idx], ballSpeeds[j]);
                        }
                    }
                }
            }
            log.debug("Collisions: {}", counter);
            ballSpeed += ballDir * acceleration / 2.F * dt;
        }

        static void predictNextPosition(sf::CircleShape& circle,
                                        sf::Vector2f& velocity, float dt) {
            // Predict next position
            auto predictedPos = circle.getPosition() + velocity * dt;
            // Update ball position
            circle.setPosition(predictedPos);
        }

        static void windowCollision(sf::RenderWindow& window,
                                    sf::CircleShape&  circle,
                                    sf::Vector2f&     velocity) {
            // Handle window collision & correct position
            auto windowCollision =
                simlab::Collision::windowCollision(circle, window);
            if (windowCollision.collided) {
                velocity = simlab::reflect(velocity, windowCollision.normal);
                // Correct position to move ball out of the wall
                auto predictedPos =
                    circle.getPosition() +
                    windowCollision.normal * windowCollision.penetration;
                circle.setPosition(predictedPos);
            }
        }

        void Draw(sf::RenderWindow& win) override {
            renderTex.clear(sf::Color::Black);
            renderTex.draw(ball);
            for (auto& ball : balls) {
                renderTex.draw(ball);
            }

            renderTex.display();
            win.draw(sprite);
        }

        void handleEvents(sf::Event& event) override {}
    };

}  // namespace Game

auto main() -> int {
    Game::CollisionGame game;

    game.Run();
    return 0;
}
