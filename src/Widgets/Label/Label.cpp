//
// Created by Ayaan on 2026-09-12.
//

#include "QTipUIKit/Widgets/Label/Label.h"

#include "QTipUIKit/Internal/Helpers.h"

Label::Label(const std::string& text, QTip::Color color, QTip::Point position, const std::optional<QTip::Font>& font, bool centered) : _font(Detail::defaultFontPath(), 16) {
    if (font.has_value()) {
        _font = font.value();
    }
    _text = text;
    _color = color;
    _position = position;
    _centered = centered;
}

void Label::renderImpl(QTip::Window& window) {
    window->setBlendMode(QTip::RenderBlendMode::None);
    if (_centered)
        window->renderTextCentered(_font, _text.c_str(), _position.x, _position.y, _color);
    else
        window->renderText(_font, _text.c_str(), _position.x, _position.y, _color);
    window->resetBlendMode();
}

QTip::Point Label::minimumSize() const {
    return _font.getTextSize(_text); // we dont scale
}

QTip::Point Label::preferredSize() const {
    return _font.getTextSize(_text); // we dont scale
}

void Label::resize(QTip::Point size) {
    // do nothing, cannot resize
}

void Label::reposition(QTip::Point position) {
    _position = position;
}

void Label::setRect(QTip::Rect rect) {
    _position = rect.origin; // the best we can do
}

const QTip::Rect& Label::rect() {
    _rect = {_position, _font.getTextSize(_text)};
    return _rect;
}

void Label::setText(const std::string& text) {
    _text = text;
}

const std::string& Label::text() const {
    return _text;
}
