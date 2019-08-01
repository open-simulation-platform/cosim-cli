#include "inspect.hpp"

#include <cse/orchestration.hpp>
#include <cse/uri.hpp>

#include <boost/filesystem.hpp>

#ifdef _WIN32
#    include <cctype>
#endif
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

void print_model_description(const cse::model_description& md)
{
    std::cout
        << std::left
        << std::setw(keyWidth) << "name:" << md.name << '\n'
        << std::setw(keyWidth) << "uuid:" << md.uuid << '\n'
        << std::setw(keyWidth) << "description:" << md.description << '\n'
        << std::setw(keyWidth) << "author:" << md.author << '\n'
        << std::setw(keyWidth) << "version:" << md.version << '\n';
}

void print_variable_descriptions(const cse::model_description& md)
{
    std::cout << "variables:\n";
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

#ifdef _WIN32
bool looks_like_path(std::string_view str)
{
    return str.size() > 2 &&
        std::isalpha(static_cast<unsigned char>(str[0])) &&
        str[1] == ':' &&
        (str[2] == '\\' || str[2] == '/');
}
#endif

} // namespace


int inspect_subcommand::run(const boost::program_options::variables_map& args) const
{
    auto currentPath = boost::filesystem::current_path();
    currentPath += boost::filesystem::path::preferred_separator;
    const auto baseUri = cse::path_to_file_uri(currentPath);

    // On Windows, we treat anything that starts with "X:\" as a path,
    // even though it could in principle be a URI with scheme "X".
    const auto uriOrPath = args["uri_or_path"].as<std::string>();
#ifdef _WIN32
    const auto uriReference = looks_like_path(uriOrPath)
        ? cse::path_to_file_uri(uriOrPath)
        : cse::uri(uriOrPath);
#else
    const auto uriReference = cse::uri(uriOrPath);
#endif

    const auto uriResolver = cse::default_model_uri_resolver();
    const auto model = uriResolver->lookup_model(baseUri, uriReference);
    print_model_description(*model->description());
    if (args.count("no-vars") == 0) {
        print_variable_descriptions(*model->description());
    }
    return 0;
}
