// SPDX-FileCopyrightText: 2026 The ICON EBFM ComIn Plugin Authors
//
// SPDX-License-Identifier: BSD-3-Clause

// Stands in for the ICON fields that main.c reads (see EBFM_FIELDS in
// ebfm_fields.h), so the replay test can exercise main.c without a real
// ICON run providing them. Test-only; never add this plugin to a real
// ICON run's comin_nml.

#ifndef FAKE_VALUES_H
#define FAKE_VALUES_H

#include "../ebfm_fields.h"

// Shared between test/fake.c (fills these into ICON's fields) and
// test/receiver.c (checks the values it receives from ebfm_comin match
// these), so the two can't drift apart.
//
// TODO: these placeholder values are only chosen to be roughly
// plausible per field; they are not physically consistent with each other.
static const double FAKE_VALUES[N_EBFM_FIELDS] = {
    288.15,   // tas [K]
    0.0,      // prls / pr_snow [kg m-2 s-1]
    0.0,      // prlr / pr [kg m-2 s-1]
    3.5,      // sfcwind [m s-1]
    200.0,    // rsds [W m-2]
    300.0,    // rlds [W m-2]
    0.5,      // clt [1]
    101325.0, // pres_sfc / sfcpres [Pa]
    0.005,    // qv2m / huss [kg kg-1]
};

#endif // FAKE_VALUES_H
