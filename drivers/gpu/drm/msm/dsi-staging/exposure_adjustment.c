// SPDX-License-Identifier: GPL-2.0-only
/*
 * OLED exposure adjustment using Qualcomm DSPP PCC.
 * Based on PixelOS Cepheus implementation, with defensive checks added.
 */
#include <linux/err.h>
#include <linux/kernel.h>
#include <drm/drm_property.h>
#include <drm/msm_drm_pp.h>

#include "dsi_display.h"
#include "dsi_panel.h"
#include "../sde/sde_crtc.h"
#include "../sde/sde_plane.h"
#include "exposure_adjustment.h"

static bool pcc_backlight_enable;
static u32 last_level = ELVSS_OFF_THRESHOLD;

static int ea_panel_send_pcc(u32 bl_lvl)
{
	struct dsi_display *display;
	struct drm_crtc *crtc;
	struct drm_msm_pcc pcc_blk = { 0 };
	struct drm_property *prop;
	struct drm_property_blob *blob;
	struct msm_drm_private *priv;
	u32 ea_coeff;
	int rc;

	display = get_main_display();
	if (!display || !display->drm_conn || !display->drm_conn->state) {
		pr_err("exposure adjustment: display/connector state unavailable\n");
		return -ENODEV;
	}

	crtc = display->drm_conn->state->crtc;
	if (!crtc || !crtc->dev || !crtc->dev->dev_private) {
		pr_err("exposure adjustment: CRTC unavailable\n");
		return -ENODEV;
	}

	priv = crtc->dev->dev_private;
	prop = priv->cp_property[1]; /* SDE_CP_CRTC_DSPP_PCC */
	if (!prop) {
		pr_err("exposure adjustment: DSPP PCC property unavailable\n");
		return -EOPNOTSUPP;
	}

	if (bl_lvl < ELVSS_OFF_THRESHOLD)
		ea_coeff = bl_lvl * PCC_BACKLIGHT_SCALE + EXPOSURE_ADJUSTMENT_MIN;
	else
		ea_coeff = EXPOSURE_ADJUSTMENT_MAX;

	pcc_blk.r.r = ea_coeff;
	pcc_blk.g.g = ea_coeff;
	pcc_blk.b.b = ea_coeff;

	blob = drm_property_create_blob(crtc->dev, sizeof(pcc_blk), &pcc_blk);
	if (IS_ERR_OR_NULL(blob)) {
		pr_err("exposure adjustment: failed to create PCC blob\n");
		return blob ? PTR_ERR(blob) : -ENOMEM;
	}

	rc = sde_cp_crtc_set_property(crtc, prop, blob->base.id);
	if (rc)
		pr_err("exposure adjustment: failed to apply PCC: %d\n", rc);
	drm_property_blob_put(blob);

	return rc;
}

bool ea_panel_is_enabled(void)
{
	return pcc_backlight_enable;
}

void ea_panel_mode_ctrl(struct dsi_panel *panel, bool enable)
{
	if (!panel)
		return;

	if (pcc_backlight_enable != enable) {
		pcc_backlight_enable = enable;
		dsi_panel_set_backlight(panel, last_level);
		if (!enable)
			ea_panel_send_pcc(ELVSS_OFF_THRESHOLD);
	} else if (!last_level && !pcc_backlight_enable) {
		ea_panel_send_pcc(ELVSS_OFF_THRESHOLD);
	}
}

u32 ea_panel_calc_backlight(u32 bl_lvl)
{
	last_level = bl_lvl;

	if (pcc_backlight_enable && bl_lvl && bl_lvl < ELVSS_OFF_THRESHOLD) {
		if (ea_panel_send_pcc(bl_lvl))
			pr_err("exposure adjustment: failed to update PCC for level %u\n",
			       bl_lvl);
		return ELVSS_OFF_THRESHOLD;
	}

	return bl_lvl;
}
