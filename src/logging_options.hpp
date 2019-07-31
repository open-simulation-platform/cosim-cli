#ifndef CSECLI_LOGGING_OPTIONS_HPP
#define CSECLI_LOGGING_OPTIONS_HPP

#include "cli_application.hpp"


/// Implements the `--log-level` and `--verbose` options.
class logging_options : public cli_option_set
{
public:
    void setup_options(
        boost::program_options::options_description& options) override;

    std::optional<int> handle_options(
        const boost::program_options::variables_map& args) override;
};


#endif
