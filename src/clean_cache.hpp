#ifndef CSECLI_CLEAN_CACHE_HPP
#define CSECLI_CLEAN_CACHE_HPP

#include "cli_application.hpp"


/// The `clean-cache` subcommand.
class clean_cache_subcommand : public cli_subcommand
{
public:
    std::string name() const noexcept override
    {
        return "clean-cache";
    }

    std::string brief_description() const noexcept override
    {
        return "Deletes the program cache";
    }

    std::string long_description() const noexcept override
    {
        return "This command deletes the directory that contains cached "
               "CSE data for the current user.\n"
               "\n"
               "WARNING: This operation is unsafe.  "
               "Cache deletion is not synchronised with other processes "
               "that use the cache, "
               "and must therefore only be performed when no such processes "
               "are running.\n"
               "\n"
               "The primary use for the cache is to unpack FMUs in a "
               "persistent location, "
               "so they don't have to be unpacked over and over.  "
               "The most common reason to do delete it is to free up disk space, "
               "especially if a lot of large FMUs have been used.  "
               "Another reason might be to get rid of misbehaving FMUs.";
    }

    void setup_options(
        boost::program_options::options_description& options,
        boost::program_options::options_description& positionalOptions,
        boost::program_options::positional_options_description& positions)
        const noexcept override;

    int run(const boost::program_options::variables_map& args) const override;
};


#endif
