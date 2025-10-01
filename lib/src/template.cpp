#include "simlab/simlab.hpp"

namespace {

    class Template : public simlab::Game {
      private:

        static auto createContextSettings() -> sf::ContextSettings {
            sf::ContextSettings settings;
            settings.sRgbCapable       = true;
            settings.antialiasingLevel = 8;
            return settings;
        }

      public:

        Template()
            : Game("Template", sf::Style::Fullscreen, createContextSettings()) {
        }

      private:

        void Update(float dt) override {}

        void Draw(sf::RenderWindow& win) override {}

        void handleEvents(sf::Event& event) override {}
    };
}  // namespace

auto main(int /*argc*/, char* /*argv*/[]) -> int {
    Template game;
    game.Run();
    return 0;
}
