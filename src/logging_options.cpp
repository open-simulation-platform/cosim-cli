/*
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this
 *  file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "logging_options.hpp"

#include <cosim/log/simple.hpp>


void logging_options::setup_options(
    boost::program_options::options_description& options)
{
    // clang-format off
    options.add_options()
        ("log-level",
            boost::program_options::value<std::string>()->default_value("warning"),
            "Sets the detail/severity level of diagnostic program output.  "
            "Valid argument values, in order of increasing detail (and "
            "decreasing severity), are: error, warning, info, debug, trace.")
        ("verbose,v",
            "Shorthand for --log-level=info.");
    // clang-format on
}


std::optional<int> logging_options::handle_options(
    const boost::program_options::variables_map& args)
{
    if (args.count("verbose")) {
        if (!args["log-level"].defaulted()) {
            throw boost::program_options::error(
                "Options '--log-level' and '--verbose' cannot be used simultaneously");
        }
        cosim::log::set_global_output_level(cosim::log::info);
    } else {
        const auto level = args["log-level"].as<std::string>();
        if (level == "trace") {
            cosim::log::set_global_output_level(cosim::log::trace);
        } else if (level == "debug") {
            cosim::log::set_global_output_level(cosim::log::debug);
        } else if (level == "info") {
            cosim::log::set_global_output_level(cosim::log::info);
        } else if (level == "warning") {
            cosim::log::set_global_output_level(cosim::log::warning);
        } else if (level == "error") {
            cosim::log::set_global_output_level(cosim::log::error);
        } else {
            throw boost::program_options::error(
                "Invalid '--log-level' value: " + level);
        }
    }
    return {};
}
