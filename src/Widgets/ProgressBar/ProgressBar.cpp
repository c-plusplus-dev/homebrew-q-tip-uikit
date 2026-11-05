//
// Created by ChatGPT on 2026-11-05.
//

#include "QTipUIKit/Widgets/ProgressBar/ProgressBar.h"

#include <algorithm>
#include <cmath>

ProgressBar::ProgressBar(
    const QTip::Rect rect,
    const ProgressBarStyle& style
)
    : _style(style) {
    _rect = rect;
    _preferredRect = rect;
}

void ProgressBar::setProgress(const float progress) {
    // NaN has no meaningful place on the [0, 1] progress scale.
    _progress = std::isnan(progress)
        ? 0.0f
        : std::clamp(progress, 0.0f, 1.0f);
}

float ProgressBar::progress() const {
    return _progress;
}

void ProgressBar::setStyle(const ProgressBarStyle& style) {
    _style = style;
}

const ProgressBarStyle& ProgressBar::style() const {
    return _style;
}

QTip::Point ProgressBar::minimumSize() const {
    return {40.0f, 8.0f};
}

QTip::Point ProgressBar::preferredSize() const {
    return _preferredRect.size;
}

void ProgressBar::resize(const QTip::Point size) {
    _rect.size = size;
    _preferredRect.size = size;
}

void ProgressBar::reposition(const QTip::Point position) {
    _rect.origin = position;
    _preferredRect.origin = position;
}

void ProgressBar::setRect(const QTip::Rect rect) {
    _rect = rect;
}

const QTip::Rect& ProgressBar::rect() {
    return _rect;
}

void ProgressBar::renderImpl(QTip::Window& window) {
    const float width = std::max(0.0f, _rect.size.x);
    const float height = std::max(0.0f, _rect.size.y);

    if (width <= 0.0f || height <= 0.0f)
        return;

    auto& renderer = window.getRenderer();

    const int maximumRadius = static_cast<int>(
        std::min(width, height) / 2.0f
    );

    const int radius = std::clamp(
        _style.cornerRadius,
        0,
        maximumRadius
    );

    renderer.setBlendMode(QTip::RenderBlendMode::Blend);

    renderer.setRenderColor(_style.trackColor);
    renderer.renderRoundedRect(
        {_rect.origin, {width, height}},
        radius
    );

    const float progressWidth = width * _progress;

    if (progressWidth > 0.0f) {
        const int progressRadius = std::min(
            radius,
            static_cast<int>(progressWidth / 2.0f)
        );

        renderer.setRenderColor(_style.progressColor);
        renderer.renderRoundedRect(
            {_rect.origin, {progressWidth, height}},
            progressRadius
        );
    }

    renderer.resetBlendMode();
}
