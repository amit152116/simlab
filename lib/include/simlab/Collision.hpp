#include <SFML/Graphics.hpp>

#include "simlab/utils.hpp"

#include <algorithm>
#include <cmath>
#include <vector>

namespace simlab {

    struct CollisionInfo {
        bool  collided{};
        float magnitude{};    // Minimum Translation Vector (direction & depth)
        sf::Vector2f point;   // Approximate collision point
        sf::Vector2f normal;  // Approximate collision normal
        float        penetration{};
    };

    class Collision {
      public:

        Collision()                                    = delete;
        Collision(const Collision&)                    = delete;
        Collision(Collision&&)                         = delete;
        auto operator=(const Collision&) -> Collision& = delete;
        auto operator=(Collision&&) -> Collision&      = delete;
        ~Collision()                                   = delete;

        // Calculate collision between two circles
        static auto circleCollision(const sf::CircleShape& circle1,
                                    const sf::CircleShape& circle2)
            -> CollisionInfo {
            CollisionInfo result;
            result.collided = false;

            // Get positions and radii
            sf::Vector2f pos1    = circle1.getPosition();
            sf::Vector2f pos2    = circle2.getPosition();
            float        radius1 = circle1.getRadius();
            float        radius2 = circle2.getRadius();

            // Calculate distance vector from circle1 to circle2
            sf::Vector2f distanceVec = pos2 - pos1;
            float        distance    = sqrt((distanceVec.x * distanceVec.x) +
                                            (distanceVec.y * distanceVec.y));

            // Sum of radii
            float radiusSum = radius1 + radius2;

            // Check if circles are colliding
            if (distance <= radiusSum &&
                distance > 0.001F) {  // Avoid division by zero
                result.collided = true;

                // Calculate penetration depth
                result.penetration = radiusSum - distance;

                // Calculate collision normal (normalized direction from circle1
                // to circle2)
                result.normal = distanceVec / distance;

                // Calculate contact point (on the line connecting centers)
                // result.contactPoint = pos1 + result.normal * radius1;
            }

            return result;
        }

        static auto windowCollision(const sf::CircleShape&  circle,
                                    const sf::RenderWindow& window)
            -> CollisionInfo {
            CollisionInfo result;
            float         radius = circle.getRadius();
            sf::Vector2f  pos    = circle.getPosition();
            float         left   = pos.x - radius;
            float         right  = pos.x + radius;
            float         top    = pos.y - radius;
            float         bottom = pos.y + radius;
            float         winW   = static_cast<float>(window.getSize().x);
            float         winH   = static_cast<float>(window.getSize().y);

            sf::Vector2f normal(0.F, 0.F);
            float        penetration = 0.F;

            if (left <= 0.F) {
                result.collided = true;
                normal.x        = 1.F;  // Bounce right
                result.point    = {0.F, pos.y};
                penetration     = -left;  // How far past the boundary
            }
            if (right >= winW) {
                result.collided = true;
                normal.x        = -1.F;  // Bounce left
                result.point    = {winW, pos.y};
                penetration     = right - winW;
            }
            if (top <= 0.F) {
                result.collided = true;
                normal.y        = 1.F;  // Bounce down
                result.point    = {pos.x, 0.F};
                penetration     = -top;
            }
            if (bottom >= winH) {
                result.collided = true;
                normal.y        = -1.F;  // Bounce up
                result.point    = {pos.x, winH};
                penetration     = bottom - winH;
            }

            result.normal      = normal;
            result.penetration = penetration;
            return result;
        }

        // General collision check
        static auto shapeCollision(const sf::Shape& s1, const sf::Shape& s2)
            -> CollisionInfo {
            auto poly1 = getGlobalPoints(s1);
            auto poly2 = getGlobalPoints(s2);
            return polygonsIntersect(poly1, poly2);
        }

        // Helper: check polygon overlap on all axes
        static auto polygonsIntersect(const std::vector<sf::Vector2f>& poly1,
                                      const std::vector<sf::Vector2f>& poly2)
            -> CollisionInfo {
            float        minOverlap = std::numeric_limits<float>::max();
            sf::Vector2f smallestAxis;

            auto checkAxes =
                [&](const std::vector<sf::Vector2f>& polyA,
                    const std::vector<sf::Vector2f>& polyB) -> bool {
                for (size_t i = 0; i < polyA.size(); i++) {
                    sf::Vector2f p1 = polyA[i];
                    sf::Vector2f p2 = polyA[(i + 1) % polyA.size()];

                    // Edge vector
                    sf::Vector2f edge = {p2.x - p1.x, p2.y - p1.y};
                    // Perpendicular axis
                    sf::Vector2f axis = {-edge.y, edge.x};

                    // Normalize
                    float len =
                        std::sqrt((axis.x * axis.x) + (axis.y * axis.y));
                    if (len == 0) {
                        continue;
                    }
                    axis.x /= len;
                    axis.y /= len;

                    float minA, maxA, minB, maxB;
                    projectPolygon(polyA, axis, minA, maxA);
                    projectPolygon(polyB, axis, minB, maxB);

                    if (maxA < minB || maxB < minA) {
                        return false;  // Gap found
                    }

                    // Overlap along this axis
                    float overlap = std::min(maxA, maxB) - std::max(minA, minB);
                    if (overlap < minOverlap) {
                        minOverlap   = overlap;
                        smallestAxis = axis;
                    }
                }
                return true;
            };

            if (!checkAxes(poly1, poly2) || !checkAxes(poly2, poly1)) {
                return {false, 0, {0, 0}, {0, 0}};  // no collision
            }

            // Approximate collision point: use centroid of poly1 projected onto
            // smallestAxis
            sf::Vector2f centroid1{0, 0};
            for (const auto& point : poly1) {
                centroid1 += point;
            }
            centroid1.x /= poly1.size();
            centroid1.y /= poly1.size();

            // Push out along MTV direction
            sf::Vector2f mtv = smallestAxis * minOverlap;

            // Compute collision normal (normalize MTV)
            auto normal = normalize(mtv);

            return {true, magnitude(mtv), centroid1, normal};
        }

      private:

        // Helper: project a polygon onto an axis
        static void projectPolygon(const std::vector<sf::Vector2f>& points,
                                   const sf::Vector2f& axis, float& min,
                                   float& max) {
            if (points.empty()) {
                return;  // Safety check
            }
            min = max = ((points[0].x * axis.x) + (points[0].y * axis.y));
            // Start from index 1 since we already processed points[0]
            for (size_t i = 1; i < points.size(); ++i) {
                float projection =
                    ((points[i].x * axis.x) + (points[i].y * axis.y));
                min = std::min(projection, min);
                max = std::max(projection, max);
            }
        }
    };
}  // namespace simlab
