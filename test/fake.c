#include "comin.h"

static t_comin_var_handle* tas_handle;

void tas_fake_secondary_constructor(void);
void tas_fake_fill_tas(void);

void comin_main(void) {
    t_comin_var_descriptor tas_descr = {
        "tas", // var name
        1,     // domain id
    };
    comin_var_request_add(tas_descr, false);
    comin_metadata_set_integer(tas_descr, "zaxis_id", COMIN_ZAXIS_2D);
    comin_metadata_set_logical(tas_descr, "restart", false);
    comin_metadata_set_logical(tas_descr, "tracer", false);

    comin_callback_register(
        EP_SECONDARY_CONSTRUCTOR, tas_fake_secondary_constructor);
    comin_callback_register(
        EP_ATM_INTEGRATE_BEFORE, tas_fake_fill_tas);
}

void tas_fake_secondary_constructor(void) {
    t_comin_entry_point eps[] = {EP_ATM_INTEGRATE_BEFORE};
    t_comin_var_descriptor tas_descr = {
        "tas", // var name
        1,     // domain id
    };
    tas_handle = comin_var_get(1, eps, tas_descr, COMIN_FLAG_WRITE);
}

void tas_fake_fill_tas(void) {
    int shape[5];
    comin_var_get_shape(tas_handle, shape);
    int n = shape[0] * shape[1] * shape[2] * shape[3] * shape[4];

    double* ptr = comin_var_get_ptr_double(tas_handle);
    for (int i = 0; i < n; ++i) {
        ptr[i] = 288.15; // fake near-surface temperature [K]
    }
}
