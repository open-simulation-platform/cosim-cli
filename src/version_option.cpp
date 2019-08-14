#include "version_option.hpp"

#include <iostream>


version_option::version_option(
    std::string_view programName,
    std::string_view programVersion)
    : programName_(programName)
    , programVersion_(programVersion)
{
}


void version_option::setup_options(
    boost::program_options::options_description& options)
{
    options.add_options()("version", "Display program version information and exit.");
}


std::optional<int> version_option::handle_options(
    const boost::program_options::variables_map& args)
{
    if (args.count("version")) {
        std::cout << programName_ << ' ' << programVersion_ << std::endl;
        return 0;
    } else {
        return {};
    }
}
