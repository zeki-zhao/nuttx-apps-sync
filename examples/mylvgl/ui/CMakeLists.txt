SET(SOURCES screens/ui_HomeScreen.c
    screens/ui_DeviceCtrl.c
    screens/ui_ModbusSlave.c
    screens/ui_FileEdit.c
    screens/ui_Terminal.c
    screens/ui_FileExplorer.c
    screens/ui_MusicPlayer.c
    ui.c
    ui_helpers.c
    ui_events.c)

nuttx_add_library(ui ${SOURCES})
target_include_directories(ui PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
