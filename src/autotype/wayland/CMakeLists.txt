find_package(KF5GlobalAccel CONFIG)
set_package_properties(KF5GlobalAccel PROPERTIES TYPE OPTIONAL)
add_feature_info("KF5GlobalAccel" KF5GlobalAccel_FOUND "Required for auto-type on KWin Wayland")

find_package(KF5Wayland CONFIG)
set_package_properties(KF5Wayland PROPERTIES TYPE OPTIONAL)
add_feature_info("KF5Wayland" KF5Wayland_FOUND "Required for auto-type on KWin Wayland")

find_package(XKBCommon)
set_package_properties(XKBCommon PROPERTIES TYPE OPTIONAL)
add_feature_info("XKBCommon" XKBCommon_FOUND "Required for auto-type on KWin Wayland")

if(KF5GlobalAccel_FOUND AND KF5Wayland_FOUND AND XKBCommon_FOUND)
    set(autotype_KWinWayland_SOURCES AutoTypeWayland.cpp KWinWaylandPlatform.cpp KeyboardState.cpp)

    add_library(keepassx-autotype-wayland MODULE ${autotype_KWinWayland_SOURCES})
    target_link_libraries(keepassx-autotype-wayland
        Qt5::Core
        Qt5::Gui
        KF5::WaylandClient
        KF5::GlobalAccel
        ${XKBCommon_LIBRARIES}
    )
    target_include_directories(keepassx-autotype-wayland PRIVATE ${XKBCommon_INCLUDE_DIRS})
    install(TARGETS keepassx-autotype-wayland
            BUNDLE DESTINATION . COMPONENT Runtime
            LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR} COMPONENT Runtime)
endif()
