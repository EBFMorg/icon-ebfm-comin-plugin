// SPDX-FileCopyrightText: 2026 The ICON EBFM ComIn Plugin Authors
//
// SPDX-License-Identifier: BSD-3-Clause

// This plugin re-implements, as a ComIn plugin, the atmosphere->EBFM
// coupling that ICON's `feature_icono2elmer` branch currently implements
// natively (guarded by `coupling_nml :: coupled_to_ebfm`). See:
//   - icon/src/coupling/mo_atmo_ebfm_coupling.f90
//       (construct_atmo_ebfm_coupling, couple_atmo_to_ebfm)
//   - icon/src/coupling/mo_atmo_coupling_frame.f90:137-232
//       (grid_name = "icon_atmos_grid" at line 140; DEFCOMP_BEFORE/AFTER and
//       the construct_atmo_ebfm_coupling call all sit in this window, which
//       is why we mirror that ordering below)
//   - icon/src/atm_dyn_iconam/mo_nh_stepping.f90:1332-1340
//       (calls couple_atmo_to_ebfm right before EP_ATM_INTEGRATE_AFTER fires,
//       which is why we put/send on that same entry point below; timestep
//       used there is time_config%tc_dt_model, i.e. the same value
//       comin_descrdata_get_timesteplength() returns)
//   - ebfm_dummy/config/coupling_icon_atmo.yaml and
//     ebfm_dummy/dummies/ICON/src/icon_dummy.c
//       (the actual YAC coupling config + a standalone C stand-in for ICON's
//       side of it; ground truth for which fields are really exchanged)
// Grid/points lookup follows
// comin/plugins/python_adapter/examples/yac_example.py on the
// 166-yac-named-points ComIn branch.

#include "comin.h"
#include "ebfm_fields.h"

#include <yac.h>

#include <stddef.h>
#include <stdio.h>

#define ICON_ATMO_CELL_POINTS_NAME "cell_centers"

static t_comin_var_handle comin_var_handles[N_EBFM_FIELDS];

static int ebfm_comp_id;
static int ebfm_field_ids[N_EBFM_FIELDS];

void secondary_constructor(void);
void defcomp(void);
void define_fields(void);
void put_fields_to_ebfm(void);
void finalize(void);

void comin_main(void) {
    comin_print_info("Hello ComIn!");
    comin_callback_register(EP_SECONDARY_CONSTRUCTOR, secondary_constructor);
    comin_callback_register(EP_ATM_YAC_DEFCOMP_BEFORE, defcomp);
    comin_callback_register(EP_ATM_YAC_DEFCOMP_AFTER, define_fields);
    comin_callback_register(EP_ATM_INTEGRATE_AFTER, put_fields_to_ebfm);
    comin_callback_register(EP_DESTRUCTOR, finalize);
}

// Requests read access to all fields ICON already provides (see
// EBFM_FIELDS). Requires ICON to run with AES physics, since most of these
// fields are only diagnosed there (see README "Prerequisites").
void secondary_constructor(void) {
    t_comin_entry_point eps[] = {EP_ATM_INTEGRATE_AFTER};
    for (size_t i = 0; i < N_EBFM_FIELDS; ++i) {
        t_comin_var_descriptor descr = {"", EBFM_DOMAIN_ID};
        snprintf(descr.name, sizeof(descr.name), "%s",
                 EBFM_FIELDS[i].icon_var_name);
        comin_var_handles[i] = comin_var_get(1, eps, &descr, COMIN_FLAG_READ);
    }
}

// Define YAC component "ebfm_comin" for this plugin
void defcomp(void) {
    int instance_id = comin_descrdata_get_global_yac_instance_id();
    if (instance_id == -1) {
        comin_plugin_finish_f(
            EBFM_COMIN_COMPONENT_NAME,
            "Host model is not configured with YAC (yac_instance_id == -1)");
    }
    yac_cpredef_comp_instance(
        instance_id,
        EBFM_COMIN_COMPONENT_NAME,
        &ebfm_comp_id);
}

// Looks up ICON's own grid/points by name (rather than rebuilding a grid
// from comin_descrdata_get_domain_cells_*/verts_*, as yac_input_plugin.c
// does) and defines one YAC field per entry in EBFM_FIELDS on top of it.
// Field timestep mirrors mo_atmo_coupling_frame.f90's `timestepstring`
// (= time_config%tc_dt_model, the model's base dynamics timestep) -- the
// actual atmo->EBFM exchange cadence (currently PT3H, see
// ebfm_dummy/config/coupling_icon_atmo.yaml) is a property of the
// component-to-component coupling definition, not of this field definition.
void define_fields(void) {
    int grid_id = yac_cget_grid_id(ICON_ATMO_GRID_NAME);
    int point_id =
        yac_cget_points_id(
            grid_id, YAC_LOCATION_CELL, ICON_ATMO_CELL_POINTS_NAME);

    double dt = comin_descrdata_get_timesteplength(EBFM_DOMAIN_ID);
    char dt_str[16];
    snprintf(dt_str, sizeof(dt_str), "%d", (int)(dt * 1000));

    for (size_t i = 0; i < N_EBFM_FIELDS; ++i) {
        comin_print_info_f("yac_cdef_field field %s", EBFM_FIELDS[i].yac_field_name);
        yac_cdef_field(EBFM_FIELDS[i].yac_field_name, ebfm_comp_id, &point_id,
                       1, 1, dt_str, YAC_TIME_UNIT_MILLISECOND,
                       &ebfm_field_ids[i]);
    }
}

// Mirrors couple_atmo_to_ebfm() in mo_atmo_ebfm_coupling.f90:167-261, called
// at the same point in the time loop (mo_nh_stepping.f90:1334-1340, right
// before EP_ATM_INTEGRATE_AFTER). Fields not currently active in
// ebfm_dummy/config/coupling_icon_atmo.yaml (see EBFM_FIELDS) are still put
// here -- an unmatched, undefined-coupling field is harmless on the source
// side, it simply has no listener.
void put_fields_to_ebfm(void) {
    for (size_t i = 0; i < N_EBFM_FIELDS; ++i) {
        double* ptr = comin_var_get_ptr_double(comin_var_handles[i]);
        int info, ierror;
        comin_print_info_f("yac_cput_ field %s", EBFM_FIELDS[i].yac_field_name);
        yac_cput_(ebfm_field_ids[i], 1, ptr, &info, &ierror);
        if (ierror != 0) {
            comin_plugin_finish_f(
                EBFM_COMIN_COMPONENT_NAME,
                "yac_cput_ failed for '%s'",
                EBFM_FIELDS[i].yac_field_name);
        }
    }
}

void finalize(void) {
    comin_print_info("See you later ComInator!");
}
