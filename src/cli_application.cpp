#include "cli_application.hpp"

#include "console_utils.hpp"

#include <boost/algorithm/string/replace.hpp>

#include <exception>
#include <iostream>
#include <map>
#include <stdexcept>
#include <utility>
#include <vector>

#ifndef _WIN32
#    include <sysexits.h>
#else
#    define EX_OK 0
#    define EX_USAGE 64
#    define EX_SOFTWARE 70
#endif


// A list type for option handlers.
using option_set_list = std::vector<std::unique_ptr<cli_option_set>>;


// Calls the `setup_options()` functions of multiple `cli_option_set` objects.
void setup_options(
    const option_set_list& optionSets,
    boost::program_options::options_description& options)
{
    for (const auto& s : optionSets) s->setup_options(options);
}


// A mapping from subcommand names to cli_subcommand objects.
using subcommand_map = std::map<std::string, std::unique_ptr<cli_subcommand>>;


// This is just a convenience function for accessing a subcommand_map
// that ensures we get a consistent error message.
const cli_subcommand* get_subcommand(
    const subcommand_map& map,
    const std::string& name)
{
    const auto it = map.find(name);
    if (it == map.end()) throw std::runtime_error("Invalid subcommand: " + name);
    return it->second.get();
}


// =============================================================================
// help_subcommand (internal)
// =============================================================================


class help_subcommand : public cli_subcommand
{
public:
    help_subcommand(
        std::string_view command,
        std::string_view briefDescription,
        std::string_view longDescription,
        const option_set_list& globalOptionSets,
        const subcommand_map& subcommands)
        : command_(command)
        , briefDescription_(briefDescription)
        , longDescription_(longDescription)
        , globalOptionSets_(globalOptionSets)
        , subcommands_(subcommands)
    {
    }

    std::string name() const noexcept override { return "help"; }

    void setup_options(
        boost::program_options::options_description& /*options*/,
        boost::program_options::options_description& positionalOptions,
        boost::program_options::positional_options_description& positions)
        const noexcept override
    {
        // clang-format off
        positionalOptions.add_options()
            ("subcommand",
                boost::program_options::value<std::string>(),
                "A subcommand to show documentation for.");
        // clang-format on
        positions.add("subcommand", 1);
    }

    std::string brief_description() const noexcept override
    {
        return "Shows documentation";
    }

    int run(const boost::program_options::variables_map& args) const override
    {
        const auto& subcommandValue = args["subcommand"];
        if (subcommandValue.empty()) {
            print_toplevel_help();
        } else {
            print_subcommand_help(
                *get_subcommand(subcommands_, subcommandValue.as<std::string>()));
        }
        return EX_OK;
    }

private:
    // This is the indentation level used by boost::property_tree for
    // options_description output.
    static constexpr int indent = 2;

    // The minimum console width for which we even try to format output.
    static constexpr int minLineWidth = indent * 3;

    void print_toplevel_help() const
    {
        const auto cols = std::max(get_console_width(), minLineWidth);
        auto options = boost::program_options::options_description(cols, cols / 2);
        ::setup_options(globalOptionSets_, options);
        setup_help_option(options);

        boost::program_options::positional_options_description positions;
        positions.add("subcommand", 1);
        positions.add("args", 2);

        print_name_section(command_, briefDescription_, cols);
        print_synopsis_section(command_, positions, cols);
        print_description_section(longDescription_, cols);
        print_subcommands_section(subcommands_, cols);
        print_options_section(options);
    }

    void print_subcommand_help(const cli_subcommand& s) const
    {
        const auto cols = std::max(get_console_width(), minLineWidth);
        auto options = boost::program_options::options_description(cols, cols / 2);
        auto positionalOptions = boost::program_options::options_description(cols, cols / 2);
        auto positions = boost::program_options::positional_options_description();
        s.setup_options(options, positionalOptions, positions);
        ::setup_options(globalOptionSets_, options);
        setup_help_option(options);

        const auto fullCommand = command_ + ' ' + s.name();

        print_name_section(fullCommand, s.brief_description(), cols);
        print_synopsis_section(fullCommand, positions, cols);
        print_description_section(s.long_description(), cols);
        print_parameters_section(positionalOptions);
        print_options_section(options);
    }

    static void setup_help_option(boost::program_options::options_description& options)
    {
        options.add_options()("help", "Display a help message and exit.");
    }

    static void print_name_section(
        std::string_view command,
        std::string_view briefDescription,
        int lineWidth)
    {
        std::cout << "NAME\n";
        print_wrapped_text(
            std::cout,
            std::string(command) + " - " + std::string(briefDescription),
            lineWidth,
            indent);
        std::cout << '\n';
    }

    static void print_synopsis_section(
        std::string_view command,
        const boost::program_options::positional_options_description& positions,
        int lineWidth)
    {
        std::stringstream usageLine;
        usageLine << command;
        if (positions.max_total_count() > 0) {
            usageLine << " <";
            for (unsigned i = 0; i < positions.max_total_count(); ++i) {
                if (i > 0) {
                    if (positions.name_for_position(i) == positions.name_for_position(i - 1)) {
                        usageLine << "...";
                        break;
                    } else {
                        usageLine << "> <";
                    }
                }
                usageLine << positions.name_for_position(i);
            }
            usageLine << '>';
        }
        usageLine << " [options...]";

        std::cout << "SYNOPSIS\n";
        print_wrapped_text(std::cout, usageLine.str(), lineWidth, indent);
        std::cout << '\n';
    }

    static void print_description_section(std::string_view text, int lineWidth)
    {
        if (text.empty()) return;
        std::cout << "DESCRIPTION\n";
        print_wrapped_text(std::cout, text, lineWidth, indent);
        std::cout << '\n';
    }

    static void print_subcommands_section(
        const subcommand_map& subcommands,
        int lineWidth)
    {
        auto fakeParameters = boost::program_options::options_description(lineWidth, lineWidth/2);
        for (const auto& sc : subcommands) {
            fakeParameters.add_options()(
                sc.first.c_str(), sc.second->brief_description().c_str());
        }
        print_parameters_section(fakeParameters, "SUBCOMMANDS");
    }

    static void print_parameters_section(
        const boost::program_options::options_description& positionalOptions,
        const char* title = "PARAMETERS")
    {
        if (positionalOptions.options().empty()) return;

        // We now make some assumptions about how boost::program_options formats
        // its options_description output.
        std::stringstream ss;
        ss << positionalOptions;
        auto s = ss.str();
        boost::algorithm::replace_all(s, "  --", "  ");
        boost::algorithm::replace_all(s, " arg ", "       ");

        std::cout
            << title << '\n'
            << s << '\n';
    }

    static void print_options_section(
        const boost::program_options::options_description& options)
    {
        if (options.options().empty()) return;
        std::cout
            << "OPTIONS\n"
            << options
            << '\n';
    }

    std::string command_;
    std::string briefDescription_;
    std::string longDescription_;
    const option_set_list& globalOptionSets_;
    const subcommand_map& subcommands_;
};


// =============================================================================
// cli_application::impl
// =============================================================================


class cli_application::impl
{
public:
    impl(
        std::string_view command,
        std::string_view briefDescription,
        std::string_view longDescription)
        :  command_(command)
    {
        subcommands_["help"] = std::make_unique<help_subcommand>(
            command,
            briefDescription,
            longDescription,
            globalOptionSets_,
            subcommands_);
    }

    ~impl() noexcept = default;

    // Non-movable and non-copyable, because we give internal pointers to the
    // help_subcommand class.
    impl(impl&&) = delete;
    impl& operator=(impl&&) = delete;
    impl(const impl&) = delete;
    impl& operator=(const impl&) = delete;

    void add_global_options(std::unique_ptr<cli_option_set> optionSet)
    {
        globalOptionSets_.push_back(std::move(optionSet));
    }

    void add_subcommand(std::unique_ptr<cli_subcommand> subcommand)
    {
        const auto name = subcommand->name();
        auto inserted = subcommands_.insert({name, std::move(subcommand)}).second;
        if (!inserted) {
            throw std::logic_error("Subcommand already defined: " + name);
        }
    }

    int run(int argc, char const* const* argv) const
    {
        // Separate subcommand from other arguments,
        // replacing any "--help" option with a "help" subcommand.
        const cli_subcommand* subcommand = nullptr;
        std::vector<std::string> args;
        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];
            if (arg[0] != '-' && !subcommand) {
                subcommand = get_subcommand(subcommands_, argv[i]);
            } else if (arg == "--help") {
                if (subcommand) {
                    args.insert(args.begin(), subcommand->name());
                }
                subcommand = get_subcommand(subcommands_, "help");
            } else {
                args.push_back(argv[i]);
            }
        }

        // Set up options
        namespace po = boost::program_options;
        po::options_description options;
        po::positional_options_description positions;

        setup_options(globalOptionSets_, options);
        if (subcommand) subcommand->setup_options(options, options, positions);

        // Parse command-line arguments.
        po::variables_map values;
        po::store(
            po::command_line_parser(args).options(options).positional(positions).run(),
            values);
        po::notify(values);

        // Handle global options
        for (const auto& s : globalOptionSets_) {
            if (const auto retVal = s->handle_options(values)) return *retVal;
        }

        // Finally, run subcommand.
        if (!subcommand) {
            throw std::runtime_error(
                "Missing command-line arguments.  Run \"" + command_ + " help\" to get help.");
        }
        return subcommand->run(values);
    }

private:
    std::string command_;
    std::vector<std::unique_ptr<cli_option_set>> globalOptionSets_;
    subcommand_map subcommands_;
};


// =============================================================================
// cli_application
// =============================================================================


cli_application::cli_application(
    std::string_view command,
    std::string_view briefDescription,
    std::string_view longDescription)
    : impl_(std::make_unique<impl>(command, briefDescription, longDescription))
{
}


cli_application::~cli_application() noexcept = default;


cli_application::cli_application(cli_application&&) noexcept = default;


cli_application& cli_application::operator=(cli_application&&) noexcept = default;


void cli_application::add_global_options(
    std::unique_ptr<cli_option_set> optionSet)
{
    impl_->add_global_options(std::move(optionSet));
}


void cli_application::add_subcommand(std::unique_ptr<cli_subcommand> subcommand)
{
    impl_->add_subcommand(std::move(subcommand));
}


int cli_application::run(int argc, char const* const* argv) const noexcept
{
    try {
        return impl_->run(argc, argv);
    } catch (const boost::program_options::error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EX_USAGE;
    } catch (const std::logic_error& e) {
        std::cerr << "Internal error: " << e.what() << std::endl;
        return EX_SOFTWARE;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
