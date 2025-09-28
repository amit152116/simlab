#include "simlab/core/Game.hpp"

namespace simlab {

    Game::Game(unsigned int width, unsigned int height,
               const std::string& title, sf::Uint32 style,
               sf::ContextSettings settings)
        : window(sf::VideoMode(width, height), title, style, settings),
          log(Logger::getLogger()) {
        setFramerateLimit(m_frameRate);
    }

    Game::Game(const std::string& title, sf::Uint32 style,
               sf::ContextSettings settings)
        : window(sf::VideoMode(WindowWidth, WindowHeight), title, style,
                 settings),
          log(Logger::getLogger()) {
        setFramerateLimit(m_frameRate);
    }

    Game::Game()
        : window(sf::VideoMode(WindowWidth, WindowHeight), "SFML Window",
                 sf::Style::Default),

          log(Logger::getLogger()) {
        setFramerateLimit(m_frameRate);
    }

    void Game::Run() {
        Benchmark bm("Game Loop");
        if (m_physicsEngine) {
            physicsManager->setPhysicsFunction(
                [this](float dt) -> void { this->Update(dt); });
            physicsManager->setTargetFPS(120);
            physicsManager->start();
        }
        sf::Clock clock;
        while (window.isOpen()) {
            Benchmark::Scope scope(bm);
            float            dt = clock.restart().asSeconds();
            dt *= m_timeScale;
            pollEvents();
            if (!m_physicsEngine) {
                Update(dt);
                window.clear();
                Draw(window);
                window.display();
            } else {
                window.clear();
                physicsManager->withDataLock(
                    [this]() -> void { Draw(window); });
                window.display();
            }
        }
        // Ensure physics thread is stopped before exiting
        if (m_physicsEngine) {
            physicsManager->stop();
        }
    }

    void Game::setFramerateLimit(uint limit) {
        m_frameRate = limit;
        window.setFramerateLimit(m_frameRate);
    }

    void Game::setTimeScale(float scale) {
        m_timeScale = scale;
    }

    void Game::enablePhysicsEngine() {
        physicsManager  = std::make_unique<simlab::PhysicsManager>();
        m_physicsEngine = true;
    }

    void Game::disablePhysicsEngine() {
        physicsManager  = nullptr;
        m_physicsEngine = false;
    }

    void Game::pollEvents() {
        sf::Event event{};
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                break;  // exit event loop
            }

            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
            if (event.type == sf::Event::KeyPressed &&
                event.key.code == sf::Keyboard::Escape) {
                window.close();
            }

            if (m_physicsEngine) {
                // Forward events safely to physics thread if needed
                physicsManager->withDataLock(
                    [this, &event]() -> void { handleEvents(event); });
            } else {
                handleEvents(event);
            }
        }
    }
}  // namespace simlab
