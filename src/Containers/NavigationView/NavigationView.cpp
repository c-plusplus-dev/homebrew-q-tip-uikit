#include "QTipUIKit/Containers/NavigationView/NavigationView.h"

#include <algorithm>
#include <stdexcept>

#include <SDL3/SDL_events.h>

#include <Q-Tip/Graphics/RenderTarget.h>

#include "QTipUIKit/Internal/Helpers.h"

NavigationView::NavigationView(
    QTip::Rect rect,
    NavigationPosition position,
    float barSize
)
    : Panel(rect),
      _position(position),
      _barSize(std::max(0.0f, barSize)) {
}

NavigationView::~NavigationView() = default;

void NavigationView::select(View& view) {
    const auto it = std::ranges::find_if(
        _entries,
        [&view](const Entry& entry) {
            return entry.view.get() == &view;
        }
    );

    if (it == _entries.end()) {
        throw std::invalid_argument(
            "NavigationView::select: view does not belong to this NavigationView"
        );
    }

    select(
        static_cast<std::size_t>(
            std::distance(_entries.begin(), it)
        )
    );
}

void NavigationView::select(std::size_t index) {
    if (index >= _entries.size()) {
        throw std::out_of_range(
            "NavigationView::select: index is out of range"
        );
    }

    _selectedIndex = index;

    for (std::size_t i = 0; i < _entries.size(); ++i) {
        _entries[i].button->setDisabled(i == _selectedIndex);
    }
}

View* NavigationView::selectedView() const {
    if (_entries.empty()) {
        return nullptr;
    }

    return _entries[_selectedIndex].view.get();
}

std::size_t NavigationView::selectedIndex() const {
    return _selectedIndex;
}

std::size_t NavigationView::viewCount() const {
    return _entries.size();
}

void NavigationView::setPosition(NavigationPosition position) {
    _position = position;
    layout();
}

NavigationPosition NavigationView::position() const {
    return _position;
}

void NavigationView::setBarSize(float size) {
    _barSize = std::max(0.0f, size);
    layout();
}

float NavigationView::barSize() const {
    return _barSize;
}

bool NavigationView::handleEvent(const SDL_Event& event) {
    const SDL_Event localEvent =
        Detail::transformEvent(event, _rect.origin);

    // Dispatch to the buttons first, so navigation interactions
    // take precedence over interactions in the active view.
    for (const auto& entry : _entries) {
        if (!entry.button->handleEvent(localEvent)) {
            return false;
        }
    }

    if (View* view = selectedView()) {
        if (!view->handleEvent(localEvent)) {
            return false;
        }
    }

    return true;
}

QTip::Point NavigationView::minimumSize() const {
    QTip::Point result{0, 0};

    if (_entries.empty()) {
        return result;
    }

    float maximumButtonWidth = 0.0f;
    float maximumButtonHeight = 0.0f;

    for (const auto& entry : _entries) {
        const QTip::Point size = entry.button->minimumSize();

        maximumButtonWidth =
            std::max(maximumButtonWidth, size.x);

        maximumButtonHeight =
            std::max(maximumButtonHeight, size.y);
    }

    if (_position == NavigationPosition::Top ||
        _position == NavigationPosition::Bottom) {
        result = {
            maximumButtonWidth *
                static_cast<float>(_entries.size()),
            maximumButtonHeight
        };
    } else {
        result = {
            maximumButtonWidth,
            maximumButtonHeight *
                static_cast<float>(_entries.size())
        };
    }

    if (_position == NavigationPosition::Top ||
        _position == NavigationPosition::Bottom) {
        result.y = std::max(result.y, _barSize);
    } else {
        result.x = std::max(result.x, _barSize);
    }

    if (const View* view = selectedView()) {
        const QTip::Point viewSize = view->minimumSize();

        if (_position == NavigationPosition::Top ||
            _position == NavigationPosition::Bottom) {
            result.x = std::max(result.x, viewSize.x);
            result.y += viewSize.y;
        } else {
            result.x += viewSize.x;
            result.y = std::max(result.y, viewSize.y);
        }
    }

    return result;
}

QTip::Point NavigationView::preferredSize() const {
    return _preferredRect.size;
}

void NavigationView::resize(QTip::Point size) {
    _rect.size = size;
    _preferredRect.size = size;

    layout();
}

void NavigationView::reposition(QTip::Point position) {
    _rect.origin = position;
    _preferredRect.origin = position;

    layout();
}

void NavigationView::setRect(QTip::Rect rect) {
    _rect = rect;
    layout();
}

const QTip::Rect& NavigationView::rect() {
    return _rect;
}

void NavigationView::tick(QTip::Clock& clock, double dt) {
    for (auto& entry : _entries) {
        entry.button->tick(clock, dt);
    }

    if (View* view = selectedView()) {
        view->tick(clock, dt);
    }
}

void NavigationView::renderImpl(QTip::Window& window) {
    initTarget();

    if (!target) {
        return;
    }

    window->setTarget(*target);

    window->setBlendMode(QTip::RenderBlendMode::None);
    window->setRenderColor(QTip::Color::clear);
    window->renderRect({
        0,
        0,
        _rect.size
    });
    window->resetBlendMode();

    // Only the selected view is rendered.
    if (View* view = selectedView()) {
        view->render(window);
    }

    // Render the navigation controls above the active view.
    for (const auto& entry : _entries) {
        entry.button->render(window);
    }

    window->resetTarget();

    window->renderTexture(
        *target,
        QTip::Rect::zero,
        _rect
    );
}

void NavigationView::layout() {
    if (_rect.size.x < 0.0f || _rect.size.y < 0.0f) {
        return;
    }

    constexpr float padding = 8.0f;
    constexpr float itemSpacing = 6.0f;

    const float width = _rect.size.x;
    const float height = _rect.size.y;

    const bool horizontal =
        _position == NavigationPosition::Top ||
        _position == NavigationPosition::Bottom;

    const float actualBarSize = std::min(
        _barSize,
        horizontal ? height : width
    );

    QTip::Rect viewRect{};
    QTip::Rect barRect{};

    switch (_position) {
    case NavigationPosition::Top:
        barRect = {
            padding,
            padding,
            width - padding * 2.0f,
            actualBarSize
        };

        viewRect = {
            padding,
            actualBarSize + padding * 2.0f,
            std::max(0.0f, width - padding * 2.0f),
            std::max(0.0f, height - actualBarSize - padding * 3.0f)
        };
        break;

    case NavigationPosition::Bottom:
        barRect = {
            padding,
            height - actualBarSize - padding,
            width - padding * 2.0f,
            actualBarSize
        };

        viewRect = {
            padding,
            padding,
            std::max(0.0f, width - padding * 2.0f),
            std::max(0.0f, height - actualBarSize - padding * 3.0f)
        };
        break;

    case NavigationPosition::Left:
        barRect = {
            padding,
            padding,
            actualBarSize,
            height - padding * 2.0f
        };

        viewRect = {
            actualBarSize + padding * 2.0f,
            padding,
            std::max(0.0f, width - actualBarSize - padding * 3.0f),
            std::max(0.0f, height - padding * 2.0f)
        };
        break;

    case NavigationPosition::Right:
        barRect = {
            width - actualBarSize - padding,
            padding,
            actualBarSize,
            height - padding * 2.0f
        };

        viewRect = {
            padding,
            padding,
            std::max(0.0f, width - actualBarSize - padding * 3.0f),
            std::max(0.0f, height - padding * 2.0f)
        };
        break;
    }

    for (auto& entry : _entries) {
        entry.view->setRect(viewRect);
    }

    if (_entries.empty()) {
        return;
    }

    const float itemCount = static_cast<float>(_entries.size());

    for (std::size_t i = 0; i < _entries.size(); ++i) {
        QTip::Rect buttonRect = barRect;

        if (horizontal) {
            const float itemWidth =
                (barRect.size.x - itemSpacing * (itemCount - 1.0f))
                / itemCount;

            buttonRect.origin.x =
                barRect.origin.x +
                static_cast<float>(i) * (itemWidth + itemSpacing);

            buttonRect.size.x = std::max(0.0f, itemWidth);
        } else {
            const float itemHeight =
                (barRect.size.y - itemSpacing * (itemCount - 1.0f))
                / itemCount;

            buttonRect.origin.y =
                barRect.origin.y +
                static_cast<float>(i) * (itemHeight + itemSpacing);

            buttonRect.size.y = std::max(0.0f, itemHeight);
        }

        _entries[i].button->setRect(buttonRect);
    }
}
