#include <cassert>
#include <ranges>
#include <Q-Tip/QTip.h>

#include "QTipUIKit/UIKitMod.h"
#include "QTipUIKit/UIObject.h"
#include "QTipUIKit/Internal/Helpers.h"
#include "QTipUIKit/Dialogs/Dialog.h"

#define UICONTEXT std::shared_ptr<UIContext>

UIKitMod::UIKitMod() = default;

UIKitMod::~UIKitMod() = default;

std::string_view UIKitMod::id() const {
    return ID;
}

std::string_view UIKitMod::name() const {
    return NAME;
}

void UIKitMod::init() {
    [[maybe_unused]] const bool
        _moddable_variable_registration_0 = [] {
            QTip::Window::extendVariable<UICONTEXT>("context");
            return true;
    }();
}

void UIKitMod::shutdown() {
}

void UIKitMod::handleEvent(const QTip::Event& event) {
    SDL_Window* windowID = SDL_GetWindowFromID(event.windowID);
    auto windows = QTip::QTipRuntime::windows();
    auto it = std::ranges::find_if(windows, [&] (const auto& window) {
        return static_cast<SDL_Window*>(*window) == windowID;
    });

    if (it != windows.end()) {
        auto& window = **it;
        auto c = context(window);
        assert(c && "Window has no UIContext");
        c->handleEvent(event);
    }
}

void UIKitMod::beforePresent(QTip::Window& window) {
    double dt = _clock.elapsedFromLastCall() / 1000.0;
    if (auto c = context(window))
        c->beforePresent(window, _clock, dt);
}

void UIKitMod::windowCreated(QTip::Window& window) {
    context(window);
}

void UIKitMod::windowDestroyed(QTip::Window& window) {}

UIKitMod* UIKitMod::instance() {
    return QTip::ModLoader::mod<UIKitMod>();
}

UIContext* UIKitMod::context(QTip::Window& window) {
    auto& storedContext = window.v<UICONTEXT>("context");
    if (!storedContext) {
        storedContext = std::make_shared<UIContext>();
        storedContext->_window = &window;
    }
    return storedContext.get();
}

Dialog& UIKitMod::addDialog(QTip::Window& window, std::unique_ptr<Dialog> dialog) {
    auto c = context(window);
    c->_dialogs.push_back(std::move(dialog));
    return *c->_dialogs.back();
}

void UIKitMod::removeDialog(QTip::Window& window, Dialog* dialog) {
    auto manager = context(window);
    auto it = std::ranges::find_if(manager->_dialogs.begin(), manager->_dialogs.end(), [&](const auto& d) {
        return d.get() == dialog;
    });
    if (it != manager->_dialogs.end()) {
        manager->removeDialog(dialog);
    }
}

void UIKitMod::setAddingChildren(QTip::Window* window, bool adding) {
    context(*window)->_addingChildren = adding;
}
