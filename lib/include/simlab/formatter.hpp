
#include <fmt/core.h>
#include <fmt/format.h>

#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>

template <typename T>
struct fmt::formatter<sf::Vector2<T>> {
    constexpr auto parse(fmt::format_parse_context& ctx)
        -> decltype(ctx.begin()) {
        return ctx.end();
    }

    template <typename FormatContext>
    auto format(const sf::Vector2<T>& v, FormatContext& ctx)
        -> decltype(ctx.out()) {
        return fmt::format_to(ctx.out(), "({}, {})", v.x, v.y);
    }
};

template <>
struct fmt::formatter<sf::Color> {
    static constexpr auto parse(format_parse_context& ctx)
        -> decltype(ctx.begin()) {
        return ctx.end();
    }

    template <typename FormatContext>
    auto format(const sf::Color& c, FormatContext& ctx) -> decltype(ctx.out()) {
        return fmt::format_to(ctx.out(), "RGBA({}, {}, {}, {})", c.r, c.g, c.b,
                              c.a);
    }
};
