cmake_minimum_required(VERSION 3.16)

project(ImageMVVM VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Quick QuickControls2)

qt_standard_project_setup()

qt_add_executable(ImageMVVMApp
    main.cpp ImageItem.h Image.h ViewModel.h ViewModel.cpp
)

qt_add_resources(ImageMVVMApp "qml"
    PREFIX "/"
    FILES
        Main.qml
)

target_link_libraries(ImageMVVMApp PRIVATE
    Qt6::Core
    Qt6::Quick
    Qt6::QuickControls2
)

# Copier les fichiers QML pour le développement
set_target_properties(ImageMVVMApp PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.ImageMVVM
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)
