//
// Created by ChatGPT on 2026-11-05.
//

#include <Q-Tip/QTip.h>
#include <Q-Tip/Window/Scancode.h>

#include <algorithm>

#include <QTipUIKit/UIKit.h>

using namespace QTip;

int main() {
    ModLoader::load<UIKitMod>();

    Window window("QTipUIKit ProgressBar Test", 800, 400);

    ProgressBarStyle blueStyle;
    blueStyle.trackColor = Color{45, 45, 45, 255};
    blueStyle.progressColor = Color{80, 160, 255, 255};
    blueStyle.cornerRadius = 8;

    ProgressBarStyle greenStyle;
    greenStyle.trackColor = Color{45, 45, 45, 255};
    greenStyle.progressColor = Color{100, 210, 140, 255};
    greenStyle.cornerRadius = 4;

    ProgressBar interactiveBar({100, 120, 600, 32}, blueStyle);
    ProgressBar halfBar({100, 200, 600, 20}, greenStyle);
    ProgressBar fullBar({100, 260, 600, 12}, greenStyle);

    interactiveBar.setProgress(0.35f);
    halfBar.setProgress(0.5f);
    fullBar.setProgress(1.0f);

    while (!window.shouldClose()) {
        QTipRuntime::fetchEvents();

        if (window.input().keyWasPressed(Scancode::RIGHT)) {
            interactiveBar.setProgress(interactiveBar.progress() + 0.05f);
        }

        if (window.input().keyWasPressed(Scancode::LEFT)) {
            interactiveBar.setProgress(interactiveBar.progress() - 0.05f);
        }

        if (window.input().keyWasPressed(Scancode::SPACE)) {
            interactiveBar.setProgress(0.0f);
        }

        window->setRenderColor(Color{24, 24, 24, 255});
        window->clear();

        interactiveBar.render(window);
        halfBar.render(window);
        fullBar.render(window);

        window->present();
    }

    return 0;
}
