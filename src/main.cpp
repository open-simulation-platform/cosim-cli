#include "clean_cache.hpp"
#include "cli_application.hpp"
#include "inspect.hpp"
#include "logging_options.hpp"
#include "run.hpp"
#include "run_single.hpp"
#include "version_option.hpp"

#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <cse/log/simple.hpp>


void setup_logging_sink()
{
    const auto sink = boost::log::add_console_log();
    namespace expr = boost::log::expressions;
    sink->set_formatter(
        expr::stream << cse::log::severity << ": " << expr::smessage);
}


int main(int argc, char const* const* argv)
{
    setup_logging_sink();
    cse::log::set_global_output_level(cse::log::warning);

    cli_application app(
        "cse",
        "Command-line interface to the Core Simulation Environment",
        "The Core Simulation Environment is free and open-source software for running distributed co-simulations.");
    app.add_global_options(std::make_unique<logging_options>());
    app.add_global_options(std::make_unique<version_option>("CSE CLI", "0.1.0"));
    app.add_subcommand(std::make_unique<clean_cache_subcommand>());
    app.add_subcommand(std::make_unique<inspect_subcommand>());
    app.add_subcommand(std::make_unique<run_subcommand>());
    app.add_subcommand(std::make_unique<run_single_subcommand>());
    return app.run(argc, argv);
}
