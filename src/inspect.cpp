#include "inspect.hpp"

#include <cse/orchestration.hpp>
#include <cse/uri.hpp>

#include <boost/filesystem.hpp>

#include <iomanip>
#include <iostream>


void inspect_subcommand::setup_options(
    boost::program_options::options_description& /*options*/,
    boost::program_options::options_description& positionalOptions,
    boost::program_options::positional_options_description& positions)
    const noexcept
{
    // clang-format off
    positionalOptions.add_options()
        ("uri_or_path",
            boost::program_options::value<std::string>()->required(),
            "A model URI or FMU path.");
    // clang-format on
    positions.add("uri_or_path", 1);
}


namespace
{
void print_model_description(const cse::model_description& md)
{
    constexpr int keyWidth = 14;
    std::cout
        << std::left
        << std::setw(keyWidth) << "name:" << md.name << '\n'
        << std::setw(keyWidth) << "uuid:" << md.uuid << '\n'
        << std::setw(keyWidth) << "description:" << md.description << '\n'
        << std::setw(keyWidth) << "author:" << md.author << '\n'
        << std::setw(keyWidth) << "version:" << md.version << '\n'
        << "variables:\n";
    for (const auto& v : md.variables) {
        std::cout
            << "  - " << std::setw(keyWidth) << "name:" << v.name << '\n'
            << "    " << std::setw(keyWidth) << "reference:" << v.index << '\n'
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
    const auto baseUri = cse::path_to_file_uri(currentPath);

    const auto uriResolver = cse::default_model_uri_resolver();
    const auto model = uriResolver->lookup_model(
        baseUri,
        args["uri_or_path"].as<std::string>());

    print_model_description(*model->description());
    return 0;
}
