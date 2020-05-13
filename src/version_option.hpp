#ifndef COSIM_VERSION_OPTION_HPP
#define COSIM_VERSION_OPTION_HPP

#include "cli_application.hpp"

#include <string>
#include <string_view>


/// Implements the `--version` option.
class version_option : public cli_option_set
{
public:
    /**
     *  \param [in] programName
     *      The (human-readable) name of the application.
     *  \param [in] programVersion
     *      The application version.
     */
    version_option(
        std::string_view programName,
        std::string_view programVersion);

    void setup_options(
        boost::program_options::options_description& options) override;

    std::optional<int> handle_options(
        const boost::program_options::variables_map& args) override;

private:
    std::string programName_;
    std::string programVersion_;
};


#endif
