//
// Created by Ayaan on 2026-09-12.
//

#include "QTipUIKit/Containers/Panel.h"

#include <Q-Tip/Graphics/RenderTarget.h>
#include <SDL3/SDL_events.h>

#include "QTipUIKit/UIKitMod.h"
#include "QTipUIKit/Internal/Helpers.h"

Panel::Panel(const QTip::Rect rect) {
    _rect = rect;
    _preferredRect = rect;
}

Panel::~Panel() = default;

void Panel::renderImpl(QTip::Window& window) {
    initTarget();

    window->setTarget(*target);

    window->setBlendMode(QTip::RenderBlendMode::None);
    window->setRenderColor(QTip::Color::clear);
    window->renderRect({0, 0, _rect.size});
    window->resetBlendMode();

    for (const auto& object : _objects) {
        object->render(window);
    }

    window->resetTarget();

    window->renderTexture(*target, QTip::Rect::zero, _rect);
}

void Panel::initTarget() {
    if (!_window)
        return;

    if (target && target->size() == _rect.size)
        return;

    target = std::make_unique<QTip::RenderTarget>(
        _window->getRenderer(),
        _rect.size
    );
}

bool Panel::handleEvent(const SDL_Event& event) {
    const SDL_Event localEvent =
        Detail::transformEvent(event, _rect.origin);

    for (const auto& object : _sortedObjects()) {
        if (!object->handleEvent(localEvent)) {
            return false;
        }
    }
    return true;
}

void Panel::remove(UIObject& object) {
    const auto it = std::ranges::find_if(_objects,
         [&object](const std::unique_ptr<UIObject>& ptr) {
             return ptr.get() == &object;
         }
    );

    if (it != _objects.end()) {
        _objects.erase(it);
    }
}

void Panel::clear() {
    _objects.clear();
}

QTip::Point Panel::minimumSize() const {
    if (_objects.empty())
        return {0, 0};
    float left = std::numeric_limits<float>::max();
    float top = std::numeric_limits<float>::max();
    float right = std::numeric_limits<float>::lowest();
    float bottom = std::numeric_limits<float>::lowest();
    for (const auto& object : _objects) {
        const QTip::Rect rect = object->rect();
        left = std::min(left, rect.origin.x);
        top = std::min(top, rect.origin.y);
        right = std::max(right, rect.origin.x + rect.size.x);
        bottom = std::max(bottom, rect.origin.y + rect.size.y);
    }
    return {right - left, bottom - top};
}

QTip::Point Panel::preferredSize() const {
    return _preferredRect.size;
}

void Panel::resize(QTip::Point size) {
    _rect.size = size;
    _preferredRect.size = size;
}

void Panel::reposition(QTip::Point position) {
    _rect.origin = position;
    _preferredRect.origin = position;
}

void Panel::setRect(QTip::Rect rect) {
    _rect = rect;
}

const QTip::Rect& Panel::rect() {
    return _rect;
}

void Panel::tick(QTip::Clock& clock, double dt) {
    for (auto& object : _objects)
        object->tick(clock, dt);
}

std::vector<UIObject*> Panel::_sortedObjects() const {
    std::vector<UIObject*> sortedObjects;
    for (const auto& object : _objects)
        sortedObjects.push_back(object.get());
    std::ranges::sort(sortedObjects,
                      [](const UIObject* a, const UIObject* b) {
                          return a->zIndex() < b->zIndex();
                      }
    );
    return sortedObjects;
}

void Panel::onAddObject(UIObject*) {
    if (minimumSize() > _rect.size) {
        _rect.size = minimumSize();
        _preferredRect.size = minimumSize();
    }
}
