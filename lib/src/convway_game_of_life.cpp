#include "simlab/simlab.hpp"

#include <random>
#include <unordered_set>

namespace {

    class ConvwayGame : public simlab::Game {
      private:

        float probabilityOfOne = 0.25F;  // 20% chance of 1, 70% chance of 0
        float cellSize         = 10.F;

        sf::RenderTexture renderGrid;
        sf::RenderTexture renderTex;
        sf::Sprite        gridSprite;
        sf::Sprite        sprite;
        int               gridWidth, gridHeight;
        sf::Color         color = sf::Color(150, 150, 150, 250);

        std::vector<std::vector<bool>> grid;
        std::vector<sf::Vector2i>      dragPos;

        static auto createContextSettings() -> sf::ContextSettings {
            sf::ContextSettings settings;
            settings.sRgbCapable       = true;
            settings.antialiasingLevel = 8;
            return settings;
        }

      public:

        ConvwayGame()
            : Game("Convway's Game of Life", sf::Style::Fullscreen,
                   createContextSettings()) {
            setFramerateLimit(120);
            setFixedUpdateRate(10);

            renderGrid.create(window.getSize().x, window.getSize().y);
            gridSprite.setTexture(renderGrid.getTexture());

            renderTex.create(window.getSize().x, window.getSize().y);
            sprite.setTexture(renderTex.getTexture());

            gridWidth  = window.getSize().x / static_cast<int>(cellSize);
            gridHeight = window.getSize().y / static_cast<int>(cellSize);

            auto color = this->color;
            color.a    = 100;
            utils::DrawGrid(renderGrid, cellSize, color);
            renderGrid.display();
            init();
        }

      private:

        void init() {
            renderTex.clear(sf::Color::Transparent);
            grid.clear();

            std::bernoulli_distribution dist(probabilityOfOne);
            std::mt19937                generator(std::random_device{}());

            grid.resize(gridHeight);
            for (int i = 0; i < gridHeight; i++) {
                grid[i].resize(gridWidth);
                for (int j = 0; j < gridWidth; j++) {
                    // grid[i][j] = dist(generator);
                    if (grid[i][j]) {
                        drawRectangle(i, j);
                    }
                }
            }
        }

        auto calcNextState(int row, int col) -> bool {
            bool state = grid[row][col];
            int  sum   = 0;

            for (int di = -1; di <= 1; di++) {
                for (int dj = -1; dj <= 1; dj++) {
                    if (di == 0 && dj == 0) {
                        continue;  // skip self
                    }

                    int ni = (row + di + gridHeight) % gridHeight;
                    int nj = (col + dj + gridWidth) % gridWidth;

                    if (grid[ni][nj]) {
                        sum++;
                    }
                }
            }

            return (state && (sum == 2 || sum == 3)) || (!state && sum == 3);
        }

        void Update(float /*dt*/) override {
            renderTex.clear(sf::Color::Transparent);

            std::vector<std::vector<bool>> nextGrid(gridHeight);

            for (int i = 0; i < gridHeight; i++) {
                nextGrid[i].resize(gridWidth);
                for (int j = 0; j < gridWidth; j++) {
                    nextGrid[i][j] = calcNextState(i, j);

                    if (nextGrid[i][j]) {
                        drawRectangle(i, j);
                    }
                }
            }
            grid = std::move(nextGrid);
        }

        void drawRectangle(int i, int j) {
            sf::Vector2f center((j * cellSize) + (cellSize / 2.F),
                                (i * cellSize) + (cellSize / 2.F));

            float cx = gridWidth / 2.F;
            float cy = gridHeight / 2.F;

            float dx   = (j - cx) / cx;
            float dy   = (i - cy) / cy;
            float dist = std::sqrt((dx * dx) + (dy * dy));

            sf::Color color = utils::HSVtoRGB(dist, 1.0F, 1.0F);
            auto      rect =
                utils::GenerateRectangle(center, {cellSize, cellSize}, color);
            renderTex.draw(rect);
        }

        void Draw(sf::RenderWindow& win) override {
            for (const auto& drag : dragPos) {
                drawRectangle(drag.y, drag.x);
            }
            renderTex.display();
            win.draw(gridSprite);
            win.draw(sprite);
        }

        void handleEvents(sf::Event& event) override {
            static bool dragging = false;

            sf::Vector2f mouseWorld =
                window.mapPixelToCoords(sf::Mouse::getPosition(window));

            sf::Vector2i mousePos = utils::toVector2i(
                {mouseWorld.x / cellSize, mouseWorld.y / cellSize});

            const auto& type   = event.type;
            const auto& button = event.mouseButton.button;

            if (type == sf::Event::MouseButtonPressed &&
                button == sf::Mouse::Right) {
                dragging = false;
                dragPos.clear();
                init();
            }

            if (type == sf::Event::MouseButtonPressed &&
                button == sf::Mouse::Left) {
                dragging = true;
                auto it  = std::find(dragPos.begin(), dragPos.end(), mousePos);
                if (it != dragPos.end()) {
                    dragPos.erase(it);
                } else {
                    dragPos.emplace_back(mousePos);
                }
            }

            if (type == sf::Event::MouseButtonReleased &&
                button == sf::Mouse::Left) {
                dragging = false;
            }

            if (type == sf::Event::MouseButtonPressed &&
                button == sf::Mouse::Middle) {
                dragging = false;

                for (auto& pos : dragPos) {
                    grid[pos.y][pos.x] = true;
                }

                dragPos.clear();
            }

            if (dragging && event.type == sf::Event::MouseMoved) {
                dragPos.emplace_back(mousePos);
            }
        }
    };

}  // namespace

auto main(int /*argc*/, char* /*argv*/[]) -> int {
    ConvwayGame game;
    game.Run();
    return 0;
}
