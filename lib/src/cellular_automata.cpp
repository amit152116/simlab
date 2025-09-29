#include "simlab/simlab.hpp"

#include <random>

namespace {

    class CellularAutomata : public simlab::Game {
      private:

        inline static int      RULE    = 30;
        static constexpr float boxSize = 5.F;

        int gridWidth, gridHeight;

        std::vector<sf::RectangleShape> squares;
        std::vector<int>                states;

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

            gridWidth  = window.getSize().x / static_cast<int>(boxSize);
            gridHeight = window.getSize().y / static_cast<int>(boxSize);

            log.info("Grid Width: {}\n", gridWidth);
            log.info("Grid Height: {}\n", gridHeight);

            squares.resize(gridWidth);

            states.resize(gridWidth);
            initalize();
        }

        void initalize() {
            renderTex.clear(sf::Color::White);
            currRow = 1;
            static std::uniform_int_distribution<int> random(0, 1);

            for (int i = 0; i < gridWidth; i++) {
                auto state = random(generator);
                states[i]  = 0;
                squares[i].setSize({boxSize, boxSize});
                squares[i].setPosition({i * boxSize, 00});
                squares[i].setFillColor(sf::Color::White);
                if (states[i] == 1) {
                    squares[i].setFillColor(sf::Color::Black);
                }
            }

            auto mid    = (gridWidth - 1) / 2;
            states[mid] = 1;
            squares[mid].setFillColor(sf::Color::Black);
        }

        static auto cellular_state(int a, int b, int c) -> int {
            uint8_t pattern = (b << 1) | (a << 2) | c;
            return (RULE >> pattern) &
                   1;  // shift right by pattern index, LSB = pattern 000
        }

        void Update(float dt) override {
            if (currRow >= gridHeight) {
                return;
            }
            std::vector<int> newStates(gridWidth);
            for (int i = 0; i < gridWidth; i++) {
                auto left  = states[(i - 1 + gridWidth) % gridWidth];
                auto mid   = states[i];
                auto right = states[(i + 1 + gridWidth) % gridWidth];

                newStates[i] = cellular_state(left, mid, right);

                squares[i].setPosition({i * boxSize, currRow * boxSize});
                squares[i].setFillColor(newStates[i] == 0 ? sf::Color::White
                                                          : sf::Color::Black);
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
                initalize();
            }
        }
    };

}  // namespace

int main(int argc, char* argv[]) {
    CellularAutomata game;
    game.Run();
    return 0;
}
