/*
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this
 *  file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "version_option.hpp"

#include <cosim/lib_info.hpp>

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
        const auto libcosimVersion = cosim::library_version();
        std::cout << "Using " << cosim::library_short_name << ' '
                  << libcosimVersion.major << '.'
                  << libcosimVersion.minor << '.'
                  << libcosimVersion.patch << std::endl;
        return 0;
    } else {
        return {};
    }
}
