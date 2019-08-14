#ifndef CSECLI_INSPECT_HPP
#define CSECLI_INSPECT_HPP

#include "cli_application.hpp"


/// The `inspect` subcommand.
class inspect_subcommand : public cli_subcommand
{
public:
    std::string name() const noexcept override
    {
        return "inspect";
    }

    std::string brief_description() const noexcept override
    {
        return "Shows information about a model";
    }

    std::string long_description() const noexcept override
    {
        return "This command shows information about a model, such as its "
               "name, description, author, version, and so on.  It also "
               "lists information about the model's variables, like their "
               "names, types, causalities, variabilities, etc.\n"
               "\n"
               "The model can be specified with a URI, or, if it's a local "
               "FMU, by its path.  Using a path is equivalent to using a "
               "`file` URI.";
    }

    void setup_options(
        boost::program_options::options_description& options,
        boost::program_options::options_description& positionalOptions,
        boost::program_options::positional_options_description& positions)
        const noexcept override;

    int run(const boost::program_options::variables_map& args) const override;
};


#endif
