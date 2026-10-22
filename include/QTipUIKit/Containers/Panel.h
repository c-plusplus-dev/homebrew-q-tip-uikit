//
// Created by Ayaan on 2026-09-12.
//

#ifndef QTIPUIKIT_PANEL_H
#define QTIPUIKIT_PANEL_H
#include <Q-Tip/Types/Lazy.h>

#include "QTipUIKit/UIObject.h"

class Panel : public UIObject {
public:
    Panel(QTip::Rect rect);

    ~Panel() override;

    bool handleEvent(const SDL_Event& event) override;

    template<std::derived_from<UIObject> T, typename... Args>
    T& add(Args&&... args) {
        auto object = std::make_unique<T>(
            std::forward<Args>(args)...
        );

        T* result = object.get();
        _objects.push_back(std::move(object));

        result->parent = this;

        onAddObject(result);

        return *result;
    }

    virtual void remove(UIObject& object);

    virtual void clear();

    [[nodiscard]] QTip::Point minimumSize() const override;
    [[nodiscard]] QTip::Point preferredSize() const override;

    void resize(QTip::Point size) override;
    void reposition(QTip::Point position) override;
    void setRect(QTip::Rect rect) override;
    [[nodiscard]] const QTip::Rect& rect() override;

    void tick(QTip::Clock& clock, double dt) override;

protected:
    std::vector<std::unique_ptr<UIObject>> _objects;

    std::unique_ptr<QTip::RenderTarget> _target;

    std::vector<UIObject*> _sortedObjects() const;

    virtual void onAddObject(UIObject* object);

    void renderImpl(QTip::Window& window) override;
    virtual void initTarget();
};

#endif //QTIPUIKIT_PANEL_H
