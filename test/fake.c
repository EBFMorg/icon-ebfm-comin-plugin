// SPDX-FileCopyrightText: 2026 The ICON EBFM ComIn Plugin Authors
//
// SPDX-License-Identifier: BSD-3-Clause

// Stands in for the ICON fields that main.c reads (see EBFM_FIELDS in
// ebfm_fields.h), so the replay test can exercise main.c without a real
// ICON run providing them. Test-only; never add this plugin to a real
// ICON run's comin_nml.

#include "comin.h"
#include "../ebfm_fields.h"
#include "fake_values.h"

#include <stddef.h>
#include <stdio.h>

static t_comin_var_handle fake_handles[N_EBFM_FIELDS];

static void secondary_constructor(void);
static void fill_fields_with_fake_data(void);

void comin_main(void) {
    for (size_t i = 0; i < N_EBFM_FIELDS; ++i) {
        t_comin_var_descriptor descr = {"", EBFM_DOMAIN_ID};
        snprintf(descr.name, sizeof(descr.name), "%s",
                 EBFM_FIELDS[i].icon_var_name);
        comin_var_request_add(&descr, false);
        comin_metadata_set_int(&descr, "zaxis_id", COMIN_ZAXIS_2D);
        comin_metadata_set_bool(&descr, "restart", false);
        comin_metadata_set_bool(&descr, "tracer", false);
    }

    comin_callback_register(
        EP_SECONDARY_CONSTRUCTOR, secondary_constructor);
    comin_callback_register(
        EP_ATM_INTEGRATE_BEFORE, fill_fields_with_fake_data);
}

static void secondary_constructor(void) {
    t_comin_entry_point eps[] = {EP_ATM_INTEGRATE_BEFORE};
    for (size_t i = 0; i < N_EBFM_FIELDS; ++i) {
        t_comin_var_descriptor descr = {"", EBFM_DOMAIN_ID};
        snprintf(descr.name, sizeof(descr.name), "%s",
                 EBFM_FIELDS[i].icon_var_name);
        fake_handles[i] = comin_var_get(1, eps, &descr, COMIN_FLAG_WRITE);
    }
}

static void fill_fields_with_fake_data(void) {
    for (size_t i = 0; i < N_EBFM_FIELDS; ++i) {
        size_t shape[5];
        comin_var_get_shape(fake_handles[i], shape);
        size_t n = shape[0] * shape[1] * shape[2] * shape[3] * shape[4];

        double* ptr = comin_var_get_ptr_double(fake_handles[i]);
        for (size_t j = 0; j < n; ++j) {
            ptr[j] = FAKE_VALUES[i];
        }
    }
}
