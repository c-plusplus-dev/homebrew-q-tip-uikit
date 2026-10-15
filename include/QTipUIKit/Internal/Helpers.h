//
// Created by Ayaan on 2026-09-11.
//

#ifndef QTIPUIKIT_HELPERS_H
#define QTIPUIKIT_HELPERS_H
#include <filesystem>
#include <Q-Tip/Graphics/Color.h>
#include <Q-Tip/Math/Point.h>
#include <SDL3/SDL_events.h>
namespace fs = std::filesystem;

union SDL_Event;
struct SDL_FColor;

namespace Detail {
    inline fs::path defaultFontPath() {
#ifdef _WIN32
        return "C:/Windows/Fonts/Arial.ttf";
#elif defined(__linux__)
        return "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf";
#elif defined(__APPLE__)
        return "/System/Library/Fonts/SFNS.ttf";
#else
#error "Unsupported platform"
#endif
    }

    SDL_Event transformEvent(const SDL_Event& event, const QTip::Point& position);

    QTip::Point eventPosition(const SDL_Event& event);

    void printMouseEvent(const SDL_Event& event);

    inline QTip::Color pieChartColor(double position) {
        position = std::clamp(position, 0.0, 1.0);

        const double hue = position * 6.0;
        const int section = static_cast<int>(hue);
        const double fraction = hue - section;

        double r = 0.0;
        double g = 0.0;
        double b = 0.0;

        switch (section) {
        case 0:
            r = 1.0;
            g = fraction;
            b = 0.0;
            break;

        case 1:
            r = 1.0 - fraction;
            g = 1.0;
            b = 0.0;
            break;

        case 2:
            r = 0.0;
            g = 1.0;
            b = fraction;
            break;

        case 3:
            r = 0.0;
            g = 1.0 - fraction;
            b = 1.0;
            break;

        case 4:
            r = fraction;
            g = 0.0;
            b = 1.0;
            break;

        case 5:
        default:
            r = 1.0;
            g = 0.0;
            b = 1.0 - fraction;
            break;
        }

        return QTip::Color {
            static_cast<std::uint8_t>(r * 255.0),
            static_cast<std::uint8_t>(g * 255.0),
            static_cast<std::uint8_t>(b * 255.0),
            255
        };
    }

    SDL_FColor toFColor(QTip::Color color);

    std::string eventName(const SDL_Event& event);
}

static bool operator<(const QTip::Point& lhs, const QTip::Point& rhs) {
    return lhs.x < rhs.x && lhs.y < rhs.y;
}

static bool operator>(const QTip::Point& lhs, const QTip::Point& rhs) {
    return lhs.x > rhs.x && lhs.y > rhs.y;
}

#endif //QTIPUIKIT_HELPERS_H
