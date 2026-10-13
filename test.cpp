#include <Q-Tip/QTip.h>
#include "QTipUIKit/UIKit.h"
#include "QTipUIKit/Containers/ScrollView.h"
#include "QTipUIKit/Dialogs/Dialog.h"
#include "QTipUIKit/Widgets/BarChart/BarChart.h"
#include "QTipUIKit/Widgets/Dropdown/Dropdown.h"
#include "QTipUIKit/Widgets/PieChart/PieChart.h"

using namespace QTip;

namespace {
    enum class [[maybe_unused]] Fruit {
        Apple,
        Apricot,
        Avocado,
        Banana,
        Blackberry,
        Blueberry,
        Cantaloupe,
        Cherry,
        Coconut,
        Cranberry,
        Date,
        Dragonfruit,
        Durian,
        Fig,
        Gooseberry,
        Grape,
        Grapefruit,
        Guava,
        Jackfruit,
        Kiwi,
        Lemon,
        Lime,
        Lychee,
        Mango,
        Mangosteen,
        Melon,
        Mulberry,
        Nectarine,
        Orange,
        Papaya,
        Passionfruit,
        Peach,
        Pear,
        Persimmon,
        Pineapple,
        Plum,
        Pomegranate,
        Raspberry,
        Starfruit,
        Strawberry,
        Tangerine,
        Watermelon
    };
}

int main() {
    ModLoader::load<UIKitMod>();

    Window window("QTipUIKit Test", 800, 600);

    Font font(Detail::defaultFontPath(), 20);

    ScrollViewSettings settings;
    settings.direction = ScrollDirection::Both;

    ScrollView view({0, 0, 800, 600}, settings);
    view.setContentSize({1920, 1080});

    auto& vbox = view.add<VBox>(Rect{10, 10, 1900, 1060});
    vbox.setSpacing(10);
    vbox.setSizing(Sizing::Stretch);
    vbox.setCrossSizing(CrossSizing::Stretch);

    auto& textbox = vbox.add<Textbox>(
        Rect{20, 20, 740, 250},
        font
    );

    auto& hbox = vbox.add<HBox>(Rect{20, 500, 760, 60});
    hbox.setSpacing(10);
    hbox.setSizing(Sizing::Stretch);
    hbox.setCrossSizing(CrossSizing::Stretch);

    Point windowSize = window.size();
    view.resize(windowSize);

    vbox.layout();

    ButtonStyle buttonStyle;
    buttonStyle.font = font;
    buttonStyle.color = Color{50, 50, 50, 255};
    buttonStyle.hoverColor = Color{70, 70, 70, 255};
    buttonStyle.pressedColor = Color{30, 30, 30, 255};
    buttonStyle.disabledColor = Color{20, 20, 20, 255};
    buttonStyle.fontColor = Color::white;

    auto& button = hbox.add<Button>(
        Rect{20, 500, 60, 60},
        "Click me",
        buttonStyle
    );

    CheckboxStyle checkboxStyle;
    checkboxStyle.color = Color{50, 50, 50, 255};
    checkboxStyle.hoverColor = Color{70, 70, 70, 255};
    checkboxStyle.pressedColor = Color{30, 30, 30, 255};
    checkboxStyle.disabledColor = Color{20, 20, 20, 255};

    auto& checkbox = hbox.add<Checkbox>(Rect{240, 520, 20, 20}, checkboxStyle);
    checkbox.checked = true;

    auto& checkboxLabel = hbox.add<Label>("Button Enabled", Color::white, Point{270, 520}, font);

    auto& dialogButton = hbox.add<Button>(
        Rect{20, 580, 60, 60},
        "Dialog",
        buttonStyle
    );

    auto& pieChartButton = hbox.add<Button>(
        Rect{20, 580, 60, 60},
        "PieChart",
        buttonStyle
    );

    button.setOnClick([&] {
        textbox.setText("Button clicked!");
        checkbox.checked = false;
    });

    dialogButton.setOnClick([&] {
        auto& dialog = Dialog::create(window);

        auto& buttons = dialog.buttons();
        auto& content = dialog.content();

        auto& contentVBox = content.add<VBox>(Rect{0, 0, content.rect().size});

        contentVBox.add<Label>("Quit?", Color::white, Point{0, 0}, font);

        contentVBox.add<Dropdown<Fruit>>(Rect{0, 0, 100, 25});

        dialog.setContentResizeHandler([&](Point size) {
            contentVBox.resize(size);
        });

        auto& cancelButton = buttons.add<Button>(Rect{0, 0, 100, buttons.rect().size.y}, "Cancel", buttonStyle);
        auto& yesButton = buttons.add<Button>(Rect{0, 0, 100, buttons.rect().size.y}, "Yes", buttonStyle);

        yesButton.setOnClick([&] {
            dialog.close();
            window.requestClose();
        });

        cancelButton.setOnClick([&] {
            dialog.close();
        });
    });

    pieChartButton.setOnClick([&] {
        auto& dialog = Dialog::create(window);

        auto& buttons = dialog.buttons();
        auto& content = dialog.content();

        auto& contentVBox = content.add<VBox>(
            Rect{0, 0, content.rect().size}
        );

        contentVBox.setSizing(Sizing::Stretch);
        contentVBox.setCrossSizing(CrossSizing::Stretch);

        auto& pieChart = contentVBox.add<PieChart<int>>(
            Rect{0, 0, 100, 100},
            std::vector{
                1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
                11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
                21, 22, 23, 24, 25, 26, 27, 28, 29, 30
            },
            PieChartFormatters<int>{
                .value = [](const int& value) {
                    return value;
                },
                .label = [](const int& value) {
                    return std::to_string(value);
                }
            },
            true
        );

        dialog.setContentResizeHandler([&](Point size) {
            contentVBox.resize(size);
        });

        auto& randomizeButton = buttons.add<Button>(
            Rect{0, 0, 120, buttons.rect().size.y},
            "Randomize",
            buttonStyle
        );

        randomizeButton.setOnClick([&] {
            std::vector<int> values;
            values.reserve(30);

            for (int i = 0; i < 30; ++i) {
                values.push_back(1 + (std::rand() % 100));
            }

            pieChart.setData(
                std::move(values),
                PieChartFormatters<int>{
                    .value = [](const int& value) {
                        return value;
                    },
                    .label = [](const int& value) {
                        return std::to_string(value);
                    }
                }
            );
        });

        auto& okButton = buttons.add<Button>(
            Rect{0, 0, 100, buttons.rect().size.y},
            "Ok",
            buttonStyle
        );

        okButton.setOnClick([&] {
            dialog.close();
        });

        auto& barChartButton = buttons.add<Button>(
            Rect{0, 0, 100, buttons.rect().size.y},
            "BarChart",
            buttonStyle
        );

        barChartButton.setOnClick([&window, buttonStyle] {
            auto& dialog = Dialog::create(window);

            auto& buttons = dialog.buttons();
            auto& content = dialog.content();

            auto& contentVBox = content.add<VBox>(
                Rect{0, 0, content.rect().size}
            );

            contentVBox.setSizing(Sizing::Stretch);
            contentVBox.setCrossSizing(CrossSizing::Stretch);

            auto& barChart = contentVBox.add<BarChart<int>>(
                Rect{0, 0, 100, 100},
                std::vector{
                    1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
                    11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
                    21, 22, 23, 24, 25, 26, 27, 28, 29, 30
                },
                BarChartFormatters<int>{
                    .value = [](int& value) {
                        return value;
                    },
                    .label = [](int& value) {
                        return std::to_string(value);
                    }
                },
                true
            );

            dialog.setContentResizeHandler([&](Point size) {
                contentVBox.resize(size);
            });

            auto& randomizeButton = buttons.add<Button>(
                Rect{0, 0, 120, buttons.rect().size.y},
                "Randomize",
                buttonStyle
            );

            randomizeButton.setOnClick([&barChart] {
                std::vector<int> values;
                values.reserve(30);

                for (int i = 0; i < 30; ++i) {
                    values.push_back((1 + (std::rand() % 100)) - 50);
                }

                barChart.setData(
                    std::move(values),
                    BarChartFormatters<int>{
                        .value = [](int& value) {
                            return value;
                        },
                        .label = [](int& value) {
                            return std::to_string(value);
                        }
                    }
                );
            });

            auto& okButton = buttons.add<Button>(
                Rect{0, 0, 100, buttons.rect().size.y},
                "Ok",
                buttonStyle
            );

            okButton.setOnClick([&dialog] {
                dialog.close();
            });
        });
    });

    while (!window.shouldClose()) {
        QTipRuntime::pollEvents();

        if (window.input().keyWasPressed(Key::Key_ESCAPE))
            checkbox.checked = true;

        if (window.size() != windowSize) {
            windowSize = window.size();

            view.resize(windowSize);
        }

        button.setDisabled(!checkbox.checked); // disable if NOT checked

        if (checkbox.checked)
            checkboxLabel.setText("Button Enabled");
        else
            checkboxLabel.setText("Button Disabled");

        window->setRenderColor(Color::black);
        window->clear();

        view.render(window);

        window->present();
    }

    return 0;
}
