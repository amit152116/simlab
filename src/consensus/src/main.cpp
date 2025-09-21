#include <SFML/Graphics.hpp>
#include <SFML/Window/WindowStyle.hpp>

constexpr int FRAME_RATE = 60;
constexpr int WINDOW_WIDTH = 1920;
constexpr int WINDOW_HEIGHT = 1080;

auto main() -> int {
  sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT),
                          "SFML Window", sf::Style::Default);
  window.setFramerateLimit(FRAME_RATE);
  sf::CircleShape circle(50.F);
  circle.setFillColor(sf::Color::Cyan);
  circle.setOutlineColor(sf::Color::Black);

  sf::RectangleShape rect(sf::Vector2f(120.F, 75.F));
  rect.setFillColor(sf::Color::Red);
  rect.setOutlineColor(sf::Color::Black);
  rect.setPosition(sf::Vector2f(50, 100));

  while (window.isOpen()) {
    sf::Event event{};
    while (window.pollEvent(event)) {
      switch (event.type) {
      case sf::Event::Closed:
        window.close();
        break;
      case sf::Event::KeyPressed:
        switch (event.key.code) {
        case sf::Keyboard::Escape:
          window.close();
          break;
        default:
          break;
        }

      default:
        break;
      }
    }
    auto pos = circle.getPosition();
    auto grid = rect.getGlobalBounds();

    if (pos.x > grid.left && pos.x < grid.left + grid.width &&
        pos.y > grid.top && pos.y < grid.top + grid.height) {
      circle.move(0.5F, 0.3F);
      rect.rotate(10);
    } else {
      circle.move(0.1F, 0.2F);
    }

    window.clear();
    window.draw(rect);
    window.draw(circle);
    window.display();
  }

  return 0;
}
