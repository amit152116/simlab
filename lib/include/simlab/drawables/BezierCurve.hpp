
#include <SFML/Graphics.hpp>

#include "simlab/core/utils.hpp"

namespace simlab {

    class BezierCurve : public sf::Drawable {
      public:

        explicit BezierCurve(std::vector<sf::Vector2f> controlPoints,
                             double                    step = 0.05);

        explicit BezierCurve(int nPoints, double step = 0.05);
        BezierCurve();

        auto setControlPoint(size_t index, sf::Vector2f point) -> void;

        auto setControlPoints(std::vector<sf::Vector2f> points) -> void;

        auto setControlPointRadius(float radius) -> void;

        auto setStep(double step) -> void;

        void setPrimitiveType(sf::PrimitiveType type);

        void setCurveColor(sf::Color color);

        void setControlPointColor(sf::Color color);

        auto setTextFont(const std::string& filename) -> void;

        auto setTextFont(sf::Font font) -> void;

        auto setTextColor(sf::Color color) -> void;

        void resize(std::size_t controlPointCount);

        void clear();

        void append(sf::Vector2f point);

        auto operator[](std::size_t index) const -> const sf::Vector2f&;

        void handleEvents(const sf::Event& event, sf::RenderWindow& window);

        void draw(sf::RenderTarget& target,
                  sf::RenderStates  states) const override;

        auto getControlPoint(size_t index) -> sf::Vector2f;

        auto getControlPoints() -> std::vector<sf::Vector2f>;

        auto enableDotLines(bool enabled) -> void;
        auto enableControlPoints(bool enabled) -> void;
        auto enableLines(bool enabled) -> void;

      private:

        static auto createControlPoint(const sf::Vector2f& pos, sf::Color color,
                                       float radius) -> sf::CircleShape;

        auto createControlText(const std::string& str, sf::Vector2f pos)
            -> sf::Text;
        auto updateCurve() -> void;

        static auto getCurvePoints(std::vector<sf::Vector2f>& pointArray,
                                   float t, sf::VertexArray& lines)
            -> std::vector<sf::Vector2f>;

        sf::VertexArray              curve;
        sf::VertexArray              dotlines;
        sf::VertexArray              lines;
        std::vector<sf::Vector2f>    controlPoints;
        std::vector<sf::CircleShape> controlPointsShapes;
        std::vector<sf::Text>        texts;
        sf::Font                     font;

        std::string filename = "assets/Fonts/DancingScript-Regular.ttf";

        double    step               = 0.05;
        float     controlPointRadius = 8.F;
        sf::Color controlPointColor  = sf::Color::White;
        sf::Color curveColor         = sf::Color::Blue;
        sf::Color textColor          = sf::Color::White;

        bool showDotLines_      = false;
        bool showControlPoints_ = true;
        bool showLines_         = false;
    };

}  // namespace simlab
