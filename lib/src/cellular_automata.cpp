#include "simlab/simlab.hpp"

#include <random>

namespace {

    class CellularAutomata : public simlab::Game {
      private:

        float probabilityOfOne = 0.10F;  // 20% chance of 1, 70% chance of 0
        inline static int      RULE     = 30;
        static constexpr float cellSize = 5.F;

        int gridWidth, gridHeight;

        std::vector<sf::RectangleShape> squares;
        std::vector<bool>               states;

        sf::RenderTexture renderTex;
        sf::Sprite        sprite;

        std::mt19937 generator;
        int          currRow = 1;
        sf::View     view;

      public:

        CellularAutomata()
            : simlab::Game("Cellular Automata", sf::Style::Fullscreen),
              generator(std::random_device{}()) {
            setFramerateLimit(60);
            renderTex.create(window.getSize().x, window.getSize().y);
            sprite.setTexture(renderTex.getTexture());

            view.setSize(window.getSize().x,
                         window.getSize().y);  // view size = window size
            view.setCenter(window.getSize().x / 2.F,
                           window.getSize().y / 2.F);  // initial center
            window.setView(view);

            gridWidth  = window.getSize().x / static_cast<int>(cellSize);
            gridHeight = window.getSize().y / static_cast<int>(cellSize);

            log.info("Grid Width: {}\n", gridWidth);
            log.info("Grid Height: {}\n", gridHeight);

            squares.resize(gridWidth);

            states.resize(gridWidth);
            init();
        }

      private:

        void init() {
            renderTex.clear(sf::Color::Black);
            currRow = 1;
            std::bernoulli_distribution dist(probabilityOfOne);

            for (int i = 0; i < gridWidth; i++) {
                auto state = dist(generator);
                states[i]  = state;
                squares[i].setSize({cellSize, cellSize});
                squares[i].setPosition({i * cellSize, 00});
                squares[i].setFillColor(getColor(states[i]));
            }

            // auto mid    = (gridWidth - 1) / 2;
            // states[mid] = true;
            // squares[mid].setFillColor(getColor(states[mid]));
        }

        static auto calcNextState(bool a, bool b, bool c) -> bool {
            uint8_t pattern = (b << 1) | (a << 2) | c;
            return ((RULE >> pattern) & 1) != 0;  // LSB = pattern 000
        }

        static auto getColor(bool state) -> sf::Color {
            return state ? sf::Color::White : sf::Color::Black;
        }

        void Update(float dt) override {
            if (currRow >= gridHeight) {
                return;
            }
            std::vector<bool> newStates(gridWidth);
            for (int i = 0; i < gridWidth; i++) {
                auto left  = states[(i - 1 + gridWidth) % gridWidth];
                auto mid   = states[i];
                auto right = states[(i + 1 + gridWidth) % gridWidth];

                newStates[i] = calcNextState(left, mid, right);

                squares[i].setPosition({i * cellSize, currRow * cellSize});
                squares[i].setFillColor(getColor(newStates[i]));
            }
            states = std::move(newStates);
            currRow++;
        }

        void Draw(sf::RenderWindow& win) override {
            for (auto& square : squares) {
                renderTex.draw(square);
            }
            renderTex.display();

            win.draw(sprite);
        }

        void handleEvents(sf::Event& event) override {
            static std::uniform_int_distribution<int> distRule(0, 255);
            if (event.type == sf::Event::MouseButtonPressed &&
                event.mouseButton.button == sf::Mouse::Right) {
                RULE = distRule(generator);
                log.info("RULE: {}", RULE);
                init();
            }
        }
    };

}  // namespace

int main(int argc, char* argv[]) {
    CellularAutomata game;
    game.Run();
    return 0;
}
