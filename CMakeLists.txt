cmake_minimum_required(VERSION 4.0)

project(QTipUIKit
    VERSION 0.0.3
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(QTip REQUIRED)
find_package(SDL3 REQUIRED)
find_package(magic_enum REQUIRED)

include(GNUInstallDirs)

add_library(QTipUIKit STATIC
        include/QTipUIKit/UIObject.h
        include/QTipUIKit/Widgets/Textbox/Textbox.h
        src/Widgets/Textbox/Textbox.cpp
        include/QTipUIKit/UIKit.h
        src/UIKit.cpp
        src/Widgets/Button/Button.cpp
        include/QTipUIKit/Widgets/Button/Button.h
        include/QTipUIKit/Internal/Helpers.h
        include/QTipUIKit/UIKitMod.h
        include/QTipUIKit/Widgets/Checkbox/Checkbox.h
        src/Widgets/Checkbox/Checkbox.cpp
        src/Widgets/Label/Label.cpp
        include/QTipUIKit/Widgets/Label/Label.h
        src/Containers/Panel.cpp
        include/QTipUIKit/Containers/Panel.h
        src/Internal/Helpers.cpp
        include/QTipUIKit/Containers/HBox.h
        src/Internal/Box.cpp
        include/QTipUIKit/Internal/Box.h
        include/QTipUIKit/Containers/VBox.h
        src/Containers/ScrollView.cpp
        include/QTipUIKit/Containers/ScrollView.h
        src/Widgets/ScrollBar/ScrollBar.cpp
        include/QTipUIKit/Widgets/ScrollBar/ScrollBar.h
        src/Dialogs/Dialog.cpp
        include/QTipUIKit/Dialogs/Dialog.h
        include/QTipUIKit/Widgets/Dropdown/Dropdown.h
        include/QTipUIKit/Widgets/Dropdown/Dropdown.tpp
        src/Containers/List.cpp
        include/QTipUIKit/Containers/List.h
        include/QTipUIKit/Widgets/PieChart/PieChart.h
        include/QTipUIKit/Widgets/PieChart/PieChart.tpp
        src/UIContext.cpp
        include/QTipUIKit/UIContext.h
        src/UIObject.cpp
        include/QTipUIKit/Widgets/BarChart/BarChart.tpp
        include/QTipUIKit/Widgets/BarChart/BarChart.h
        src/Widgets/Button/MagickButton.cpp
        include/QTipUIKit/Widgets/Button/MagickButton.h
        src/Containers/NavigationView/View.cpp
        include/QTipUIKit/Containers/NavigationView/View.h
        src/Containers/NavigationView/NavigationView.cpp
        include/QTipUIKit/Containers/NavigationView/NavigationView.h
)

add_library(QTipUIKit::QTipUIKit ALIAS QTipUIKit)

target_include_directories(QTipUIKit
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(QTipUIKit
    PUBLIC
        QTip::QTip
        magic_enum::magic_enum
    PRIVATE
        SDL3::SDL3
)

include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/QTipUIKitConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/QTipUIKitConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/QTipUIKit
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/QTipUIKitConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/QTipUIKitConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/QTipUIKitConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/QTipUIKit
)

add_executable(QTipUIKitTest
    test.cpp
)

target_link_libraries(QTipUIKitTest PUBLIC
    QTipUIKit::QTipUIKit
)

add_executable(QTipUIKitTest2
    test2.cpp
)

target_link_libraries(QTipUIKitTest2 PUBLIC
    QTipUIKit::QTipUIKit
)

install(
    TARGETS QTipUIKit
    EXPORT QTipUIKitTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(
    DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    EXPORT QTipUIKitTargets
    FILE QTipUIKitTargets.cmake
    NAMESPACE QTipUIKit::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/QTipUIKit
)
