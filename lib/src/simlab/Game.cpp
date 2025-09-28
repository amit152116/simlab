#include "simlab/Game.hpp"

namespace simlab {

    Game::Game(unsigned int width, unsigned int height,
               const std::string& title, sf::Uint32 style,
               sf::ContextSettings settings)
        : m_window(sf::VideoMode(width, height), title, style, settings),
          log(Logger::getLogger()) {
        setFramerateLimit(m_frameRate);
    }

    Game::Game(const std::string& title, sf::Uint32 style,
               sf::ContextSettings settings)
        : m_window(sf::VideoMode(WindowWidth, WindowHeight), title, style,
                   settings),
          log(Logger::getLogger()) {
        setFramerateLimit(m_frameRate);
    }

    Game::Game()
        : m_window(sf::VideoMode(WindowWidth, WindowHeight), "SFML Window",
                   sf::Style::Default),

          log(Logger::getLogger()) {
        setFramerateLimit(m_frameRate);
    }

    void Game::Run() {
        Benchmark bm("Game Loop");
        if (m_physicsEngine) {
            m_physicsManager->setPhysicsFunction(
                [this](float dt) -> void { this->Update(dt); });
            m_physicsManager->setTargetFPS(120);
            m_physicsManager->start();
        }
        sf::Clock clock;
        while (m_window.isOpen()) {
            Benchmark::Scope scope(bm);
            float            dt = clock.restart().asSeconds();
            dt *= m_timeScale;
            pollEvents();
            if (!m_physicsEngine) {
                Update(dt);
                m_window.clear();
                Draw(m_window);
                m_window.display();
            } else {
                m_window.clear();
                m_physicsManager->withDataLock(
                    [this]() -> void { Draw(m_window); });
                m_window.display();
            }
        }
        // Ensure physics thread is stopped before exiting
        if (m_physicsEngine) {
            m_physicsManager->stop();
        }
    }

    void Game::setFramerateLimit(uint limit) {
        m_frameRate = limit;
        m_window.setFramerateLimit(m_frameRate);
    }

    void Game::setTimeScale(float scale) {
        m_timeScale = scale;
    }

    void Game::enablePhysicsEngine() {
        m_physicsManager = std::make_unique<simlab::PhysicsManager>();
        m_physicsEngine  = true;
    }

    void Game::disablePhysicsEngine() {
        m_physicsManager = nullptr;
        m_physicsEngine  = false;
    }

    void Game::pollEvents() {
        sf::Event event{};
        while (m_window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                m_window.close();
                break;  // exit event loop
            }

            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
            if (event.type == sf::Event::KeyPressed &&
                event.key.code == sf::Keyboard::Escape) {
                m_window.close();
            }

            if (m_physicsEngine) {
                // Forward events safely to physics thread if needed
                m_physicsManager->withDataLock(
                    [this, &event]() -> void { handleEvents(event); });
            } else {
                handleEvents(event);
            }
        }
    }
}  // namespace simlab
