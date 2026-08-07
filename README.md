<!--
SPDX-FileCopyrightText: 2026 The ICON EBFM ComIn Plugin Authors

SPDX-License-Identifier: BSD-3-Clause
-->

# ICON EBFM ComIn Plugin

ComIn Plugin to implement EBFM coupling ICON.

## Preparations

Clone ComIn where you want it:

```sh
git clone https://gitlab.dkrz.de/icon-comin/comin.git $COMIN_REPO_DIR
cd $COMIN_REPO_DIR
cmake -B build -DCOMIN_ENABLE_EXAMPLES=OFF -DBUILD_TESTING=ON -DCOMIN_ENABLE_REPLAY_TOOL=ON comin/
make -C build
```

## Build

From this folder

```sh
export ComIn_DIR=$COMIN_REPO_DIR/build
cmake -B build .
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
# 2: INFO(ebfm_comin): Got data 288.150000
# ...
# 2: INFO(ebfm_comin): See you later ComInator!

```

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
