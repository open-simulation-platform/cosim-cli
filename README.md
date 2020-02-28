CSE CLI
=======

CSE CLI is a command-line interface (CLI) to the [Core Simulation Environment]
(CSE).  It has three primary use cases:

  * Running simulations from other programs or scripts
  * FMU testing and debugging
  * Users who simply prefer to work from the command line

Specifically, the CSE CLI can be used to perform the following tasks:

  * Run a simulation based on a system structure given in either OSP XML or SSP format
  * Run a simulation based on a single FMU (usually for testing/debugging purposes)
  * Show information about an FMU

The output from the simulations is in the form of CSV files that can be easily
parsed by other programs, for example Microsoft Excel.

Usage
-----
The command syntax for the program has a "Git-like" subcommand structure.  That is,
all commands are on the form

    cse <subcommand> <arguments...> [options...]

For example, the following will run a simulation based on the system description
in `path/to/my_system` and write the results to `path/to/my_results`, providing
extra information (e.g. progress) to the user's terminal:

    cse run path/to/my_system --output-dir=path/to/my_results -v

The documentation is built into the program itself, by means of the `help` subcommand.


How to build
------------

The tools and steps required to build CSE CLI are more or less the same as those
required for cse-core, so we refer to the [cse-core README] for this information.
There are some noteworthy differences, though:

  * Conan is a *mandatory* requirement for the time being.
  * Doxygen is not needed, as there is no API documentation to generate.

To summarise, a typical configure–build–run session might look like the following.
On Linux:

    mkdir build
    cd build
    conan install ..
    cmake .. -DCMAKE_BUILD_TYPE=Debug
    cmake --build .
    ./cse help

And on Windows:

    mkdir build
    cd build
    conan install .. -s build_type=Debug -g virtualrunenv
    cmake .. -A x64
    cmake --build .
    activate_run.bat
    Debug\cse help
    deactivate_run.bat


[Core Simulation Environment]: https://github.com/open-simulation-platform
[cse-core README]: https://github.com/open-simulation-platform/cse-core#readme
