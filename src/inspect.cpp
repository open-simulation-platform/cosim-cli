/*
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this
 *  file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "inspect.hpp"

#include "cache.hpp"
#include "tools.hpp"

#include <boost/filesystem.hpp>
#include <cosim/orchestration.hpp>
#include <cosim/uri.hpp>

#include <iomanip>
#include <iostream>


void inspect_subcommand::setup_options(
    boost::program_options::options_description& options,
    boost::program_options::options_description& positionalOptions,
    boost::program_options::positional_options_description& positions)
    const noexcept
{
    // clang-format off
    options.add_options()
        ("no-vars", "Do not print information about variables.");
    positionalOptions.add_options()
        ("uri_or_path",
            boost::program_options::value<std::string>()->required(),
            "A model URI or FMU path.");
    // clang-format on
    positions.add("uri_or_path", 1);
}


namespace
{
constexpr int keyWidth = 14;

void print_model_description(const cosim::model_description& md)
{
    std::cout
        << std::left
        << std::setw(keyWidth) << "name:" << md.name << '\n'
        << std::setw(keyWidth) << "uuid:" << md.uuid << '\n'
        << std::setw(keyWidth) << "description:" << md.description << '\n'
        << std::setw(keyWidth) << "author:" << md.author << '\n'
        << std::setw(keyWidth) << "version:" << md.version << '\n';
}

void print_variable_descriptions(const cosim::model_description& md)
{
    std::cout << "variables:\n";
    for (const auto& v : md.variables) {
        std::cout
            << "  - " << std::setw(keyWidth) << "name:" << v.name << '\n'
            << "    " << std::setw(keyWidth) << "reference:" << v.reference << '\n'
            << "    " << std::setw(keyWidth) << "type:" << v.type << '\n'
            << "    " << std::setw(keyWidth) << "causality:" << v.causality << '\n'
            << "    " << std::setw(keyWidth) << "variability:" << v.variability << '\n';
        if (v.start) {
            std::cout << "    " << std::setw(keyWidth) << "start value:";
            std::visit([](const auto& x) { std::cout << x << '\n'; }, *v.start);
        }
    }
}

} // namespace


int inspect_subcommand::run(const boost::program_options::variables_map& args) const
{
    auto currentPath = boost::filesystem::current_path();
    currentPath += boost::filesystem::path::preferred_separator;
    const auto baseUri = cosim::path_to_file_uri(currentPath);
    const auto uriReference = to_uri(args["uri_or_path"].as<std::string>());
    const auto uriResolver = caching_model_uri_resolver();
    const auto model = uriResolver->lookup_model(baseUri, uriReference);
    print_model_description(*model->description());
    if (args.count("no-vars") == 0) {
        print_variable_descriptions(*model->description());
    }
    return 0;
}
