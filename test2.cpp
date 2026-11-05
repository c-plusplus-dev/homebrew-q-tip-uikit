//
// Created by Ayaan on 2026-09-29.
//

#include <Q-Tip/QTip.h>
#include <QTipUIKit/UIKit.h>

#include "QTipUIKit/Containers/List.h"
using namespace QTip;

int main() {
    ModLoader::load<UIKitMod>();

    Window window("helloo", 2000, 2000);

    ButtonStyle style;
    style.color = Color::white;
    style.disabledColor = Color::blue;
    style.hoverColor = Color::green;
    style.pressedColor = Color::red;

    {
        Button button(0, 0, 100, 100, "hello", style);

        List list({100, 100, 100, 100}, List::Direction::Down, 5.0f);
        list.add<Label>("label1", Color::white, Point{0, 0});
        list.add<Label>("label2", Color::white, Point{0, 0});
        list.add<Label>("label3", Color::white, Point{0, 0});
        list.add<Label>("label4", Color::white, Point{0, 0});
        list.add<Label>("label5", Color::white, Point{0, 0});

        bool shouldClose = false;

        button.setOnClick([&shouldClose] {
            shouldClose = true;
        });

        while (!window.shouldClose() && !shouldClose) {
            QTipRuntime::fetchEvents();

            window->setRenderColor(Color::black);
            window->clear();

            button.render(window);
            list.render(window);

            window->present();
        }
    }

    Button button(100, 100, 100, 100, "hello2", style);

    while (!window.shouldClose()) {
        QTipRuntime::fetchEvents();

        window->setRenderColor(Color::black);
        window->clear();

        button.render(window);

        window->present();
    }

    return 0;
}
