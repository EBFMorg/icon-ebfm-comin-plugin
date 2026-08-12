// SPDX-FileCopyrightText: 2026 The ICON EBFM ComIn Plugin Authors
//
// SPDX-License-Identifier: BSD-3-Clause

// Standalone YAC application that receives every field ebfm_comin (main.c)
// puts, and prints the values it gets. A smoke-test harness for the plugin,
// not a stand-in for the real EBFM model: it defines its own throwaway
// component/grid and couples explicitly against main.c's already-registered
// component/grid/fields, mirroring
// comin/plugins/yac_input/yac_input_external.c. The real EBFM-side coupling
// (different component/grid names, driven by a YAML config) lives in
// ebfm_dummy/dummies/EBFM/src/coupling.py and
// ebfm_dummy/config/coupling_icon_atmo.yaml.

#include "../ebfm_fields.h"
#include "fake_values.h"

#include <yac.h>
#include <yac_config.h>

#include <math.h>
#include <stddef.h>
#include <stdio.h>

#define EBFM_RECEIVER_COMPONENT_NAME "ebfm_comin_receiver"
#define EBFM_RECEIVER_GRID_NAME "ebfm_comin_receiver_grid"

#define RECEIVER_NLON 2
#define RECEIVER_NLAT 2
#define RECEIVER_N_POINTS (RECEIVER_NLON * RECEIVER_NLAT)

// Arbitrary small quad (10 deg on a side); adjust to fall within whatever
// ICON domain you're testing against.
static const double receiver_lon_deg[RECEIVER_NLON] = {0.0, 10.0};
static const double receiver_lat_deg[RECEIVER_NLAT] = {40.0, 50.0};

int main(void) {
    yac_cinit();

    int comp_id;
    yac_cdef_comp(EBFM_RECEIVER_COMPONENT_NAME, &comp_id);

    double x_vertices[RECEIVER_NLON], y_vertices[RECEIVER_NLAT];
    for (int i = 0; i < RECEIVER_NLON; ++i) {
        x_vertices[i] = receiver_lon_deg[i] * M_PI / 180.0;
    }
    for (int i = 0; i < RECEIVER_NLAT; ++i) {
        y_vertices[i] = receiver_lat_deg[i] * M_PI / 180.0;
    }
    int dims[]   = {RECEIVER_NLON, RECEIVER_NLAT};
    int cyclic[] = {0, 0};
    int grid_id;
    yac_cdef_grid_reg2d(EBFM_RECEIVER_GRID_NAME, dims, cyclic, x_vertices,
                        y_vertices, &grid_id);

    int point_id;
    yac_cdef_points_reg2d(grid_id, dims, YAC_LOCATION_CORNER, x_vertices,
                          y_vertices, &point_id);

    yac_csync_def();

    // Query each field's timestep from ebfm_comin's already-registered side
    // rather than hard-coding it here, so the two can't drift apart.
    int field_ids[N_EBFM_FIELDS];
    char dt_strs[N_EBFM_FIELDS][32];
    for (size_t i = 0; i < N_EBFM_FIELDS; ++i) {
        const char* dt = yac_cget_field_timestep(
            EBFM_COMIN_COMPONENT_NAME, ICON_ATMO_GRID_NAME,
            EBFM_FIELDS[i].yac_field_name);
        snprintf(dt_strs[i], sizeof(dt_strs[i]), "%s", dt);
        yac_cdef_field(EBFM_FIELDS[i].yac_field_name, comp_id, &point_id, 1, 1,
                       dt_strs[i], YAC_TIME_UNIT_ISO_FORMAT, &field_ids[i]);
    }

    int interp_id;
    yac_cget_interp_stack_config(&interp_id);
#if YAC_VERSION_MAJOR == 3 && YAC_VERSION_MINOR <= 3
    yac_cadd_interp_stack_config_nnn(interp_id, YAC_NNN_AVG, 1, 1.0);
#else
    yac_cadd_interp_stack_config_nnn(interp_id, YAC_NNN_AVG, 1, 0.0, 1.0);
#endif

    for (size_t i = 0; i < N_EBFM_FIELDS; ++i) {
        yac_cdef_couple(EBFM_COMIN_COMPONENT_NAME, ICON_ATMO_GRID_NAME,
                        EBFM_FIELDS[i].yac_field_name,
                        EBFM_RECEIVER_COMPONENT_NAME, EBFM_RECEIVER_GRID_NAME,
                        EBFM_FIELDS[i].yac_field_name, dt_strs[i],
                        YAC_TIME_UNIT_ISO_FORMAT, YAC_REDUCTION_TIME_NONE,
                        interp_id, 1, 0);
    }
    yac_cfree_interp_stack_config(interp_id);

    yac_cenddef();

    // Neither main.c nor comin_replay register a shared YAC calendar/
    // end-date (cf. README "Not yet verified against a real coupled run"),
    // so YAC_ACTION_OUT_OF_BOUND/PUT_FOR_RESTART never fires here to signal
    // "no more data is coming" -- without this cap yac_cget_ would spin
    // forever once the source side (ebfm_comin) has exited. Update this if
    // the replay data this app is run against records a different number
    // of timesteps.
    #define N_EXPECTED_EXCHANGES 2

    double data[N_EBFM_FIELDS][RECEIVER_N_POINTS];
    int info   = YAC_ACTION_NONE;
    int ierror = 0;
    for (int t = 0; t < N_EXPECTED_EXCHANGES; ++t) {
        for (size_t i = 0; i < N_EBFM_FIELDS; ++i) {
            yac_cget_(field_ids[i], 1, data[i], &info, &ierror);
            if (ierror != 0) {
                fprintf(stderr, "yac_cget_ failed for '%s'\n",
                        EBFM_FIELDS[i].yac_field_name);
                return 1;
            }
            printf("%-10s:", EBFM_FIELDS[i].yac_field_name);
            for (size_t p = 0; p < RECEIVER_N_POINTS; ++p) {
                printf(" %f", data[i][p]);
            }
            printf("\n");

            // tas_fake fills every point with FAKE_VALUES[i], and our NNN
            // interpolation is an exact nearest-neighbor copy, so every
            // point should come back unchanged.
            for (size_t p = 0; p < RECEIVER_N_POINTS; ++p) {
                if (fabs(data[i][p] - FAKE_VALUES[i]) > 1e-6) {
                    fprintf(stderr,
                            "MISMATCH: '%s' point %zu = %f, expected %f "
                            "(tas_fake's FAKE_VALUES)\n",
                            EBFM_FIELDS[i].yac_field_name, p, data[i][p],
                            FAKE_VALUES[i]);
                    return 1;
                }
            }
        }
        if (info == YAC_ACTION_OUT_OF_BOUND ||
            info == YAC_ACTION_PUT_FOR_RESTART) {
            break;
        }
    }

    yac_cfinalize();
    return 0;
}
