#include "simlab/simlab.hpp"

#include <algorithm>
#include <random>

namespace {

    class DrawBezierCurve : public simlab::Game {
      private:

        sf::RenderTexture renderTex;
        sf::Sprite        sprite;

        sf::Vector2f startPoint;
        sf::Vector2f endPoint;
        sf::Vector2f midPoint;

        static auto createContextSettings() -> sf::ContextSettings {
            sf::ContextSettings settings;

            settings.sRgbCapable       = true;
            settings.antialiasingLevel = 8;
            settings.depthBits         = 24;
            return settings;
        }

        simlab::BezierCurve curve;

      public:

        DrawBezierCurve()
            : simlab::Game("Bezier Curve", sf::Style::Fullscreen,
                           createContextSettings()) {
            setFramerateLimit(120);
            renderTex.create(window.getSize().x, window.getSize().y,
                             createContextSettings());
            sprite.setTexture(renderTex.getTexture());

            startPoint = {100, 500};
            endPoint   = {1000, 500};
            midPoint   = {500, 250};

            auto controlPoints = {startPoint, midPoint, endPoint};
            curve.setControlPoints(controlPoints);
            curve.append({700, 750});
            curve.append({200, 750});
            curve.append({1000, 1000});
            curve.setPrimitiveType(sf::LineStrip);
            curve.setControlPointRadius(10.0);
            curve.setStep(0.001);
            curve.enableLines(true);
        }

      private:

        void Update(float /*dt*/) override {}

        void Draw(sf::RenderWindow& win) override {
            renderTex.clear(sf::Color::Black);

            renderTex.draw(curve);

            renderTex.display();

            win.draw(sprite);
        };

        void handleEvents(sf::Event& event) override {
            curve.handleEvents(event, window);
        }
    };

}  // namespace

int main(int argc, char* argv[]) {
    DrawBezierCurve game;
    game.Run();
    return 0;
}
