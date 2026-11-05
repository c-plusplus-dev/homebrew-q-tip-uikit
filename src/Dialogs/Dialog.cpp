//
// Created by Ayaan on 2026-09-30.
//

#include "QTipUIKit/Dialogs/Dialog.h"

#include "QTipUIKit/UIKitMod.h"

Dialog::Dialog() : UIObject(NoRegTag{}), _content({}), _buttons({}), _lastWindowSize() { // dont reg in global
    _buttons.setSizing(Sizing::Stretch);
    _content.parent = this;
    _buttons.parent = this;
}

Dialog::~Dialog() = default;

void Dialog::renderImpl(QTip::Window& window) {
    layout();

    window->setBlendMode(QTip::RenderBlendMode::Blend);
    window->setRenderColor({0, 0, 0, 128});
    window->renderRect({0, 0, window.size()});
    window->resetBlendMode();

    window->setRenderColor({64, 64, 64, 255});

    QTip::Rect rect = {window.size() / 8.0f, window.size() / 1.33f};

    window->renderRoundedRect(rect, 10);

    window->setRenderColor(QTip::Color::white);

    _content.render(window);
    _buttons.render(window);
}

bool Dialog::handleEvent(const QTip::Event& event) {
    _content.handleEvent(event);
    _buttons.handleEvent(event);

    if (_requestedClose)
        end();
    return false;
}

QTip::Point Dialog::minimumSize() const { return {}; }

QTip::Point Dialog::preferredSize() const { return {}; }

void Dialog::resize(QTip::Point size) {}
void Dialog::reposition(QTip::Point position) {}
void Dialog::setRect(QTip::Rect rect) {}
const QTip::Rect& Dialog::rect() { return _rect; }

void Dialog::setContentResizeHandler(ResizeHandler handler) {
    _content_resize_handler = handler;
}

void Dialog::setButtonsResizeHandler(ResizeHandler handler) {
    _buttons_resize_handler = handler;
}

void Dialog::tick(QTip::Clock& clock, double dt) {
    _content.tick(clock, dt);
    _buttons.tick(clock, dt);
}

Panel& Dialog::content() {
    return _content;
}

HBox& Dialog::buttons() {
    return _buttons;
}

void Dialog::close() {
    _requestedClose = true;
}

Dialog& Dialog::create(QTip::Window& window) {
    auto dialog = std::unique_ptr<Dialog>(new Dialog());

    dialog->initialize(window);

    auto& manager = *UIKitMod::instance();

    return manager.addDialog(
        window,
        std::move(dialog)
    );
}

void Dialog::initialize(QTip::Window& window) {
    _window = &window;
    layout();
}

void Dialog::end() {
    auto& manager = *UIKitMod::instance();

    manager.removeDialog(*_window, this);
}

void Dialog::layout() {
    QTip::Point windowSize = _window->size();
    if (windowSize == _lastWindowSize)
        return;
    _lastWindowSize = windowSize;

    QTip::Rect rect = {windowSize / 8.0f, windowSize / 1.33f};

    QTip::Rect contentRect = {rect.origin.x + 10, rect.origin.y + 10, rect.size.x - 20, rect.size.y - 80};
    _content.setRect(contentRect);

    QTip::Rect buttonRect = {rect.origin.x + 10, rect.origin.y + 10 + rect.size.y - 70, rect.size.x - 20, 50};
    _buttons.setRect(buttonRect);

    if (_content_resize_handler)
        _content_resize_handler(contentRect.size);

    if (_buttons_resize_handler)
        _buttons_resize_handler(buttonRect.size);
}
