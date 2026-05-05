SET(SOURCES screens/ui_LedCtrlTest.c
    screens/ui_ModebusTest.c
    screens/ui_SdCardAndFatFsTest.c
    ui.c
    components/ui_comp_hook.c
    ui_helpers.c
    ui_events.c)

nuttx_add_library(ui ${SOURCES})
