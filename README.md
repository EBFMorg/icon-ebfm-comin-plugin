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
