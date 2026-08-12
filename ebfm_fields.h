// SPDX-FileCopyrightText: 2026 The ICON EBFM ComIn Plugin Authors
//
// SPDX-License-Identifier: BSD-3-Clause

#ifndef EBFM_FIELDS_H
#define EBFM_FIELDS_H

#include <stddef.h>

// Maps an ICON/ComIn variable name to the field name used on the EBFM side
// of the coupling. Names taken from mo_atmo_ebfm_coupling.f90:64-116 (define
// side) and mo_nh_stepping.f90:1336-1338 (data source side) on ICON's
// `feature_icono2elmer` branch, cross-checked against
// ebfm_dummy/config/coupling_icon_atmo.yaml (the actual YAC coupling config
// used to run ICON<->EBFM today).
//
// Shared between main.c (real plugin) and test/fake.c (replay-test stand-in
// for the ICON fields this plugin reads) so the two can't drift apart.
typedef struct {
    const char* icon_var_name; // name to request via comin_var_get
    const char* yac_field_name; // name to register via yac_cdef_field
    // `active` mirrors whether the field is currently enabled in
    // ebfm_dummy/config/coupling_icon_atmo.yaml (7 of 9 are; huss/sfcpres
    // are commented out there). Inactive fields are still read from ICON
    // and defined as YAC fields below so turning them on is a config-only
    // change, but nothing currently consumes them on the EBFM side.
    int active;
    // TODO: units / CF standard_name, once agreed with the EBFM side
    // (mo_atmo_ebfm_coupling.f90 itself only has "some metadata"
    // placeholders for all 9 fields; nothing authoritative to copy yet).
} t_ebfm_field;

enum EBFM_FIELD_IDS {
    TAS_FIELD_ID,
    PRLS_FIELD_ID,
    PRLR_FIELD_ID,
    SFCWIND_FIELD_ID,
    RSDS_FIELD_ID,
    RLDS_FIELD_ID,
    CLT_FIELD_ID,
    PRES_SFC_FIELD_ID,
    HUSS_FIELD_ID,
    N_EBFM_FIELDS,  // total number of fields
};

static const t_ebfm_field EBFM_FIELDS[] = {
    {"tas",      "tas",      1}, // 2m temperature
    {"prls",     "pr_snow",  1}, // snowfall flux (prm_field%ssfl)
    {"prlr",     "pr",       1}, // liquid precipitation flux (prm_field%rsfl)
    {"sfcwind",  "sfcwind",  1}, // 10m wind speed
    {"rsds",     "rsds",     1}, // surface downward shortwave radiation
    {"rlds",     "rlds",     1}, // surface downward longwave radiation
    {"clt",      "clt",      1}, // total cloud cover fraction (prm_field%aclcov)
    // TODO: coupling_icon_atmo.yaml's commented-out entry for this field is
    // named "sfcPressure", not "sfcpres" as used here and in
    // mo_atmo_ebfm_coupling.f90. Resolve with the EBFM/coupling maintainers
    // before enabling it, or the yac_cdef_couple() on the EBFM side simply
    // won't find a match.
    {"pres_sfc", "sfcpres",  0}, // surface pressure (p_nh_state%diag%pres_sfc)
    {"qv2m",     "huss",     0}, // 2m specific humidity
};

// Catches EBFM_FIELD_IDS drifting out of sync with EBFM_FIELDS (e.g. a field
// added to one but not the other).
_Static_assert(sizeof(EBFM_FIELDS) / sizeof(EBFM_FIELDS[0]) == N_EBFM_FIELDS,
               "EBFM_FIELD_IDS must have exactly one entry per EBFM_FIELDS row");

// TODO: confirm domain id; native code only supports jg=1 for now
// (mo_nh_stepping.f90:1335).
#define EBFM_DOMAIN_ID 1

// Names main.c registers this plugin's YAC component under, and the ICON
// grid it borrows its points from. Shared with test/receiver.c so the two
// can't drift apart.
#define EBFM_COMIN_COMPONENT_NAME "ebfm_comin"
#define ICON_ATMO_GRID_NAME "icon_atmos_grid"

#endif // EBFM_FIELDS_H
