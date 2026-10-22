//
// Created by Ayaan on 2026-09-27.
//

#include "QTipUIKit/Containers/ScrollView.h"

#include <algorithm>
#include <SDL3/SDL_events.h>

#include "include/Helpers.h"
#include "QTipUIKit/Internal/Helpers.h"

bool overlap(const QTip::Rect& rect1, const QTip::Rect& rect2) {
    const float left1   = rect1.origin.x;
    const float right1  = rect1.origin.x + rect1.size.x;
    const float top1    = rect1.origin.y;
    const float bottom1 = rect1.origin.y + rect1.size.y;

    const float left2   = rect2.origin.x;
    const float right2  = rect2.origin.x + rect2.size.x;
    const float top2    = rect2.origin.y;
    const float bottom2 = rect2.origin.y + rect2.size.y;

    return left1 <= right2 &&
           right1 >= left2 &&
           top1 <= bottom2 &&
           bottom1 >= top2;
}

ScrollView::ScrollView(
    QTip::Rect rect,
    ScrollViewSettings settings
)
    : Panel(rect),
      _settings(settings) {
    _horizontalScrollBar = new ScrollBar{
        QTip::Rect{
            5.0f,
            rect.size.y - 5.0f,
            {rect.size.x - 10, 10.0f}
        },
        ScrollBarOrientation::Horizontal
    };
    _verticalScrollBar = new ScrollBar{
        QTip::Rect{
            rect.size.x - 5.0f,
            5.0f,
            {10.0f, rect.size.y - 10}
        },
        ScrollBarOrientation::Vertical
    };
    _horizontalScrollBar->parent = this;
    _verticalScrollBar  ->parent = this;
    updateScrollBarsDimensions();
    updateScrollBars();
}

ScrollView::~ScrollView() {
    delete _horizontalScrollBar;
    delete _verticalScrollBar;
    _horizontalScrollBar = nullptr;
    _verticalScrollBar = nullptr;
}

void ScrollView::scrollTo(QTip::Point position) {
    _scrollPosition = position;

    clampScrollPosition();
    updateScrollBars();
}

void ScrollView::scrollBy(QTip::Point delta) {
    _scrollPosition += delta;

    clampScrollPosition();
    updateScrollBars();
}

QTip::Point ScrollView::scrollPosition() const {
    return _scrollPosition;
}

void ScrollView::setContentSize(QTip::Point size) {
    _contentSize = size;

    clampScrollPosition();
    updateScrollBars();
}

QTip::Point ScrollView::contentSize() const {
    return _contentSize;
}

const ScrollViewSettings& ScrollView::settings() const {
    return _settings;
}

void ScrollView::setScrollSpeed(float scrollSpeed) {
    _settings.scrollSpeed = scrollSpeed;
}

void ScrollView::showScrollbars(bool show) {
    _settings.showScrollbar = show;
}

void ScrollView::clampScrollPosition() {
    const float viewportWidth = rect().size.x;
    const float viewportHeight = rect().size.y;

    const float maxX = std::max(0.0f, _contentSize.x - viewportWidth);
    const float maxY = std::max(0.0f, _contentSize.y - viewportHeight);

    switch (_settings.direction) {
        case ScrollDirection::Horizontal:
            _scrollPosition.x = std::clamp(_scrollPosition.x, 0.0f, maxX);
            _scrollPosition.y = 0.0f;
            break;

        case ScrollDirection::Vertical:
            _scrollPosition.x = 0.0f;
            _scrollPosition.y = std::clamp(_scrollPosition.y, 0.0f, maxY);
            break;

        case ScrollDirection::Both:
            _scrollPosition.x = std::clamp(_scrollPosition.x, 0.0f, maxX);
            _scrollPosition.y = std::clamp(_scrollPosition.y, 0.0f, maxY);
            break;
    }
}

void ScrollView::updateScrollBars() {
    const float maxX =
        std::max(0.0f, _contentSize.x - rect().size.x);

    const float maxY =
        std::max(0.0f, _contentSize.y - rect().size.y);

    _horizontalScrollBar->setPosition(
        maxX > 0.0f
            ? _scrollPosition.x / maxX
            : 0.0f
    );

    _verticalScrollBar->setPosition(
        maxY > 0.0f
            ? _scrollPosition.y / maxY
            : 0.0f
    );

    _horizontalScrollBar->setViewRatio(
        _contentSize.x > 0.0f
            ? rect().size.x / _contentSize.x
            : 1.0f
    );

    _verticalScrollBar->setViewRatio(
        _contentSize.y > 0.0f
            ? rect().size.y / _contentSize.y
            : 1.0f
    );
}

void ScrollView::updateScrollBarsDimensions() {
    _horizontalScrollBar->resize({rect().size.x - 20.0f, 20.0f});
    if (_settings.direction == ScrollDirection::Both)
        _verticalScrollBar->resize({20.0f, rect().size.y - 20.0f - 10.0f});
    else
        _verticalScrollBar->resize({20.0f, rect().size.y - 20.0f});

    _horizontalScrollBar->reposition({rect().origin.x + 15.0f, rect().origin.y + rect().size.y - 15.0f});
    _verticalScrollBar->reposition({rect().origin.x + rect().size.x - 15.0f, rect().origin.y + 15.0f});
}

bool ScrollView::x() const {
    return _settings.direction == ScrollDirection::Both || _settings.direction == ScrollDirection::Horizontal;
}

bool ScrollView::y() const {
    return _settings.direction == ScrollDirection::Both || _settings.direction == ScrollDirection::Vertical;
}

void ScrollView::renderImpl(QTip::Window& window) {
    initTarget();

    window->setTarget(*target);

    window->setBlendMode(QTip::RenderBlendMode::None);
    window->setRenderColor(QTip::Color::clear);
    window->renderRect({0, 0, _contentSize});
    window->resetBlendMode();

    for (const auto& child : _objects) {
        const QTip::Rect childRect = child->rect();

        bool overlapping = overlap(childRect, {
            0,
            0,
            _contentSize.x,
            _contentSize.y
        });
        if (!overlapping)
            continue;

        child->render(window);
    }

    window->resetTarget();

    QTip::Point size = rect().size;

    size.x = std::min(size.x, _contentSize.x);
    size.y = std::min(size.y, _contentSize.y);

    window->renderTexture(
        *target,
        QTip::Rect{
            .origin = _scrollPosition,
            .size = size
        },
        QTip::Rect{
            .origin = rect().origin,
            .size = size
        }
    );

    if (x() && _contentSize.x > rect().size.x && _settings.showScrollbar)
        _horizontalScrollBar->render(window);
    if (y() && _contentSize.y > rect().size.y && _settings.showScrollbar)
        _verticalScrollBar->render(window);
}

void ScrollView::initTarget() {
    if (!_window)
        return;

    if (target && target->size() == _contentSize)
        return;

    target = std::make_unique<QTip::RenderTarget>(
        *_window,
        _contentSize
    );
}

bool ScrollView::handleEvent(const SDL_Event& event) {
    auto position = Detail::eventPosition(event);
    if (position != QTip::Point{-1, -1} && !rect().isPointInside(position))
        return true;

    SDL_Event localEvent =
        Detail::transformEvent(event, _rect.origin);

    if (_settings.showScrollbar) {
        bool x = !_horizontalScrollBar->handleEvent(event);
        bool y = !_verticalScrollBar->handleEvent(event);

        const float maxX =
            std::max(0.0f, _contentSize.x - rect().size.x);

        const float maxY =
            std::max(0.0f, _contentSize.y - rect().size.y);

        _scrollPosition.x = _horizontalScrollBar->position() * maxX;
        _scrollPosition.y = _verticalScrollBar->position() * maxY;

        if (x || y)
            return false;
    }

    localEvent =
        Detail::transformEvent(event, _rect.origin - _scrollPosition);

    bool propagate = true;
    for (const auto& object : _sortedObjects()) {
        if (!object->handleEvent(localEvent)) {
            propagate = false;
            break;
        }
    }

    if (event.type == SDL_EVENT_MOUSE_WHEEL) {
        QTip::Point delta{
            event.wheel.x * _settings.scrollSpeed,
            -event.wheel.y * _settings.scrollSpeed
        };

        switch (_settings.direction) {
            case ScrollDirection::Horizontal:
                delta.y = 0.0f;
                break;

            case ScrollDirection::Vertical:
                delta.x = 0.0f;
                break;

            case ScrollDirection::Both:
                break;
        }

        scrollBy(delta);
    }
    return propagate;
}

void ScrollView::resize(QTip::Point size) {
    Panel::resize(size);
    updateScrollBars();
    updateScrollBarsDimensions();
}

void ScrollView::reposition(QTip::Point position) {
    Panel::reposition(position);
    updateScrollBars();
    updateScrollBarsDimensions();
}

void ScrollView::setRect(QTip::Rect rect) {
    Panel::setRect(rect);
    updateScrollBars();
    updateScrollBarsDimensions();
}

const QTip::Rect& ScrollView::rect() {
    return Panel::rect();
}
