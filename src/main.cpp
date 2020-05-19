/*
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this
 *  file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "clean_cache.hpp"
#include "cli_application.hpp"
#include "inspect.hpp"
#include "logging_options.hpp"
#include "project_version_from_cmake.hpp"
#include "run.hpp"
#include "run_single.hpp"
#include "version_option.hpp"

#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <cosim/log/simple.hpp>


void setup_logging_sink()
{
    const auto sink = boost::log::add_console_log();
    namespace expr = boost::log::expressions;
    sink->set_formatter(
        expr::stream << cosim::log::severity << ": " << expr::smessage);
}


int main(int argc, char const* const* argv)
{
    setup_logging_sink();
    cosim::log::set_global_output_level(cosim::log::warning);

    cli_application app(
        "cosim",
        "Command-line co-simulation tool based on libcosim",
        "cosim and libcosim are free and open-source software for running distributed co-simulations.");
    app.add_global_options(std::make_unique<logging_options>());
    app.add_global_options(std::make_unique<version_option>("cosim", project_version));
    app.add_subcommand(std::make_unique<clean_cache_subcommand>());
    app.add_subcommand(std::make_unique<inspect_subcommand>());
    app.add_subcommand(std::make_unique<run_subcommand>());
    app.add_subcommand(std::make_unique<run_single_subcommand>());
    return app.run(argc, argv);
}
