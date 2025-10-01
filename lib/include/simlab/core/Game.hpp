#pragma once
#include <SFML/Graphics.hpp>

#include "simlab/core/PhysicsManager.hpp"
#include "simlab/logger/Logger.hpp"

#include <memory>

constexpr int WindowWidth  = 1920;
constexpr int WindowHeight = 1200;

namespace simlab {

    class Game {
      public:

        Game(const Game&)                    = delete;
        Game(Game&&)                         = delete;
        auto operator=(const Game&) -> Game& = delete;
        auto operator=(Game&&) -> Game&      = delete;

        Game(unsigned int width, unsigned int height, const std::string& title,
             sf::Uint32          style    = sf::Style::Default,
             sf::ContextSettings settings = sf::ContextSettings());

        explicit Game(const std::string&  title,
                      sf::Uint32          style    = sf::Style::Default,
                      sf::ContextSettings settings = sf::ContextSettings());

        Game();

        virtual ~Game() {
            if (m_physicsEngine) {
                physicsManager->stop();  // stop and join thread safely
            }
        }

        void Run();

      protected:

        // --- to be overridden by subclasses ---
        virtual void Update(float dt) {};
        virtual void Draw(sf::RenderWindow& win) = 0;

        // default event handler (can override if needed)
        virtual void handleEvents(sf::Event& event) {};

        void setFramerateLimit(uint limit);

        void setTimeScale(float scale);

        void setFixedUpdateRate(float limit);

        void enablePhysicsEngine();

        void disablePhysicsEngine();

        std::unique_ptr<simlab::PhysicsManager> physicsManager;

        sf::RenderWindow window;
        Logger::Logger&  log;

      private:

        void pollEvents();
        void fixedUpdate(float dt);

        uint  m_frameRate       = 120;
        float m_timeScale       = 1.0F;
        bool  m_physicsEngine   = false;
        bool  m_fixedUpdate     = false;
        float m_updateRateLimit = static_cast<float>(m_frameRate);
        float m_fixedDeltaTime  = 1.0F / m_updateRateLimit;
    };
}  // namespace simlab
