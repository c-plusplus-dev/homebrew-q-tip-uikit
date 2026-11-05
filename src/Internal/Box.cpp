//
// Created by Ayaan on 2026-09-20.
//

#include "QTipUIKit/Internal/Box.h"

#include <algorithm>
#include <numeric>

namespace Detail {

Box::Box(QTip::Rect rect, Settings settings)
    : Panel(rect),
      _settings(settings) {
    layout();
}

void Box::renderImpl(QTip::Window& window) {
    Panel::renderImpl(window);
}

bool Box::handleEvent(const QTip::Event& event) {
    return Panel::handleEvent(event);
}

void Box::remove(UIObject& object) {
    Panel::remove(object);
    layout();
}

void Box::clear() {
    Panel::clear();
    layout();
}

QTip::Point Box::minimumSize() const {
    if (_objects.empty()) {
        return {
            _settings.paddingLeft + _settings.paddingRight,
            _settings.paddingTop + _settings.paddingBottom
        };
    }

    float main = 0.0f;
    float cross = 0.0f;

    for (const auto& object : _objects) {
        const QTip::Point size = object->minimumSize();

        main += mainAxis(size);
        cross = std::max(cross, crossAxis(size));
    }

    main += totalSpacing();

    if (_settings.direction == Direction::Horizontal) {
        return {
            main + _settings.paddingLeft + _settings.paddingRight,
            cross + _settings.paddingTop + _settings.paddingBottom
        };
    }

    return {
        cross + _settings.paddingLeft + _settings.paddingRight,
        main + _settings.paddingTop + _settings.paddingBottom
    };
}

QTip::Point Box::preferredSize() const {
    if (_objects.empty()) {
        return {
            _settings.paddingLeft + _settings.paddingRight,
            _settings.paddingTop + _settings.paddingBottom
        };
    }

    float main = 0.0f;
    float cross = 0.0f;

    for (const auto& object : _objects) {
        const QTip::Point size = object->preferredSize();

        main += mainAxis(size);
        cross = std::max(cross, crossAxis(size));
    }

    main += totalSpacing();

    if (_settings.direction == Direction::Horizontal) {
        return {
            main + _settings.paddingLeft + _settings.paddingRight,
            cross + _settings.paddingTop + _settings.paddingBottom
        };
    }

    return {
        cross + _settings.paddingLeft + _settings.paddingRight,
        main + _settings.paddingTop + _settings.paddingBottom
    };
}

void Box::resize(QTip::Point size) {
    _rect.size = size;
    layout();
}

void Box::reposition(QTip::Point position) {
    _rect.origin = position;
    layout();
}

void Box::setRect(QTip::Rect rect) {
    _rect = rect;
    layout();
}

const QTip::Rect& Box::rect() {
    return _rect;
}

void Box::setSettings(Settings settings) {
    _settings = settings;
    layout();
}

const Box::Settings& Box::settings() const {
    return _settings;
}

void Box::setDirection(Direction direction) {
    _settings.direction = direction;
    layout();
}

Box::Direction Box::direction() const {
    return _settings.direction;
}

void Box::setSizing(Sizing sizing) {
    _settings.sizing = sizing;
    layout();
}

Box::Sizing Box::sizing() const {
    return _settings.sizing;
}

void Box::setCrossSizing(CrossSizing sizing) {
    _settings.crossSizing = sizing;
    layout();
}

Box::CrossSizing Box::crossSizing() const {
    return _settings.crossSizing;
}

void Box::setSpacing(float spacing) {
    _settings.spacing = std::max(0.0f, spacing);
    layout();
}

float Box::spacing() const {
    return _settings.spacing;
}

void Box::setPadding(
    float left,
    float top,
    float right,
    float bottom
) {
    _settings.paddingLeft = std::max(0.0f, left);
    _settings.paddingTop = std::max(0.0f, top);
    _settings.paddingRight = std::max(0.0f, right);
    _settings.paddingBottom = std::max(0.0f, bottom);

    layout();
}

void Box::setPadding(float horizontal, float vertical) {
    setPadding(
        horizontal,
        vertical,
        horizontal,
        vertical
    );
}

float Box::paddingLeft() const {
    return _settings.paddingLeft;
}

float Box::paddingTop() const {
    return _settings.paddingTop;
}

float Box::paddingRight() const {
    return _settings.paddingRight;
}

float Box::paddingBottom() const {
    return _settings.paddingBottom;
}

void Box::layout() {
    if (_objects.empty()) {
        return;
    }

    if (computingLayout) {
        return;
    }

    computingLayout = true;

    const float availableMain = availableMainSize();
    const float availableCross = availableCrossSize();

    const float spacing = totalSpacing();

    float preferredMain = 0.0f;

    for (const auto& object : _objects) {
        preferredMain += mainAxis(object->preferredSize());
    }

    preferredMain += spacing;

    float extraMain = 0.0f;

    if (_settings.sizing == Sizing::Stretch) {
        extraMain = std::max(
            0.0f,
            availableMain - preferredMain
        );
    }

    const float extraPerChild =
        extraMain / static_cast<float>(_objects.size());

    float offset = 0.0f;

    for (std::size_t i = 0; i < _objects.size(); ++i) {
        UIObject& object = *_objects[i];

        const QTip::Point preferred = object.preferredSize();
        const QTip::Point minimum = object.minimumSize();

        float childMain = std::max(
            mainAxis(preferred),
            mainAxis(minimum)
        );

        if (_settings.sizing == Sizing::Stretch) {
            childMain += extraPerChild;
        }

        childMain = std::max(0.0f, childMain);

        // -------------------------
        // Cross-axis sizing
        // -------------------------

        float childCross = crossAxis(preferred);

        childCross = std::max(
            childCross,
            crossAxis(minimum)
        );

        if (_settings.crossSizing == CrossSizing::Stretch) {
            childCross = availableCross;
        }

        childCross = std::min(
            childCross,
            availableCross
        );

        // -------------------------
        // Main-axis position
        // -------------------------

        const float mainPosition =
            (_settings.direction == Direction::Horizontal)
                ? _settings.paddingLeft + offset
                : _settings.paddingTop + offset;

        // -------------------------
        // Cross-axis position
        // -------------------------

        float crossPosition;

        if (_settings.crossSizing == CrossSizing::Stretch) {
            crossPosition =
                (_settings.direction == Direction::Horizontal)
                    ? _settings.paddingTop
                    : _settings.paddingLeft;
        }
        else {
            crossPosition =
                (_settings.direction == Direction::Horizontal)
                    ? _settings.paddingTop
                        + (availableCross - childCross) / 2.0f
                    : _settings.paddingLeft
                        + (availableCross - childCross) / 2.0f;
        }

        const QTip::Point position =
            makePoint(
                mainPosition,
                crossPosition
            );

        const QTip::Point size =
            makeSize(
                childMain,
                childCross
            );

        object.setRect({
            position,
            size
        });

        offset += childMain;

        if (i + 1 < _objects.size()) {
            offset += _settings.spacing;
        }
    }

    if (auto* p = dynamic_cast<Box*>(parent)) {
        p->layout();
    }

    computingLayout = false;
}

void Box::onAddObject(UIObject*) {
    layout();
}

float Box::mainAxis(QTip::Point point) const {
    if (_settings.direction == Direction::Horizontal) {
        return point.x;
    }

    return point.y;
}

float Box::crossAxis(QTip::Point point) const {
    if (_settings.direction == Direction::Horizontal) {
        return point.y;
    }

    return point.x;
}

QTip::Point Box::makePoint(
    float main,
    float cross
) const {
    if (_settings.direction == Direction::Horizontal) {
        return {
            main,
            cross
        };
    }

    return {
        cross,
        main
    };
}

QTip::Point Box::makeSize(
    float main,
    float cross
) const {
    if (_settings.direction == Direction::Horizontal) {
        return {
            main,
            cross
        };
    }

    return {
        cross,
        main
    };
}

float Box::availableMainSize() const {
    if (_settings.direction == Direction::Horizontal) {
        return std::max(
            0.0f,
            _rect.size.x
                - _settings.paddingLeft
                - _settings.paddingRight
        );
    }

    return std::max(
        0.0f,
        _rect.size.y
            - _settings.paddingTop
            - _settings.paddingBottom
    );
}

float Box::availableCrossSize() const {
    if (_settings.direction == Direction::Horizontal) {
        return std::max(
            0.0f,
            _rect.size.y
                - _settings.paddingTop
                - _settings.paddingBottom
        );
    }

    return std::max(
        0.0f,
        _rect.size.x
            - _settings.paddingLeft
            - _settings.paddingRight
    );
}

float Box::totalPreferredMainSize() const {
    float result = 0.0f;

    for (const auto& object : _objects) {
        result += mainAxis(object->preferredSize());
    }

    return result;
}

float Box::totalMinimumMainSize() const {
    float result = 0.0f;

    for (const auto& object : _objects) {
        result += mainAxis(object->minimumSize());
    }

    return result;
}

float Box::totalSpacing() const {
    if (_objects.size() <= 1) {
        return 0.0f;
    }

    return _settings.spacing *
           static_cast<float>(_objects.size() - 1);
}

} // namespace Detail