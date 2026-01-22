add_library(blossomuicommon${QT_MAJOR_VERSION} OBJECT blossomuiboxshadowrenderer.cpp)

set_target_properties(blossomuicommon${QT_MAJOR_VERSION} PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

target_link_libraries(blossomuicommon${QT_MAJOR_VERSION}
    PUBLIC
        Qt${QT_MAJOR_VERSION}::Core
        Qt${QT_MAJOR_VERSION}::Gui)
