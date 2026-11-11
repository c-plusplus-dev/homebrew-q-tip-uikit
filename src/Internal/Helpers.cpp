//
// Created by Ayaan on 2026-09-14.
//

#include <QTipUIKit/Internal/Helpers.h>

#include <cmath>
#include <iostream>
#include <numbers>
#include <string>
#include <vector>

#include <magic_enum/magic_enum.hpp>

#include <Q-Tip/Window/Window.h>
#include <Q-Tip/Events/Event.h>

namespace Detail {

     QTip::Event transformEvent(const QTip::Event& event, const QTip::Point& position) {
        QTip::Event transformed = event;

        switch (transformed.type) {

            // Mouse
            case QTip::EventType::MouseMoved:
                transformed.mouseMove.position -= position;
                break;

            case QTip::EventType::MouseButtonPressed:
            case QTip::EventType::MouseButtonReleased:
                transformed.mouseButton.position -= position;
                break;

            case QTip::EventType::MouseScrolled:
                transformed.mouseScroll.position -= position;

            case QTip::EventType::FileDropped:
                transformed.fileDrop.position -= position;

            default:
                break;
        }

        return transformed;
    }

     QTip::Point eventPosition(const QTip::Event& event) {
        switch (event.type) {

            case QTip::EventType::MouseMoved:
                return event.mouseMove.position;

            case QTip::EventType::MouseButtonPressed:
            case QTip::EventType::MouseButtonReleased:
                return event.mouseButton.position;

            case QTip::EventType::MouseScrolled:
                return event.mouseScroll.position;

            case QTip::EventType::FileDropped:
                return event.fileDrop.position;

            default:
                return {-1, -1};
        }
    }

     void printMouseEvent(const QTip::Event& event) {
        switch (event.type) {

            case QTip::EventType::MouseMoved:
                std::cout
                    << "MOUSE_MOVED"
                    << " window=" << event.windowID
                    << " position=("
                    << event.mouseMove.position.x << ", "
                    << event.mouseMove.position.y << ")"
                    << " delta=("
                    << event.mouseMove.delta.x << ", "
                    << event.mouseMove.delta.y << ")"
                    << " buttons=" << event.mouseMove.buttons
                    << '\n';
                break;

            case QTip::EventType::MouseButtonPressed:
            case QTip::EventType::MouseButtonReleased:
                std::cout
                    << (event.type == QTip::EventType::MouseButtonPressed
                        ? "MOUSE_BUTTON_PRESSED"
                        : "MOUSE_BUTTON_RELEASED")
                    << " window=" << event.windowID
                    << " position=("
                    << event.mouseButton.position.x << ", "
                    << event.mouseButton.position.y << ")"
                    << " button=" << static_cast<int>(event.mouseButton.button)
                    << " clicks=" << static_cast<int>(event.mouseButton.clicks)
                    << '\n';
                break;

            case QTip::EventType::MouseScrolled:
                std::cout
                    << "MOUSE_SCROLLED"
                    << " window=" << event.windowID
                    << " offset=("
                    << event.mouseScroll.offset.x << ", "
                    << event.mouseScroll.offset.y << ")"
                    << '\n';
                break;

            default:
                break;
        }
    }

     std::string eventName(const QTip::Event& event) {
        return std::string(magic_enum::enum_name(event.type));
    }

     bool pointInTriangle(
        float px,
        float py,
        const QTip::Point& v0,
        const QTip::Point& v1,
        const QTip::Point& v2
    ) {
        const float dX = px - v2.x;
        const float dY = py - v2.y;

        const float dX21 = v2.x - v1.x;
        const float dY12 = v1.y - v2.y;

        const float D = dY12 * (v0.x - v2.x) + dX21 * (v0.y - v2.y);

        const float s = dY12 * dX + dX21 * dY;
        const float t = (v2.y - v0.y) * dX + (v0.x - v2.x) * dY;

        if (D < 0)
            return s <= 0 && t <= 0 && s + t >= D;

        return s >= 0 && t >= 0 && s + t <= D;
    }

     bool pointInVB(
        float px,
        float py,
        const std::vector<QTip::Point>& vb
    ) {
        if (vb.size() < 3)
            return false;

        // Triangle fan: first vertex is center.
        for (std::size_t i = 1; i < vb.size() - 1; ++i) {
            if (pointInTriangle(px, py, vb[0], vb[i], vb[i + 1]))
                return true;
        }

        return false;
    }

     double drawPieSlice(
        QTip::Window& window,
        std::vector<QTip::Point>* vb,
        float cx,
        float cy,
        float radius,
        double percent,
        QTip::Color color,
        double startAngle
    ) {
        if (percent <= 0.0)
            return startAngle;

        percent = std::min(percent, 1.0);

        constexpr int SEGMENTS_PER_CIRCLE = 64;

        const int segments = std::max(
         1,
         static_cast<int>(std::ceil(percent * SEGMENTS_PER_CIRCLE))
        );

        const double endAngle =
            startAngle + 2.0 * std::numbers::pi_v<double> * percent;

        std::vector<QTip::Point> vertices;
        std::vector<int> indices;

        // Center point.
        vertices.push_back({cx, cy});

        // Arc points.
        for (int i = 0; i <= segments; ++i) {
            const double theta =
                startAngle + (endAngle - startAngle) * i / segments;

            vertices.push_back({
                static_cast<float>(cx + radius * std::cos(theta)),
                static_cast<float>(cy + radius * std::sin(theta))
            });
        }

        // Triangle fan indices.
        for (int i = 1; i < static_cast<int>(vertices.size()) - 1; ++i) {
            indices.push_back(0);
            indices.push_back(i);
            indices.push_back(i + 1);
        }

        window->setRenderColor(color);
        window->renderPolygon(QTip::Polygon{vertices});

        if (vb)
            *vb = vertices;

        return endAngle;
    }

}
