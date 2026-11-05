#include <Q-Tip/QTip.h>
#include <QTipUIKit/UIKit.h>

using namespace QTip;

int main() {
    ModLoader::load<UIKitMod>();

    Window window("QTipUIKit ProgressBar Test", 800, 600);

    ProgressBarStyle style;
    style.trackColor = Color{45, 45, 45, 255};
    style.progressColor = Color{90, 170, 255, 255};
    style.cornerRadius = 8;

    ProgressBar empty({100, 100, 500, 24}, style);
    ProgressBar quarter({100, 160, 500, 24}, style);
    ProgressBar halfway({100, 220, 500, 24}, style);
    ProgressBar nearlyComplete({100, 280, 500, 24}, style);
    ProgressBar complete({100, 340, 500, 24}, style);

    empty.setProgress(0.0f);
    quarter.setProgress(0.25f);
    halfway.setProgress(0.5f);
    nearlyComplete.setProgress(0.9f);
    complete.setProgress(1.0f);

    while (!window.shouldClose()) {
        QTipRuntime::fetchEvents();

        window->setRenderColor(Color{24, 24, 24, 255});
        window->clear();

        empty.render(window);
        quarter.render(window);
        halfway.render(window);
        nearlyComplete.render(window);
        complete.render(window);

        window->present();
    }

    return 0;
}
