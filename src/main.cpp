#include "cli_application.hpp"
#include "version_option.hpp"


int main(int argc, char const* const* argv)
{
    cli_application app(
        "cse",
        "Command-line interface to the Core Simulation Environment",
        "The Core Simulation Environment is free and open-source software for running distributed co-simulations.");
    app.add_global_options(std::make_unique<version_option>("CSE CLI", "0.1.0"));
    return app.run(argc, argv);
}
