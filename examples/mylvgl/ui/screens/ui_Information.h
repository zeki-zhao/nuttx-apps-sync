#ifndef UI_INFORMATION_H
#define UI_INFORMATION_H

#ifdef __cplusplus
extern "C" {
#endif

extern void ui_Information_screen_init(void);
extern void ui_Information_screen_destroy(void);
extern lv_obj_t *ui_Information;
extern lv_obj_t *ui_labelVersion;
extern lv_obj_t *ui_labelUptime;
extern lv_obj_t *ui_labelUptimePrefix;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* UI_INFORMATION_H */
