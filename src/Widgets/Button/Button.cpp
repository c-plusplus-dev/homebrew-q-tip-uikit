//
// Created by Ayaan on 2026-09-11.
//

#include "QTipUIKit/Widgets/Button/Button.h"

#include <SDL3/SDL_events.h>

#include <utility>

Button::Button(float x, float y, float width, float height, std::string_view text, const ButtonStyle& style) {
    _style = style;
    _rect = QTip::Rect{x, y, width, height};
    _preferredRect = _rect;
    _text = std::string(text);
}

Button::Button(QTip::Rect rect, std::string_view text, const ButtonStyle& style) {
    _style = style;
    _rect = rect;
    _preferredRect = _rect;
    _text = std::string(text);
}

bool Button::handleEvent(const SDL_Event& event) {
    if (_disabled) {
        _state = ButtonState::Disabled;
        auto position = Detail::eventPosition(event);
        if (position != QTip::Point{-1, -1}) {
            return !_rect.isPointInside(position);
        }
        return true;
    }
    switch (event.type) {
    default:
        break;
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
        if (_rect.isPointInside({
            event.button.x,
            event.button.y
        })) {
            _state = ButtonState::Pressed;
        }
        break;
    case SDL_EVENT_MOUSE_BUTTON_UP:
        if (_state == ButtonState::Pressed) {
            if (_rect.isPointInside({
                event.button.x,
                event.button.y
            })) {
                if (_onClick) _onClick();
                _state = ButtonState::Hovered;
            } else
                _state = ButtonState::Normal;
        }
        break;
    case SDL_EVENT_MOUSE_MOTION: {
        if (_rect.isPointInside({
            event.motion.x,
            event.motion.y
        })) {
            if (_state != ButtonState::Pressed)
                _state = ButtonState::Hovered;
        } else
            if (_state != ButtonState::Pressed)
                _state = ButtonState::Normal;
        break;
    }
    }
    auto position = Detail::eventPosition(event);
    if (position != QTip::Point{-1, -1}) {
        return !_rect.isPointInside(position);
    }
    return true;
}

void Button::renderImpl(QTip::Window& window) {
    window->setBlendMode(QTip::RenderBlendMode::Blend);
    switch (_state) {
    case ButtonState::Normal:
        window->setRenderColor(_style.color);
        break;
    case ButtonState::Hovered:
        window->setRenderColor(_style.hoverColor);
        break;
    case ButtonState::Pressed:
        window->setRenderColor(_style.pressedColor);
        break;
    case ButtonState::Disabled:
        window->setRenderColor(_style.disabledColor);
        break;
    }
    window->renderRoundedRect(_rect, 10);
    window->resetBlendMode();

    window->setBlendMode(QTip::RenderBlendMode::None);
    window->renderTextCentered(_style.font, _text.c_str(), _rect.center().x, _rect.center().y, _style.fontColor);
    window->resetBlendMode();
}

void Button::setText(std::string text) {
    _text = std::move(text);
}

const std::string& Button::text() const {
    return _text;
}

QTip::Point Button::minimumSize() const {
    return _style.font.getTextSize(_text) + QTip::Point{10, 10}; // 5 px margin on all sides
}

QTip::Point Button::preferredSize() const {
    return _preferredRect.size; // we want what the user had set
}

void Button::resize(QTip::Point size) {
    _rect.size = size;
    _preferredRect.size = size;
}

void Button::reposition(QTip::Point position) {
    _rect.origin = position;
    _preferredRect.origin = position;
}

void Button::setRect(QTip::Rect rect) {
    _rect = rect;
}

const QTip::Rect& Button::rect() {
    return _rect;
}

void Button::setOnClick(std::function<void()> callback) {
    _onClick = std::move(callback);
}

void Button::disable() {
    if (!_disabled)
        _state = ButtonState::Disabled;
    _disabled = true;
}

void Button::enable() {
    if (_disabled)
        _state = ButtonState::Normal;
    _disabled = false;
}

void Button::setDisabled(bool disabled) {
    if (disabled)
        disable();
    else
        enable();
}

bool Button::isDisabled() const {
    return _disabled;
}
