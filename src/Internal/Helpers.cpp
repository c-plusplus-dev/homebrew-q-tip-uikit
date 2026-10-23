//
// Created by Ayaan on 2026-09-14.
//

#include "QTipUIKit/Internal/Helpers.h"

#include <iostream>
#include <magic_enum/magic_enum.hpp>
#include <Q-Tip/Window/Window.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_render.h>

namespace Detail {
    SDL_Event transformEvent(const SDL_Event& event, const QTip::Point& position) {
        SDL_Event transformed = event;

        switch (transformed.type) {

            // Mouse
        case SDL_EVENT_MOUSE_MOTION:
            transformed.motion.x -= position.x;
            transformed.motion.y -= position.y;
            break;

        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP:
            transformed.button.x -= position.x;
            transformed.button.y -= position.y;
            break;

        case SDL_EVENT_MOUSE_WHEEL:
            transformed.wheel.mouse_x -= position.x;
            transformed.wheel.mouse_y -= position.y;
            break;


            // Touch
        case SDL_EVENT_FINGER_DOWN:
        case SDL_EVENT_FINGER_UP:
        case SDL_EVENT_FINGER_MOTION:
        case SDL_EVENT_FINGER_CANCELED:
            // Touch coordinates are normalized [0, 1], so these
            // should NOT simply have a pixel position subtracted.
            //
            // They need to be converted into the coordinate space
            // of the UI object separately.
            break;


            // Drag and drop
        case SDL_EVENT_DROP_POSITION:
            transformed.drop.x -= position.x;
            transformed.drop.y -= position.y;
            break;


            // Pinch gestures
        case SDL_EVENT_PINCH_BEGIN:
        case SDL_EVENT_PINCH_UPDATE:
        case SDL_EVENT_PINCH_END:
            // No ordinary window-space x/y position to transform.
            break;


        default:
            break;
        }

        return transformed;
    }

    QTip::Point eventPosition(const SDL_Event& event) {
        switch (event.type) {
        case SDL_EVENT_MOUSE_MOTION:
            return {event.motion.x, event.motion.y};

        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            return {event.button.x, event.button.y};

        case SDL_EVENT_MOUSE_WHEEL:
            return {event.wheel.mouse_x, event.wheel.mouse_y};

        default:
            return {-1, -1};
        }
    }

    void printMouseEvent(const SDL_Event& event) {
        switch (event.type) {
        case SDL_EVENT_MOUSE_MOTION:
            std::cout
                << "MOUSE_MOTION"
                << " window=" << event.motion.windowID
                << " position=(" << event.motion.x << ", " << event.motion.y << ")"
                << " delta=(" << event.motion.xrel << ", " << event.motion.yrel << ")"
                << " buttons=" << event.motion.state
                << '\n';
            break;

        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            std::cout
                << "MOUSE_BUTTON_DOWN"
                << " window=" << event.button.windowID
                << " position=(" << event.button.x << ", " << event.button.y << ")"
                << " button=" << static_cast<int>(event.button.button)
                << " clicks=" << static_cast<int>(event.button.clicks)
                << '\n';
            break;

        case SDL_EVENT_MOUSE_BUTTON_UP:
            std::cout
                << "MOUSE_BUTTON_UP"
                << " window=" << event.button.windowID
                << " position=(" << event.button.x << ", " << event.button.y << ")"
                << " button=" << static_cast<int>(event.button.button)
                << " clicks=" << static_cast<int>(event.button.clicks)
                << '\n';
            break;

        case SDL_EVENT_MOUSE_WHEEL:
            std::cout
                << "MOUSE_WHEEL"
                << " window=" << event.wheel.windowID
                << " position=(" << event.wheel.mouse_x << ", " << event.wheel.mouse_y << ")"
                << " scroll=(" << event.wheel.x << ", " << event.wheel.y << ")"
                << '\n';
            break;

        default:
            break;
        }
    }

    SDL_FColor toFColor(QTip::Color color) {
        return SDL_FColor {
            static_cast<float>(color.r) / 255.0f,
            static_cast<float>(color.g) / 255.0f,
            static_cast<float>(color.b) / 255.0f,
            static_cast<float>(color.a) / 255.0f
        };
    }

    std::string eventName(const SDL_Event& event) {
        char buffer[256]{};
        SDL_GetEventDescription(&event, buffer, sizeof(buffer));
        return buffer;
    }

    bool pointInTriangle(float px, float py, const QTip::Point& v0, const QTip::Point& v1, const QTip::Point& v2) {
        float x0 = v0.x, y0 = v0.y;
        float x1 = v1.x, y1 = v1.y;
        float x2 = v2.x, y2 = v2.y;

        float dX = px - x2;
        float dY = py - y2;
        float dX21 = x2 - x1;
        float dY12 = y1 - y2;
        float D = dY12*(x0 - x2) + dX21*(y0 - y2);
        float s = dY12*dX + dX21*dY;
        float t = (y2 - y0)*dX + (x0 - x2)*dY;

        if (D < 0) return (s <= 0) && (t <= 0) && (s + t >= D);
        return (s >= 0) && (t >= 0) && (s + t <= D);
    }

    bool pointInVB(float px, float py, const std::vector<QTip::Point>& vb) {
        if (vb.size() < 3) return false; // need at least one triangle

        // Triangle fan: first vertex is center
        for (size_t i = 1; i < vb.size() - 1; ++i) {
            if (pointInTriangle(px, py, vb[0], vb[i], vb[i + 1])) {
                return true;
            }
        }
        return false;
    }

    double drawPieSlice(QTip::Window& window, std::vector<QTip::Point>* vb, float cx, float cy, float radius, double percent, QTip::Color color, double startAngle) {
        if (percent <= 0.0) return startAngle;
        if (percent > 1.0) percent = 1.0;  // clamp

        constexpr int segments = 30;  // increase for smoother slices
        double endAngle = startAngle + 2.0 * M_PI * percent;

        std::vector<QTip::Point> vertices;
        std::vector<int> indices;

        // Center point
        QTip::Point center{};
        center.x = cx;
        center.y = cy;
        vertices.push_back(center);

        // Arc points
        for (int i = 0; i <= segments; ++i) {
            double theta = startAngle + (endAngle - startAngle) * i / segments;
            QTip::Point v{};
            v.x = static_cast<float>(cx + radius * cos(theta));
            v.y = static_cast<float>(cy + radius * sin(theta));
            vertices.push_back(v);
        }

        // Triangle fan indices
        for (int i = 1; i < static_cast<int>(vertices.size()) - 1; ++i) {
            indices.push_back(0);
            indices.push_back(i);
            indices.push_back(i + 1);
        }

        window->setRenderColor(color);
        window->renderPolygon(QTip::Polygon{vertices});

        if (vb)
            *vb = vertices;

        return endAngle;  // return the angle where the next slice should start
    }
}
