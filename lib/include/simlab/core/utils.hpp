#pragma once
#include <fmt/core.h>

#include <SFML/Graphics.hpp>

#include <algorithm>
#include <cmath>
#include <vector>

constexpr double DEG_TO_RAD = M_PI / 180.0;
constexpr double RAD_TO_DEG = 180.0 / M_PI;

template <typename T>
auto operator+(const sf::Vector2<T>& v, T scalar) -> sf::Vector2<T> {
    return {v.x + scalar, v.y + scalar};
}

template <typename T>
auto operator*(const sf::Vector2<T>& a, const sf::Vector2<T>& b)
    -> sf::Vector2<T> {
    return {a.x * b.x, a.y * b.y};
}

template <typename T>
auto operator/(const sf::Vector2<T>& a, const sf::Vector2<T>& b)
    -> sf::Vector2<T> {
    return {a.x / b.x, a.y / b.y};
}

template <typename T>
auto operator-(const sf::Vector2<T>& v, T scalar) -> sf::Vector2<T> {
    return {v.x - scalar, v.y - scalar};
}

template <typename T>
struct Vector2Hash {
    auto operator()(const sf::Vector2<T>& v) const noexcept -> std::size_t {
        return std::hash<T>()(v.x) ^ (std::hash<T>()(v.y) << 1);
    }
};

namespace utils {

    inline auto toVector2i(const sf::Vector2f& v) -> sf::Vector2i {
        return {static_cast<int>(std::floor(v.x)),
                static_cast<int>(std::floor(v.y))};
    }

    // ✅ Clamp a shape’s position inside the window
    inline void clampToWindow(sf::Shape& shape, const sf::RenderWindow& window,
                              sf::Vector2f padding = {0, 0}) {
        static const auto WINDOW_WIDTH  = window.getSize().x;
        static const auto WINDOW_HEIGHT = window.getSize().y;

        auto       origin      = shape.getOrigin();
        auto       shapePos    = shape.getPosition();
        const auto shapeBounds = shape.getGlobalBounds();

        shapePos.x = std::clamp(shapePos.x, origin.x + padding.x,
                                static_cast<float>(WINDOW_WIDTH) -
                                    shapeBounds.width + origin.x - padding.x);

        shapePos.y = std::clamp(shapePos.y, origin.y + padding.y,
                                static_cast<float>(WINDOW_HEIGHT) -
                                    shapeBounds.height + origin.y - padding.y);
        shape.setPosition(shapePos);
    }

    // ✅ Center origin (useful for rotation/scale)
    inline void centerOrigin(sf::Transformable& obj) {
        if (auto* drawable = dynamic_cast<sf::Drawable*>(&obj)) {
            if (auto* shape = dynamic_cast<sf::Shape*>(drawable)) {
                auto bounds = shape->getLocalBounds();
                obj.setOrigin(bounds.width / 2.F, bounds.height / 2.F);
            } else if (auto* sprite = dynamic_cast<sf::Sprite*>(drawable)) {
                auto bounds = sprite->getLocalBounds();
                obj.setOrigin(bounds.width / 2.F, bounds.height / 2.F);
            } else if (auto* text = dynamic_cast<sf::Text*>(drawable)) {
                auto bounds = text->getLocalBounds();
                obj.setOrigin(bounds.width / 2.F, bounds.height / 2.F);
            }
        }
    }

    // ✅ Get global points of any shape
    inline auto getGlobalPoints(const sf::Shape& shape)
        -> std::vector<sf::Vector2f> {
        std::vector<sf::Vector2f> pts;
        pts.reserve(shape.getPointCount());
        for (size_t i = 0; i < shape.getPointCount(); i++) {
            pts.push_back(
                shape.getTransform().transformPoint(shape.getPoint(i)));
        }
        return pts;
    }

    // ✅ Distance between two points
    template <typename T>
    inline auto distance(const sf::Vector2<T>& a, const sf::Vector2<T>& b)
        -> T {
        float dx = a.x - b.x;
        float dy = a.y - b.y;
        return static_cast<T>(std::sqrt((dx * dx) + (dy * dy)));
    }

    template <typename T>
    inline auto distanceSquared(const sf::Vector2<T>& a,
                                const sf::Vector2<T>& b) -> T {
        float dx = a.x - b.x;
        float dy = a.y - b.y;
        return (dx * dx) + (dy * dy);
    }

    // ✅ Magnitude of a vector
    template <typename T>
    inline auto magnitude(const sf::Vector2<T>& v) -> T {
        return static_cast<T>(std::sqrt((v.x * v.x) + (v.y * v.y)));
    }

    template <typename T>
    inline auto magnitudeSquared(const sf::Vector2<T>& v) -> T {
        return (v.x * v.x) + (v.y * v.y);
    }

    // ✅ Normalize a vector
    template <typename T>
    inline auto normalize(const sf::Vector2<T>& v) -> sf::Vector2<T> {
        auto len = magnitude(v);
        return (len != 0.F) ? sf::Vector2<T>(v.x / len, v.y / len)
                            : sf::Vector2<T>(0.F, 0.F);
    }

    // Dot product
    template <typename T>
    inline auto dotProduct(const sf::Vector2<T>& a, const sf::Vector2<T>& b)
        -> T {
        return (a.x * b.x) + (a.y * b.y);
    }

    // Cross product (2D) → returns scalar (z-component)
    template <typename T>
    inline auto crossProduct(const sf::Vector2<T>& a, const sf::Vector2<T>& b)
        -> T {
        return static_cast<T>((a.x * b.y) - (a.y * b.x));
    }

    // Cross product (3D) → returns vector (useful if you extend with
    // sf::Vector3f)
    template <typename T>
    inline auto crossProduct(const sf::Vector3<T>& a, const sf::Vector3<T>& b)
        -> sf::Vector3<T> {
        return {(a.y * b.z) - (a.z * b.y), (a.z * b.x) - (a.x * b.z),
                (a.x * b.y) - (a.y * b.x)};
    }

    // Projection of vector a onto b
    template <typename T>
    inline auto projection(const sf::Vector2<T>& vector,
                           const sf::Vector2<T>& normal) -> sf::Vector2<T> {
        auto bn = normalize(normal);
        return dotProduct(vector, bn) * bn;
    }

    // Angle of a vector a onto b
    template <typename T>
    inline auto angle(const sf::Vector2<T>& vector,
                      const sf::Vector2<T>& normal) -> float {
        auto  dot  = dotProduct(vector, normal);
        float magA = magnitude(vector);
        float magB = magnitude(normal);
        if (magA == 0.F || magB == 0.F) {
            return 0.F;  // avoid divide by 0
        }

        auto cosTheta = dot / (magA * magB);

        // Clamp to [-1, 1] to avoid floating-point errors outside acos range
        cosTheta = std::clamp(cosTheta, -1.F, 1.F);
        return std::acos(cosTheta);
    }

    // Perpendicular (normal) of vector
    template <typename T>
    inline auto normalVector(const sf::Vector2<T>& v) -> sf::Vector2<T> {
        return {-v.y, v.x};
    }

    // ✅ Rotate a vector by degrees
    template <typename T>
    inline auto rotate(const sf::Vector2<T>& v, float degrees)
        -> sf::Vector2<T> {
        float rad = degrees * M_PI / 180.F;
        float cs  = std::cos(rad);
        float sn  = std::sin(rad);
        return {(v.x * cs) - (v.y * sn), (v.x * sn) + (v.y * cs)};
    }

    // Reflect velocity using normal: v' = v - 2*(v . n)*n
    template <typename T>
    inline auto reflect(const sf::Vector2<T>& vector,
                        const sf::Vector2<T>& normal) -> sf::Vector2<T> {
        return vector - 2.F * projection(vector, normal);
    }

    // Apply arbitrary affine transform matrix
    template <typename T>
    inline auto transformPoint(const sf::Transform& t, const sf::Vector2<T>& p)
        -> sf::Vector2<T> {
        return t.transformPoint(p);
    }

    // ✅ Convert mouse position to world coordinates
    inline auto mouseWorldPos(const sf::RenderWindow& window) -> sf::Vector2f {
        return window.mapPixelToCoords(sf::Mouse::getPosition(window));
    }

    // ✅ Check if a point is inside a shape
    template <typename T>
    inline auto contains(const sf::Shape& shape, const sf::Vector2<T>& point)
        -> bool {
        return shape.getGlobalBounds().contains(point);
    }

    // ✅ Linearly interpolate between two vectors
    template <typename T>
    inline auto lerp(const sf::Vector2<T>& a, const sf::Vector2<T>& b, float t)
        -> sf::Vector2<T> {
        if (t < 0.F || t > 1.F) {
            throw std::runtime_error(fmt::format(
                "wrong value: {}\nlerp: t must be between 0 and 1", t));
        }
        return (1 - t) * a + b * t;
    }

    inline auto GenerateTriangle(sf::Vector2f center, float size,
                                 sf::Color color) -> sf::VertexArray {
        sf::VertexArray triangle(sf::PrimitiveType::Triangles);
        sf::Vertex      v1;
        sf::Vertex      v2;
        sf::Vertex      v3;

        v1.position = {center.x, center.y - size};
        v2.position = {center.x - size, center.y + size};
        v3.position = {center.x + size, center.y + size};

        v1.color = v2.color = v3.color = color;
        return triangle;
    }

    inline auto GenerateCircle(sf::Vector2f center, float radius,
                               sf::Color color) -> sf::VertexArray {
        const int       segments = 12;
        sf::VertexArray circle(sf::PrimitiveType::TriangleFan);

        // center of the fan
        sf::Vertex centerVertex(center, color);
        circle.append(centerVertex);

        for (int i = 0; i <= segments; i++) {
            float        angle = i * 2.F * M_PI / segments;
            sf::Vector2f pos{center.x + (radius * std::cos(angle)),
                             center.y + (radius * std::sin(angle))};
            circle.append(sf::Vertex(pos, color));
        }

        return circle;
    }

    inline auto GenerateRectangle(sf::Vector2f center, sf::Vector2f size,
                                  sf::Color color) -> sf::VertexArray {
        // Half sizes
        sf::VertexArray rectangle(sf::PrimitiveType::Triangles);
        float           hw = size.x * 0.5F;
        float           hh = size.y * 0.5F;

        // 4 corners
        sf::Vector2f topLeft(center.x - hw, center.y - hh);
        sf::Vector2f topRight(center.x + hw, center.y - hh);
        sf::Vector2f bottomLeft(center.x - hw, center.y + hh);
        sf::Vector2f bottomRight(center.x + hw, center.y + hh);

        // Two triangles → (topLeft, bottomLeft, topRight) and (topRight,
        // bottomLeft, bottomRight)
        rectangle.append(sf::Vertex(topLeft, color));
        rectangle.append(sf::Vertex(bottomLeft, color));
        rectangle.append(sf::Vertex(topRight, color));

        rectangle.append(sf::Vertex(topRight, color));
        rectangle.append(sf::Vertex(bottomLeft, color));
        rectangle.append(sf::Vertex(bottomRight, color));
        return rectangle;
    }

    inline auto HSLtoRGB(float h, float s, float l) -> sf::Color {
        // h in [0, 360), s and l in [0, 1]
        float C = (1 - std::fabs((2 * l) - 1)) * s;
        float X = C * (1 - std::fabs(fmod(h / 60.0F, 2) - 1));
        float m = l - (C / 2);

        float r = 0;
        float g = 0;
        float b = 0;

        if (h < 60) {
            r = C;
            g = X;
        } else if (h < 120) {
            r = X;
            g = C;
        } else if (h < 180) {
            g = C;
            b = X;
        } else if (h < 240) {
            g = X;
            b = C;
        } else if (h < 300) {
            r = X;
            b = C;
        } else {
            r = C;
            b = X;
        }

        return {static_cast<sf::Uint8>((r + m) * 255),
                static_cast<sf::Uint8>((g + m) * 255),
                static_cast<sf::Uint8>((b + m) * 255)};
    }

    inline void DrawGrid(sf::RenderTarget& window, float cellSize,
                         sf::Color color = sf::Color(150, 150, 150)) {
        sf::Vector2u winSize = window.getSize();

        // Create a vertex array for lines
        sf::VertexArray lines(sf::Lines);

        // Vertical lines
        for (float x = 0; x <= winSize.x; x += cellSize) {
            lines.append(sf::Vertex(sf::Vector2f(x, 0), color));
            lines.append(sf::Vertex(
                sf::Vector2f(x, static_cast<float>(winSize.y)), color));
        }

        // Horizontal lines
        for (float y = 0; y <= winSize.y; y += cellSize) {
            lines.append(sf::Vertex(sf::Vector2f(0, y), color));
            lines.append(sf::Vertex(
                sf::Vector2f(static_cast<float>(winSize.x), y), color));
        }

        // Draw all lines in one call
        window.draw(lines);
    }
}  // namespace utils
