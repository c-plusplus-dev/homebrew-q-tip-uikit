//
// Created by Ayaan on 2026-09-28.
//

#include "QTipUIKit/Widgets/ScrollBar/ScrollBar.h"

#include <algorithm>

#include "QTipUIKit/Internal/Helpers.h"

namespace {

constexpr float ScrollbarThickness = 10.0f;
constexpr float MinimumThumbLength = 20.0f;

}

ScrollBar::ScrollBar(
    const QTip::Rect rect,
    const ScrollBarOrientation orientation
)
    : _orientation(orientation) {
    _rect = rect;
    _preferredRect = rect;
}

void ScrollBar::setPosition(const float position) {
    _position = std::clamp(position, 0.0f, 1.0f);
}

float ScrollBar::position() const {
    return _position;
}

void ScrollBar::setViewRatio(const float ratio) {
    _viewRatio = std::clamp(ratio, 0.0f, 1.0f);
}

float ScrollBar::viewRatio() const {
    return _viewRatio;
}

QTip::Rect ScrollBar::thumbRect() const {
    const float length = thumbLength();
    const float travel = std::max(
        0.0f,
        trackLength() - length
    );

    const float offset = _position * travel;

    if (_orientation == ScrollBarOrientation::Horizontal) {
        return {
            _rect.origin.x + offset,
            _rect.origin.y,
            length,
            ScrollbarThickness
        };
    }

    return {
        _rect.origin.x,
        _rect.origin.y + offset,
        ScrollbarThickness,
        length
    };
}

float ScrollBar::trackStart() const {
    if (_orientation == ScrollBarOrientation::Horizontal) {
        return _rect.origin.x;
    }

    return _rect.origin.y;
}

float ScrollBar::trackLength() const {
    if (_orientation == ScrollBarOrientation::Horizontal) {
        return _rect.size.x;
    }

    return _rect.size.y;
}

float ScrollBar::thumbLength() const {
    return std::clamp(
        trackLength() * _viewRatio,
        MinimumThumbLength,
        trackLength()
    );
}

void ScrollBar::renderImpl(QTip::Window& window) {
    const float trackLengthValue = trackLength();
    //const float thumbLengthValue = thumbLength();

    if (trackLengthValue <= 0.0f) {
        return;
    }

    const QTip::Point center = _rect.center();

    window->setRenderColor(QTip::Color{
        64,
        64,
        64,
        255
    });

    if (_orientation == ScrollBarOrientation::Horizontal) {
        window->renderThickLine(
            QTip::Line{
                {
                    _rect.origin.x,
                    center.y
                },
                {
                    _rect.origin.x + _rect.size.x,
                    center.y
                }
            },
            ScrollbarThickness,
            true
        );
    } else {
        window->renderThickLine(
            QTip::Line{
                {
                    center.x,
                    _rect.origin.y
                },
                {
                    center.x,
                    _rect.origin.y + _rect.size.y
                }
            },
            ScrollbarThickness,
            true
        );
    }

    const QTip::Rect thumb = thumbRect();

    window->setRenderColor(QTip::Color{
        127,
        127,
        127,
        255
    });

    if (_orientation == ScrollBarOrientation::Horizontal) {
        window->renderThickLine(
            QTip::Line{
                {
                    thumb.origin.x,
                    center.y
                },
                {
                    thumb.origin.x + thumb.size.x,
                    center.y
                }
            },
            ScrollbarThickness,
            true
        );
    } else {
        window->renderThickLine(
            QTip::Line{
                {
                    center.x,
                    thumb.origin.y
                },
                {
                    center.x,
                    thumb.origin.y + thumb.size.y
                }
            },
            ScrollbarThickness,
            true
        );
    }
}

bool ScrollBar::handleEvent(const QTip::Event& event) {
    switch (event.type) {
        case QTip::EventType::MouseButtonPressed: {
            const QTip::Point mouse{
                event.mouseButton.position.x,
                event.mouseButton.position.y
            };

            QTip::Rect thumb = thumbRect();
            thumb.origin -= QTip::Point{5, 5};
            thumb.size += QTip::Point{10, 10};

            if (!thumb.isPointInside(mouse)) {
                break;
            }

            _dragging = true;

            if (_orientation == ScrollBarOrientation::Horizontal) {
                _dragOffset = mouse.x - thumb.origin.x;
            } else {
                _dragOffset = mouse.y - thumb.origin.y;
            }

            return false;
        }

        case QTip::EventType::MouseMoved: {
            if (!_dragging) {
                break;
            }

            const float length = thumbLength();
            const float travel = trackLength() - length;

            if (travel <= 0.0f) {
                _position = 0.0f;
                break;
            }

            const float mousePosition =
                _orientation == ScrollBarOrientation::Horizontal
                    ? event.mouseMove.position.x
                    : event.mouseMove.position.y;

            float thumbPosition =
                mousePosition - _dragOffset;

            thumbPosition = std::clamp(
                thumbPosition,
                trackStart(),
                trackStart() + travel
            );

            _position =
                (thumbPosition - trackStart()) / travel;
            auto position = Detail::eventPosition(event);
            if (position != QTip::Point{-1, -1}) {
                return !_rect.isPointInside(position);
            }
            return true;
        }

        case QTip::EventType::MouseButtonReleased: {
            _dragging = false;
            _dragOffset = 0.0f;
            auto position = Detail::eventPosition(event);
            if (position != QTip::Point{-1, -1}) {
                return !_rect.isPointInside(position);
            }
            return true;
        }

        default:
            break;
    }
    auto position = Detail::eventPosition(event);
    if (position != QTip::Point{-1, -1}) {
        return !_rect.isPointInside(position);
    }
    return true;
}

QTip::Point ScrollBar::minimumSize() const {
    if (_orientation == ScrollBarOrientation::Horizontal) {
        return {
            MinimumThumbLength,
            ScrollbarThickness
        };
    }

    return {
        ScrollbarThickness,
        MinimumThumbLength
    };
}

QTip::Point ScrollBar::preferredSize() const {
    return _preferredRect.size;
}

void ScrollBar::resize(const QTip::Point size) {
    _rect.size = size;
    _preferredRect.size = size;
}

void ScrollBar::reposition(const QTip::Point position) {
    _rect.origin = position;
    _preferredRect.origin = position;
}

void ScrollBar::setRect(const QTip::Rect rect) {
    _rect = rect;
}

const QTip::Rect& ScrollBar::rect() {
    return _rect;
}
