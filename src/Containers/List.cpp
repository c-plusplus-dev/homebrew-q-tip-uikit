//
// Created by Ayaan on 2026-10-03.
//

#include "QTipUIKit/Containers/List.h"

#include <algorithm>

List::List(QTip::Rect rect, Direction direction, float spacing)
    : Panel(rect),
      _direction(direction),
      _spacing(spacing) {

    updateLayout();
}

void List::renderImpl(QTip::Window& window) {
    Panel::renderImpl(window);
}

bool List::handleEvent(const SDL_Event& event) {
    return Panel::handleEvent(event);
}

void List::remove(UIObject& object) {
    Panel::remove(object);
    updateLayout();
}

void List::clear() {
    Panel::clear();
    updateLayout();
}

void List::setDirection(Direction direction) {
    _direction = direction;
    updateLayout();
}

List::Direction List::direction() const {
    return _direction;
}

void List::setSpacing(float spacing) {
    _spacing = spacing;
    updateLayout();
}

float List::spacing() const {
    return _spacing;
}

void List::onAddObject(UIObject*) {
    updateLayout();
}

void List::updateLayout() {
    resizeToFitChildren();
    layout();
}

void List::layout() {
    float x = 0.0f;
    float y = 0.0f;

    switch (_direction) {
        case Direction::Up:
            y = _rect.size.y;
            break;

        case Direction::Left:
            x = _rect.size.x;
            break;

        case Direction::Down:
        case Direction::Right:
            break;
    }

    for (auto& object : _objects) {
        const QTip::Rect rect = object->rect();

        switch (_direction) {
            case Direction::Down:
                object->reposition({x, y});
                y += rect.size.y + _spacing;
                break;

            case Direction::Up:
                y -= rect.size.y;
                object->reposition({x, y});
                y -= _spacing;
                break;

            case Direction::Right:
                object->reposition({x, y});
                x += rect.size.x + _spacing;
                break;

            case Direction::Left:
                x -= rect.size.x;
                object->reposition({x, y});
                x -= _spacing;
                break;
        }
    }
}

void List::resizeToFitChildren() {
    float width = 0.0f;
    float height = 0.0f;

    if (_objects.empty()) {
        _rect.size.x = 0.0f;
        _rect.size.y = 0.0f;
        return;
    }

    for (auto& object : _objects) {
        const QTip::Rect rect = object->rect();

        width = std::max(width, rect.size.x);
        height = std::max(height, rect.size.y);
    }

    if (_direction == Direction::Down ||
        _direction == Direction::Up) {

        height = 0.0f;

        for (auto& object : _objects)
            height += object->rect().size.y;

        height += _spacing * static_cast<float>(_objects.size() - 1);
    }
    else {
        width = 0.0f;

        for (auto& object : _objects)
            width += object->rect().size.x;

        width += _spacing * static_cast<float>(_objects.size() - 1);
    }

    _rect.size.x = width;
    _rect.size.y = height;
}
