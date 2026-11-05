//
// Created by Ayaan on 2026-10-09.
//

#include "../include/QTipUIKit/UIContext.h"

#include <magic_enum/magic_enum.hpp>
#include <SDL3/SDL_events.h>

#include "QTipUIKit/UIKitMod.h"
#include "QTipUIKit/UIObject.h"
#include "QTipUIKit/Internal/Helpers.h"

UIContext::UIContext() {
    UIKitMod::instance()->_contexts.push_back(this);
}

UIContext::~UIContext() {
    auto manager = UIKitMod::instance();
    auto it = std::ranges::find_if(manager->_contexts, [&] (const auto context) {
        return context == this;
    });
    if (it != manager->_contexts.end())
        manager->_contexts.erase(it);
}

void UIContext::add(UIObject* object) {
    if (_addingChildren) return;
    _objects.push_back(object);
}

void UIContext::remove(UIObject* object) {
    const auto it = std::ranges::find_if(_objects,
         [&object](const UIObject* ptr) {
             return ptr == object;
         }
    );

    if (it != _objects.end()) {
        _objects.erase(it);
    }
}

void UIContext::beforePresent(QTip::Window& window, QTip::Clock& clock, double dt) {
    for (const auto object : _objects)
        object->tick(clock, dt);

    if (!_removalPendingDialogs.empty()) {
        for (Dialog* dialog : _removalPendingDialogs) {
            auto it = std::ranges::find_if(
                _dialogs,
                [dialog](const auto& ownedDialog) {
                    return ownedDialog.get() == dialog;
                }
            );

            if (it != _dialogs.end())
                _dialogs.erase(it);
        }

        _removalPendingDialogs.clear();
    }

    for (auto& dialog : _dialogs) {
        dialog->render(window);
        dialog->tick(clock, dt);
    }

    _firstEventPool = false;
}

void UIContext::handleEvent(const QTip::Event& event) {
    if (_firstEventPool) {
        _eventPool.push_back(event);
        return;
    }
    if (!_eventPool.empty()) {
        for (auto& e : _eventPool) {
            if (!_dialogs.empty()) {
                _dialogs.back()->handleEvent(e);
                continue;
            }

            for (const auto object : _objects)
                object->handleEvent(e);
        }
        _eventPool.clear();
    }

    if (!_dialogs.empty()) {
        _dialogs.back()->handleEvent(event);
        return;
    }

    for (const auto object : _objects)
        object->handleEvent(event);
}

void UIContext::removeDialog(Dialog* dialog) {
    _removalPendingDialogs.push_back(dialog);
}
