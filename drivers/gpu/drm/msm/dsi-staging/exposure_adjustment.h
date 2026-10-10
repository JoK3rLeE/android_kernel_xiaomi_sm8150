/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * OLED exposure adjustment using Qualcomm DSPP PCC.
 * Device-specific values are based on the PixelOS Cepheus tuning.
 */
#ifndef EXPOSURE_ADJUSTMENT_H
#define EXPOSURE_ADJUSTMENT_H

#include <linux/types.h>

/* Minimum hardware backlight level used to avoid the low-brightness ELVSS range. */
#define ELVSS_OFF_THRESHOLD        1024U
#define EXPOSURE_ADJUSTMENT_MIN    580U
#define EXPOSURE_ADJUSTMENT_MAX    32768U
#define PCC_BACKLIGHT_SCALE ((EXPOSURE_ADJUSTMENT_MAX - EXPOSURE_ADJUSTMENT_MIN) / ELVSS_OFF_THRESHOLD)

struct dsi_panel;

void ea_panel_mode_ctrl(struct dsi_panel *panel, bool enable);
bool ea_panel_is_enabled(void);
u32 ea_panel_calc_backlight(u32 bl_lvl);

#endif /* EXPOSURE_ADJUSTMENT_H */
