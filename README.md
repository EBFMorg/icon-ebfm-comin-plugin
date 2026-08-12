<!--
SPDX-FileCopyrightText: 2026 The ICON EBFM ComIn Plugin Authors

SPDX-License-Identifier: BSD-3-Clause
-->

# ICON EBFM ComIn Plugin

ComIn Plugin to implement EBFM coupling ICON.

## Preparations

Clone ComIn where you want it. This plugin needs the named-YAC-points API from branch
[`166-yac-named-points`](https://gitlab.dkrz.de/icon-comin/comin/-/tree/166-yac-named-points) —
`main.c` will not build against the default branch:

```sh
git clone --branch 166-yac-named-points https://gitlab.dkrz.de/icon-comin/comin.git $COMIN_REPO_DIR
cd $COMIN_REPO_DIR
cmake -B build -DCOMIN_ENABLE_EXAMPLES=OFF -DBUILD_TESTING=ON -DCOMIN_ENABLE_REPLAY_TOOL=ON -DCOMIN_ENABLE_YAC=ON -DCMAKE_PREFIX_PATH=$YAC_INSTALL_DIR comin/
make -C build
```

This plugin also needs YAC itself (`find_package(YAC)` in [`CMakeLists.txt`](CMakeLists.txt));
point `CMAKE_PREFIX_PATH` at your YAC installation both here and in the [Build](#build) step below.

## Build

From this folder

```sh
export ComIn_DIR=$COMIN_REPO_DIR/build
cmake -B build -DCMAKE_PREFIX_PATH=$YAC_INSTALL_DIR .
make -C build
```

## Run Tests

```sh
cd build
ctest
# if you want to run individual tests and see their output
ctest -R ebfm_comin_replay -V

# 2: INFO(ebfm_comin): Hello ComIn!
# ...
# 2: INFO(ebfm_comin): See you later ComInator!

```

## Use this plugin in ICON

After compilation you need to do the following steps.

### Prerequisites

- ICON must be built with `--enable-comin`, using a ComIn version compatible with the one this
  plugin was built against (matching major version, host minor version at least as high as the
  plugin's).
- ICON must be run with **AES physics**. This plugin reads the near-surface temperature `tas`,
  which is only diagnosed by the AES physics package (`mo_aes_phy_memory`); under NWP physics
  `tas` does not exist, and the plugin will abort as soon as it tries to access the variable.
- **Not yet verified against a real coupled run.** `main.c` defines its YAC fields on top of
  ICON's own grid by looking it up as `yac_cget_grid_id("icon_atmos_grid")` /
  `yac_cget_points_id(grid_id, YAC_LOCATION_CELL, "cell_centers")`
  (see [`yac_example.py`](https://gitlab.dkrz.de/icon-comin/comin/-/blob/166-yac-named-points/plugins/python_adapter/examples/yac_example.py)).
  This assumes ICON's own coupling code registers its cell-center point set under the name
  `"cell_centers"` — I could not confirm this by reading ICON's `feature_icono2elmer` branch (no
  occurrences of that name in `src/coupling`), and the replay test does not exercise
  `yac_cenddef()` so it cannot catch a mismatch either. Confirm this against a real ICON run (or a
  replay test with a second, real YAC participant, cf. comin's own `yac_input` test) before
  relying on it.
- The component name this plugin registers (`ebfm_comin`) and the coupling period (currently
  `PT3H` in `ebfm_dummy/config/coupling_icon_atmo.yaml`) need to be agreed with whatever the real
  EBFM-side coupling config targets.

### Make the plugin discoverable

Add the directory containing `libebfm_comin.so` to `LD_LIBRARY_PATH` in your ICON run script, e.g.

```sh
export LD_LIBRARY_PATH="/path/to/this/repo/build:$LD_LIBRARY_PATH"
```

or, if your run script's `basedir` variable is set to your ICON build directory, use the run
script helper function instead:

```sh
add_comin_setup "/path/to/this/repo/build"
```

### Add the plugin to `comin_nml`

Add the following to your `comin_nml` (part of ICON's `atmo_namelist` file):

```fortran
&comin_nml
  plugin_list(1)%name           = "ebfm_comin"
  plugin_list(1)%plugin_library = "libebfm_comin.so"
/
```

To deactivate the plugin, remove (or comment out) this block.

### pre-commit

This project uses pre-commit hooks for some tasks described in detail below. To setup pre-commit please do the following:

```sh
pipx install pre-commit
pre-commit install
```

### Copyright and licensing

This project uses [REUSE](https://reuse.software/) to track information regarding copyright and licensing. Therefore, all files in this repository are required to provide the corresponding information. Please refer to the documentation of REUSE for details.

You can use pre-commit to automatically check if all files in the repository provide the necessary information:

```sh
pre-commit run reuse --all-files
```

If files are missing copyright information you can use the following command to annotate with reuse:

```sh
reuse annotate --license BSD-3-Clause --copyright "The ICON EBFM ComIn Plugin Authors" path/to/new/file
```
