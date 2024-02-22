cosim
=====

`cosim` is a command-line co-simulation tool based on [libcosim].
It has three primary use cases:

  * Running co-simulations from a command-line environment
  * Running co-simulations from other programs or scripts
  * FMU testing and debugging

Specifically, `cosim` can be used to perform the following tasks:

  * Run a simulation based on a system structure given in either OSP or SSP format
  * Run a simulation based on a single FMU (usually for testing/debugging purposes)
  * Show information about an FMU

The output from the simulations is in the form of CSV files that can be easily
parsed by other programs.

Usage
-----
The command syntax for the program has a "Git-like" subcommand structure.  That is,
all commands are on the form

    cosim <subcommand> <arguments...> [options...]

For example, the following will run a simulation based on the system description
in `path/to/my_system` and write the results to `path/to/my_results`, providing
extra information (e.g. progress) to the user's terminal:

    cosim run path/to/my_system --output-dir=path/to/my_results -v

The documentation is built into the program itself and can be accessed by
means of the `help` subcommand.

How to build
------------

The tools and steps required to build `cosim` are more or less the same as those
required for libcosim, so we refer to the [libcosim README] for this information.
There are some noteworthy differences, though:

  * Conan 2.x is a *mandatory* requirement for the time being.
  * Doxygen is not needed, as there is no API documentation to generate.

To summarise, a typical configure–build–run session might look like the following.
On Linux, starting from the root source directory (the one that contains this
README):
```sh
conan install -s build_type=Release --build=missing .   # Install dependencies
cmake --preset=conan-release                            # Configure build system
cmake --build --preset=conan-release                    # Build
cmake --build --preset=conan-release --target=install   # Install to dist/
build/Release/dist/bin/cosim help                       # Run
```
And on Windows:
```bat
conan install -s build_type=Release --build=missing .   &:: Install dependencies
cmake --preset=conan-default                            &:: Configure build system
cmake --build --preset=conan-release                    &:: Build
cmake --build --preset=conan-release --target=install   &:: Install to dist/
build/dist/bin/cosim help                               &:: Run
```
In both cases, `Release` and `conan-release` can be replaced with `Debug` and
`conan-debug`, respectively, if you're building for development purposes.

The `cmake --target=install` command will copy the resulting `cosim`
executable to the `build/Release/dist/bin` directory. The shared libraries
that `cosim` depends on will be copied to the same directory or to
`build/Release/lib`, depending on platform, by the `conan install` command.
Thus, the `dist` directory contains the entire release bundle. You may also
choose to install to a different directory by setting the
[`CMAKE_INSTALL_PREFIX`] variable, but note that dependencies won't be
included in the installation then.


[`CMAKE_INSTALL_PREFIX`]: https://cmake.org/cmake/help/latest/variable/CMAKE_INSTALL_PREFIX.html
[libcosim]: https://github.com/open-simulation-platform/libcosim
[libcosim README]: https://github.com/open-simulation-platform/libcosim#readme
