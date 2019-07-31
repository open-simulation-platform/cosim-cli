#include "cli_application.hpp"
#include "logging_options.hpp"
#include "version_option.hpp"

#include <cse/log/simple.hpp>

#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/console.hpp>


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
    return app.run(argc, argv);
}
