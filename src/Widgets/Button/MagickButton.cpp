//
// Created by Ayaan on 2026-10-19.
//

#include "QTipUIKit/Widgets/Button/MagickButton.h"

MagickButton::MagickButton(
    QTip::Rect rect,
    const ButtonStyle& style,
    RenderCallback callback
) : Button(rect, "", style),
    _callback(std::move(callback)) {
}

void MagickButton::renderImpl(QTip::Window& window) {
    Button::renderImpl(window);
    if (_callback) {
        _callback(window, _rect);
    }
}

