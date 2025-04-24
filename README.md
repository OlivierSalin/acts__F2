# Acts Common Tracking Software
ACTS version 31.2.0

## Step by step instruction:

#### Clone repo
```
git clone https://github.com/OlivierSalin/acts__F2.git acts-src
cd acts-src
git submodule update --init
```
#### Install all dependencies if access to cvmfs
```
source CI/setup_cvmfs_lcg.sh
```
#### Build
```
cmake -B acts-build -S acts-src \
  -GNinja \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DCMAKE_CXX_STANDARD=17 \
  -DCMAKE_INSTALL_PREFIX="acts-install" \
  -DACTS_BUILD_FATRAS=ON \
  -DACTS_BUILD_FATRAS_GEANT4=ON \
  -DACTS_BUILD_EXAMPLES_GEANT4=ON \
  -DACTS_BUILD_EXAMPLES_PYTHIA8=ON \
  -DACTS_BUILD_EXAMPLES_PYTHON_BINDINGS=ON \
  -DACTS_FORCE_ASSERTIONS=ON \
  -DACTS_ENABLE_LOG_FAILURE_THRESHOLD=ON

cmake --build acts-build --target install
```

ACTS needs to be source before each use:
```
source CI/setup_cvmfs_lcg.sh
source acts-install/bin/this_acts.sh
source acts-install/python/setup.sh
```

Setup can be testied using this tutorial examples if bug contact me: olivier.salin@cern.ch
```
python acts-src/Examples/Scripts/Python/truth_tracking_kalman.py
python acts-src/Examples/Scripts/Python/truth_tracking_telescope.py
```

More information can be found in the [Acts documentation](https://acts.readthedocs.io/).

## Repository organization

The repository contains all code of the Acts projects, not just the core library
that a physics experiment is expected to use as part of its reconstruction code.
All optional components are disabled by default. Please see the
[getting started guide](docs/getting_started.md) on how-to enable them.

-   `Core/` contains the core library that provides functionality in the `Acts`
    namespace.
-   `Plugins/` contains plugins for core functionality that requires
    additional external packages. The functionality also resides in the `Acts`
    namespace.
-   `Fatras/` provides fast track simulation tools based on the core
    library. This is not part of the core functionality and thus resides in the
    separate `ActsFatras` namespace.
-   `Examples/` contains simulation and reconstruction examples. These are
    internal tools for manual full-chain development and tests and reside in
    the `ActsExamples` namespace.
-   `Tests/` contains automated unit tests, integration tests, and
    (micro-)benchmarks.
-   `thirdparty/` contains external dependencies that are usually not available
    through the system package manager.

