#pragma once
#include <SFML/Graphics.hpp>

#include "simlab/Benchmark.hpp"
#include "simlab/PhysicsManager.hpp"
#include "simlab/logger/Logger.hpp"

constexpr int WindowWidth  = 1920;
constexpr int WindowHeight = 1080;

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
                m_physicsManager->stop();  // stop and join thread safely
            }
        }

        void Run();

      protected:

        // --- to be overridden by subclasses ---
        virtual void Update(float dt)            = 0;
        virtual void Draw(sf::RenderWindow& win) = 0;

        // default event handler (can override if needed)
        virtual void handleEvents(sf::Event& event) {};

        void setFramerateLimit(uint limit);

        void setTimeScale(float scale);

        void enablePhysicsEngine();

        void disablePhysicsEngine();

        std::unique_ptr<simlab::PhysicsManager> m_physicsManager;

        sf::RenderWindow m_window;
        Logger::Logger&  log;

      private:

        void pollEvents();

        uint  m_frameRate     = 60;
        float m_timeScale     = 1.0F;
        bool  m_physicsEngine = false;
    };
}  // namespace simlab
