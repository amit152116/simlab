
#include "simlab/simlab.hpp"

#include <optional>
#include <random>
#include <unordered_map>

namespace {

    class PosissonDiscSampling : public simlab::Game {
      private:

        std::vector<std::optional<sf::Vector2f>> grid;
        std::vector<sf::CircleShape>             activePoints;

        std::unordered_map<int, sf::CircleShape> points;

        const float            R         = 20.F;
        static constexpr float radius    = 5.F;
        const int              K         = 30;
        const int              Dimension = 2;
        const double           cellSize  = R / std::sqrt(Dimension);

        const int width;
        const int height;

        int rows, cols;
        int counter = 0;

        std::mt19937 generator;

        std::uniform_real_distribution<float> distAngle;
        std::uniform_real_distribution<float> distL;

        sf::RenderTexture renderTex;
        sf::Sprite        pointSprite;

        static auto createContextSettings() -> sf::ContextSettings {
            sf::ContextSettings settings;
            settings.sRgbCapable       = true;
            settings.antialiasingLevel = 8;
            return settings;
        }

      public:

        PosissonDiscSampling()
            : simlab::Game("Posisson-Disc Sampling", sf::Style::Fullscreen,
                           createContextSettings()),
              width(static_cast<int>(window.getSize().x)),
              height(static_cast<int>(window.getSize().y)),
              rows(std::floor(height / cellSize)),
              cols(std::floor(width / cellSize)),
              generator(std::random_device{}()),
              distAngle(0.F, 2 * M_PI),
              distL(R, 2.F * R) {
            setFramerateLimit(120);
            log.info("R: {}\n", R);
            log.info("cellSize: {}\n", cellSize);
            renderTex.create(width, height);
            pointSprite.setTexture(renderTex.getTexture());
            initalize();
        }

      private:

        void initalize() {
            counter = 0;
            renderTex.clear(sf::Color::Black);
            renderTex.display();  // IMPORTANT: Display after clear

            // STEP 1
            activePoints.clear();
            grid.clear();
            points.clear();
            grid.resize(cols * rows);
            points.reserve(cols * rows);

            log.info("Grid size: {}\n", grid.size());

            // STEP 2

            // Initial point setup
            sf::Vector2f pos         = {static_cast<float>(width / 2),
                                        static_cast<float>(height / 2)};
            int          col         = std::floor(pos.x / cellSize);
            int          row         = std::floor(pos.y / cellSize);
            grid[col + (row * cols)] = pos;

            sf::CircleShape point;
            setProperties(point, pos);
            points.emplace(col + (row * cols), point);

            // Draw initial point to texture
            renderTex.draw(point);
            renderTex.display();

            // Add to active points for algorithm (not for rendering)
            activePoints.push_back(point);
        }

        void activatePoint(sf::CircleShape& point) {
            point.setFillColor(sf::Color::Red);
            activePoints.emplace_back(point);
        }

        void setProperties(sf::CircleShape& point, sf::Vector2f& position) {
            point.setRadius(radius);
            point.setOrigin(point.getRadius(), point.getRadius());

            std::uniform_real_distribution<float> distColor(0.F, 255.F);
            sf::Color fillColor(distColor(generator), distColor(generator),

                                distColor(generator));
            point.setFillColor(fillColor);
            point.setPosition(position);
        }

      private:

        void Draw(sf::RenderWindow& win) override {
            win.draw(pointSprite);

            for (auto& point : activePoints) {
                win.draw(point);
            }
        }

        void Update(float /*dt*/) override {
            counter++;
            for (int total = 0; total < counter; total++) {
                if (activePoints.empty()) {
                    return;
                }
                std::uniform_int_distribution<int> dist(
                    0, activePoints.size() - 1);

                int idx = dist(generator);

                auto activePoint = activePoints[idx];

                bool found = false;

                for (int count = 0; count < K; count++) {
                    auto angle  = distAngle(generator);
                    auto length = distL(generator);

                    sf::Vector2f sample{std::cos(angle) * length,
                                        std::sin(angle) * length};

                    sample += activePoint.getPosition();

                    int col = std::floor(sample.x / cellSize);
                    int row = std::floor(sample.y / cellSize);

                    if (col < 0 || col >= cols || row < 0 || row >= rows) {
                        continue;
                    }
                    int gridIdx = col + (row * cols);

                    if (points.find(gridIdx) != points.end()) {
                        continue;
                    }
                    auto ok = true;
                    for (auto i = -1; i <= 1; i++) {
                        for (auto j = -1; j <= 1; j++) {
                            int ncol = col + i;
                            int nrow = row + j;

                            if (ncol < 0 || ncol >= cols || nrow < 0 ||
                                nrow >= rows) {
                                continue;
                            }

                            int  index    = ncol + (nrow * cols);
                            auto neighbor = grid[index];
                            if (!neighbor) {
                                continue;
                            }

                            if (utils::distance(sample, neighbor.value()) <=
                                R) {
                                ok = false;
                                break;
                            }
                        }
                    }
                    if (ok) {
                        grid[gridIdx] = sample;
                        sf::CircleShape point;
                        setProperties(point, sample);
                        points.emplace(gridIdx, point);
                        found = true;
                        renderTex.draw(point);
                        activatePoint(point);
                    }
                }
                if (!found) {
                    std::swap(activePoints[idx], activePoints.back());
                    activePoints.pop_back();
                }
            }
        }

        void handleEvents(sf::Event& event) override {
            if (event.type == sf::Event::MouseButtonPressed) {
                initalize();
            }
        }
    };

}  // namespace

auto main(int /*argc*/, char* /*argv*/[]) -> int {
    PosissonDiscSampling game;
    game.Run();
    return 0;
}
