#include "simlab/simlab.hpp"

#include <cmath>
#include <random>

namespace {

    class Phyllotaxis : public simlab::Game {
      private:

        sf::RenderTexture renderTex;
        sf::Sprite        sprite;

        sf::VertexArray points;

        int n = 0;
        int c = 5;

        static auto createContextSettings() -> sf::ContextSettings {
            sf::ContextSettings settings;
            settings.sRgbCapable       = true;
            settings.antialiasingLevel = 8;
            return settings;
        }

      public:

        Phyllotaxis()
            : Game("Phyllotaxis", sf::Style::Fullscreen,
                   createContextSettings()),
              points(sf::PrimitiveType::Points) {
            renderTex.create(window.getSize().x, window.getSize().y,
                             createContextSettings());
            sprite.setTexture(renderTex.getTexture());
        }

      private:

        void GeneratePoints(int total = 1) {
            for (int i = 0; i < total; i++) {
                static std::uniform_int_distribution<int> distColor(0, 255);
                std::mt19937                              generator(42);

                auto theta  = n * 137.5;
                auto radius = c * std::sqrt(n);

                theta *= DEG_TO_RAD;

                sf::Vector2f pos(
                    static_cast<float>((std::cos(theta) * radius) +
                                       (window.getSize().x / 2.F)),
                    static_cast<float>((std::sin(theta) * radius) +
                                       (window.getSize().y / 2.F)));

                if (pos.x < 0 || pos.x > window.getSize().x || pos.y < 0 ||
                    pos.y > window.getSize().y) {
                    continue;
                }

                // 🌈 smooth rainbow effect
                float     hue   = std::fmod(n * 0.5F, 360.F);  // 0.5° per step
                sf::Color color = utils::HSLtoRGB(hue, 1.F, 0.5F);

                sf::Vertex vertex(pos, color);
                points.append(vertex);
                points.setPrimitiveType(sf::PrimitiveType::Points);

                n++;
            }
        }

        void Draw(sf::RenderWindow& win) override {
            renderTex.clear(sf::Color::Black);
            GeneratePoints(10);
            renderTex.draw(points);
            renderTex.display();
            win.draw(sprite);
        }

        void handleEvents(sf::Event& event) override {
            if (event.type == sf::Event::MouseButtonPressed &&
                event.mouseButton.button == sf::Mouse::Right) {
                n = 0;
                points.clear();
            }
        }
    };
}  // namespace

auto main(int /*argc*/, char* /*argv*/[]) -> int {
    Phyllotaxis game;
    game.Run();
    return 0;
}
