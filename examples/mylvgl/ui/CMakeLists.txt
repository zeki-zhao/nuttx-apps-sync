SET(SOURCES screens/ui_HomeScreen.c
    screens/ui_Screen1.c
    screens/ui_Screen2.c
    screens/ui_Screen3.c
    screens/ui_Terminal.c
    ui.c
    components/ui_comp_hook.c
    ui_helpers.c
    ui_events.c)

nuttx_add_library(ui ${SOURCES})
target_include_directories(ui PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
