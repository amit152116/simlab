#include <SFML/Graphics.hpp>
#include <SFML/Window/WindowStyle.hpp>

#include "simlab/simlab.hpp"

#include <random>

constexpr int WindowWidth  = 1920;
constexpr int WindowHeight = 1080;
int           count        = 0;

namespace Game {
    class CollisionGame : public simlab::Game {
      public:

        explicit CollisionGame(std::string title = "SFML Window")
            : simlab::Game(WindowWidth, WindowHeight, title),
              ball(50.F),
              rect({500.F, 10.F}),
              convex(6),
              acceleration(100.F),
              nBalls(10),
              balls(nBalls),
              ballSpeeds(nBalls) {
            setFramerateLimit(120);
            enablePhysicsEngine();
            // m_physicsManager->setFixedTimeStep(false);
            // m_window.setVerticalSyncEnabled(true);

            ball.setFillColor(sf::Color::Black);
            ball.setOutlineColor(sf::Color::Red);
            ball.setOutlineThickness(10);
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
                sf::Color outlineColor(distColor(gen), distColor(gen),
                                       distColor(gen));
                ball.setFillColor(sf::Color::Black);
                ball.setOutlineColor(outlineColor);

                ball.setOutlineThickness(2.F);
            }

            float minSpeed = -500.F;
            float maxSpeed = 500.F;

            // Distribution for speed
            std::uniform_real_distribution<float> distSpeed(minSpeed, maxSpeed);

            for (auto& speed : ballSpeeds) {
                speed = {distSpeed(gen), distSpeed(gen)};
                log.info("Ball speed: {}\n", speed);
            }

            rect.setFillColor(sf::Color::Red);
            rect.setOutlineColor(sf::Color::Black);
            rect.setPosition({250, 100});
            rect.rotate(10);
            log.info("Rect has {} points\n", rect.getPointCount());

            trailCanvas.create(m_window.getSize().x, m_window.getSize().y);
            trailCanvas.clear(sf::Color::Transparent);  // start empty

            trailSprite = sf::Sprite(trailCanvas.getTexture());

            convex.setPoint(0, {50, 50});
            convex.setPoint(1, {200, 50});
            convex.setPoint(2, {200, 400});
            convex.setPoint(3, {50, 300});
            convex.setPoint(4, {100, 50});
            convex.setPoint(5, {50, 50});
            convex.setFillColor(sf::Color::Red);
            convex.setOutlineColor(sf::Color::Black);
            convex.setOutlineThickness(1);

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

        sf::CircleShape    ball;
        sf::RectangleShape rect;
        sf::ConvexShape    convex;
        sf::RenderTexture  trailCanvas;
        sf::Sprite         trailSprite;
        sf::Text           text;
        sf::Font           font;
        sf::Vector2f       ballSpeed;  // pixels per second
        sf::Vector2f       velocity;
        sf::Vector2f       direction;
        float              acceleration{};

        const int                    nBalls = 5;
        std::vector<sf::CircleShape> balls;
        std::vector<sf::Vector2f>    ballSpeeds;

        void Update(float dt) override {
            auto ballDir = simlab::normalize(ballSpeed);

            ballSpeed += ballDir * acceleration / 2.F * dt;

            // 2. Predict next position
            predictNextPosition(ball, ballSpeed, dt);
            windowCollision(ball, ballSpeed, dt);

            for (int i = 0; i < nBalls; i++) {
                auto& ball  = balls[i];
                auto& speed = ballSpeeds[i];

                predictNextPosition(ball, speed, dt);
                windowCollision(ball, speed, dt);
            }

            // Then handle elastic collisions between balls
            for (int i = 0; i < nBalls; i++) {
                for (int j = i + 1; j < nBalls; j++) {
                    elasticCollision(balls[i], balls[j], ballSpeeds[i],
                                     ballSpeeds[j]);
                }
                elasticCollision(ball, balls[i], ballSpeed, ballSpeeds[i]);
            }

            ballSpeed += ballDir * acceleration / 2.F * dt;
        }

        static void predictNextPosition(sf::CircleShape& circle,
                                        sf::Vector2f& velocity, float dt) {
            // Predict next position
            auto predictedPos = circle.getPosition() + velocity * dt;
            // Update ball position
            circle.setPosition(predictedPos);
        }

        void windowCollision(sf::CircleShape& circle, sf::Vector2f& velocity,
                             float dt) {
            // Handle window collision & correct position
            auto windowCollision =
                simlab::Collision::windowCollision(circle, m_window);
            if (windowCollision.collided) {
                velocity = simlab::reflect(velocity, windowCollision.normal);
                // Correct position to move ball out of the wall
                auto predictedPos =
                    circle.getPosition() +
                    windowCollision.normal * windowCollision.penetration;
                circle.setPosition(predictedPos);
            }
        }

        // Elastic collision response between two circles
        static void elasticCollision(sf::CircleShape& circle1,
                                     sf::CircleShape& circle2,
                                     sf::Vector2f&    velocity1,
                                     sf::Vector2f&    velocity2) {
            auto collision =
                simlab::Collision::circleCollision(circle1, circle2);

            if (!collision.collided) {
                return;
            }
            count++;
            fmt::print("Collisions: {}\n", count);

            // Get masses (assuming uniform density, mass = area)
            float mass1 = circle1.getRadius() * circle1.getRadius();
            float mass2 = circle2.getRadius() * circle2.getRadius();

            // Separate circles to prevent overlap
            sf::Vector2f pos1 = circle1.getPosition();
            sf::Vector2f pos2 = circle2.getPosition();

            // Move circles apart based on their masses
            float totalMass   = mass1 + mass2;
            float separation1 = collision.penetration * (mass2 / totalMass);
            float separation2 = collision.penetration * (mass1 / totalMass);

            pos1 -= collision.normal * separation1;
            pos2 += collision.normal * separation2;

            circle1.setPosition(pos1);
            circle2.setPosition(pos2);

            // Calculate relative velocity
            sf::Vector2f relativeVelocity = velocity2 - velocity1;

            // Calculate relative velocity along collision normal
            float velocityAlongNormal =
                (relativeVelocity.x * collision.normal.x) +
                (relativeVelocity.y * collision.normal.y);

            // Don't resolve if velocities are separating
            if (velocityAlongNormal > 0) {
                return;
            }

            // Calculate restitution (bounciness) - 1.0 for perfectly elastic
            float restitution = 1.0F;

            // Calculate impulse scalar
            float impulseScalar = -(1.0F + restitution) * velocityAlongNormal;
            impulseScalar /= ((1.0F / mass1) + (1.0F / mass2));

            // Apply impulse
            sf::Vector2f impulse = impulseScalar * collision.normal;
            velocity1 -= impulse / mass1;
            velocity2 += impulse / mass2;
        }

        void Draw(sf::RenderWindow& win) override {
            // window.draw(rect);
            // window.draw(text);
            // window.setView(window.getDefaultView());
            for (auto& ball : balls) {
                win.draw(ball);
            }
            win.draw(ball);
            // window.draw(trailSprite);
            // window.draw(convex);
        }

        void handleEvents(sf::Event& event) override {
            switch (event.type) {
                case sf::Event::KeyPressed:
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                    if (event.key.code == sf::Keyboard::Escape) {
                        m_window.close();
                    }
                    break;

                default:
                    break;
            }
        }
    };

}  // namespace Game

auto main() -> int {
    Game::CollisionGame game;
    game.Run();
    return 0;
}
