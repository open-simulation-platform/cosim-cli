#include "logging_options.hpp"

#include <cse/log/simple.hpp>


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
        cse::log::set_global_output_level(cse::log::info);
    } else {
        const auto level = args["log-level"].as<std::string>();
        if (level == "trace") {
            cse::log::set_global_output_level(cse::log::trace);
        } else if (level == "debug") {
            cse::log::set_global_output_level(cse::log::debug);
        } else if (level == "info") {
            cse::log::set_global_output_level(cse::log::info);
        } else if (level == "warning") {
            cse::log::set_global_output_level(cse::log::warning);
        } else if (level == "error") {
            cse::log::set_global_output_level(cse::log::error);
        } else {
            throw boost::program_options::error(
                "Invalid '--log-level' value: " + level);
        }
    }
    return {};
}
