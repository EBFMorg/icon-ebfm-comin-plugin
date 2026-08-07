#include "comin.h"

void init(void);
void print_tas(void);
void finalize(void);

static t_comin_var_handle* comin_var_tas;

void comin_main(void) {
    // Follow example https://gitlab.dkrz.de/icon-comin/comin/-/blob/166-yac-named-points/plugins/python_adapter/examples/yac_example.py?ref_type=heads
    // needs ComIn branch https://gitlab.dkrz.de/icon-comin/comin/-/tree/166-yac-named-points?ref_type=heads
    comin_print_info("Hello ComIn!");
    comin_callback_register(EP_SECONDARY_CONSTRUCTOR, init);
    comin_callback_register(EP_ATM_INTEGRATE_AFTER, print_tas);
    comin_callback_register(EP_DESTRUCTOR, finalize);
}

void init(void) {
    t_comin_entry_point eps[] = {EP_ATM_INTEGRATE_AFTER};
    t_comin_var_descriptor tas_descr = {
        "tas", // var name
        1,     // domain id
    };
    comin_var_tas = comin_var_get(1, eps, tas_descr, COMIN_FLAG_READ);
}

void print_tas(void) {
    double const * const ptr = comin_var_get_ptr_double(comin_var_tas);
    comin_print_info_f("Got data %f", *ptr);
}

void finalize(void) {
    comin_print_info("See you later ComInator!");
}
