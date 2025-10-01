#include "simlab/drawables/BezierCurve.hpp"

#include <random>

namespace Drawables {
    using sf::Lines;

    BezierCurve::BezierCurve(std::vector<sf::Vector2f> controlPoints,
                             double                    step)
        : curve(sf::LinesStrip),
          dotlines(sf::PrimitiveType::Lines),
          lines(sf::Lines),
          controlPointsShapes(controlPoints.size()),
          texts(controlPoints.size()),
          step(step) {
        setControlPoints(controlPoints);
        setTextFont(filename);
        updateCurve();
    }

    BezierCurve::BezierCurve(int nPoints, double step)
        : curve(sf::LinesStrip),
          dotlines(sf::Lines),
          lines(Lines),
          controlPoints(nPoints),
          controlPointsShapes(nPoints),
          texts(controlPoints.size()),
          step(step) {
        setTextFont(filename);
    }

    BezierCurve::BezierCurve()
        : curve(sf::LinesStrip), dotlines(sf::Lines), lines(sf::Lines) {
        setTextFont(filename);
    }

    auto BezierCurve::setControlPoint(size_t index, sf::Vector2f point)
        -> void {
        if (index < 0 || index >= controlPoints.size()) {
            throw std::runtime_error(
                "index out of range for the Control Point");
        }
        controlPoints[index] = point;
        controlPointsShapes[index].setPosition(point);
        texts[index].setPosition(point + 10.0F);
        updateCurve();
    }

    auto BezierCurve::setControlPoints(std::vector<sf::Vector2f> points)
        -> void {
        clear();
        controlPoints = points;

        controlPointsShapes.reserve(points.size());
        texts.reserve(points.size());

        for (int i = 0; i < controlPoints.size(); i++) {
            controlPointsShapes.emplace_back(createControlPoint(
                points[i], controlPointColor, controlPointRadius));
            texts.emplace_back(createControlText(std::to_string(i), points[i]));
        }
        updateCurve();
    }

    void BezierCurve::setControlPointColor(sf::Color color) {
        controlPointColor = color;

        for (auto& point : controlPointsShapes) {
            point.setFillColor(color);
        }
    }

    auto BezierCurve::setControlPointRadius(float radius) -> void {
        this->controlPointRadius = radius;

        for (auto& point : controlPointsShapes) {
            point.setRadius(radius);
        }
    }

    auto BezierCurve::setStep(double step) -> void {
        this->step = step;
        updateCurve();
    }

    void BezierCurve::setPrimitiveType(sf::PrimitiveType type) {
        curve.setPrimitiveType(type);
    }

    void BezierCurve::setCurveColor(sf::Color color) {
        curveColor = color;
    }

    auto BezierCurve::setTextColor(sf::Color color) -> void {
        textColor = color;

        for (auto& text : texts) {
            text.setFillColor(color);
        }
    }

    auto BezierCurve::setTextFont(const std::string& filename) -> void {
        auto flag = font.loadFromFile(filename);
        if (!flag) {
            throw std::runtime_error("unable to load font");
        }
    }

    void BezierCurve::resize(std::size_t controlPointCount) {
        controlPoints.resize(controlPointCount);
        controlPointsShapes.resize(controlPointCount);
        texts.resize(controlPointCount);
        updateCurve();
    }

    void BezierCurve::clear() {
        controlPoints.clear();
        controlPointsShapes.clear();
        curve.clear();
        texts.clear();
    }

    void BezierCurve::append(const sf::Vector2f point) {
        controlPoints.push_back(point);
        controlPointsShapes.emplace_back(
            createControlPoint(point, controlPointColor, controlPointRadius));
        texts.emplace_back(
            createControlText(std::to_string(controlPoints.size() - 1), point));
        updateCurve();
    }

    auto BezierCurve::operator[](std::size_t index) const
        -> const sf::Vector2f& {
        return controlPoints[index];
    }

    auto BezierCurve::createControlPoint(const sf::Vector2f& pos,
                                         sf::Color color, float radius)
        -> sf::CircleShape {
        auto circle = sf::CircleShape(radius);
        circle.setFillColor(color);
        circle.setOrigin(radius, radius);
        circle.setPosition(pos);
        return circle;
    }

    auto BezierCurve::createControlText(const std::string& str,
                                        sf::Vector2f       pos) -> sf::Text {
        sf::Text text;
        text.setFont(font);
        text.setFillColor(textColor);
        text.setString(str);
        text.setCharacterSize(20);
        text.setPosition(pos + 10.0F);
        return text;
    }

    auto BezierCurve::updateCurve() -> void {
        curve.clear();
        dotlines.clear();
        lines.clear();
        int delta = static_cast<int>(1.0 / step);
        for (int k = 0; k <= delta; ++k) {
            double i    = static_cast<double>(k) * step;
            i           = std::min(i, 1.0);  // clamp to avoid o
            auto vertex = getCurvePoints(controlPoints, i, lines)[0];
            curve.append(sf::Vertex(vertex, curveColor));
        }
        for (const auto& point : controlPoints) {
            dotlines.append(sf::Vertex(point, controlPointColor));
        }
        // fmt::print("Control Points: {}\n", controlPoints.size());
        // fmt::print("curve Points: {}\n", curve.getVertexCount());
        // fmt::print("Lines Points: {}\n", lines.getVertexCount());
    }

    auto BezierCurve::enableDotLines(bool enabled) -> void {
        showDotLines_ = enabled;
    }

    auto BezierCurve::enableLines(bool enabled) -> void {
        showLines_ = enabled;
    }

    auto BezierCurve::enableControlPoints(bool enabled) -> void {
        showControlPoints_ = enabled;
    }

    auto BezierCurve::getControlPoint(size_t index) -> sf::Vector2f {
        return controlPoints[index];
    }

    auto BezierCurve::getControlPoints() -> std::vector<sf::Vector2f> {
        return controlPoints;
    }

    auto BezierCurve::getCurvePoints(std::vector<sf::Vector2f>& pointArray,
                                     float t, sf::VertexArray& lines)
        -> std::vector<sf::Vector2f> {
        static std::uniform_int_distribution<int> distColor(0, 255);
        static std::mt19937                       generator(100);
        static int                                delta = -1;
        int  step    = static_cast<int>(t * 25);
        bool changed = false;
        if (delta != step) {
            delta   = step;
            changed = true;
            fmt::print("delta: {}\n", delta);
        }
        if (pointArray.size() == 1) {
            return pointArray;
        }

        auto result = std::vector<sf::Vector2f>();

        for (size_t i = 0; i < pointArray.size() - 1; i++) {
            auto pointA = pointArray[i];
            auto pointB = pointArray[i + 1];

            auto lerp = utils::lerp(pointA, pointB, t);
            if (changed) {
                sf::Color color(distColor(generator), distColor(generator),
                                distColor(generator));
                lines.append(sf::Vertex(lerp, color));
            }

            result.emplace_back(lerp);
        }

        return getCurvePoints(result, t, lines);
    }

    void BezierCurve::handleEvents(const sf::Event&  event,
                                   sf::RenderWindow& window) {
        if (!showControlPoints_) {
            return;
        }
        static constexpr float hitRadius = 20.F;
        static BezierCurve*    object;
        static int             draggingIndex =
            -1;  // which point is being dragged (-1 = none)

        sf::Vector2i mousePixel = sf::Mouse::getPosition(window);
        sf::Vector2f mouseWorld = window.mapPixelToCoords(mousePixel);

        if (event.type == sf::Event::MouseButtonPressed &&
            event.mouseButton.button == sf::Mouse::Left) {
            // Check all control points
            for (size_t i = 0; i < controlPointsShapes.size(); ++i) {
                auto& shape = controlPointsShapes[i];
                if (utils::distance(shape.getPosition(), mouseWorld) <
                    hitRadius) {
                    draggingIndex = static_cast<int>(i);
                    object        = this;
                    break;
                }
            }
        }

        if (event.type == sf::Event::MouseButtonReleased &&
            event.mouseButton.button == sf::Mouse::Left) {
            draggingIndex = -1;  // stop dragging
        }

        if (draggingIndex != -1 && event.type == sf::Event::MouseMoved &&
            object == this) {
            setControlPoint(draggingIndex, mouseWorld);
        }
    }

    void BezierCurve::draw(sf::RenderTarget& target,
                           sf::RenderStates  states) const {
        target.draw(curve, states);

        if (showControlPoints_) {
            for (int i = 0; i < controlPointsShapes.size(); i++) {
                target.draw(controlPointsShapes[i], states);
                target.draw(texts[i], states);
            }
        }
        if (showDotLines_) {
            target.draw(dotlines, states);
        }
        if (showLines_) {
            target.draw(lines, states);
        }
    }
}  // namespace Drawables
