/*
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this
 *  file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#ifndef COSIM_CLEAN_CACHE_HPP
#define COSIM_CLEAN_CACHE_HPP

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
        return "Removes unused data from the program cache";
    }

    std::string long_description() const noexcept override
    {
        return "This command removes unused files from the directory that "
               "contains cached data for the current user.\n"
               "\n"
               "The primary use for the cache is to unpack FMUs in a "
               "persistent location, "
               "so they don't have to be unpacked over and over for each run.  "
               "This can be a major time saver, "
               "especially when working with large FMUs.  "
               "Over time, however, the cache can grow to take up "
               "a significant amount of disk space.\n"
               "\n"
               "This command allows for safe removal of files from the cache.  "
               "It will remove all files that are not currently in use by another "
               "cosim process.";
    }

    void setup_options(
        boost::program_options::options_description& options,
        boost::program_options::options_description& positionalOptions,
        boost::program_options::positional_options_description& positions)
        const noexcept override;

    int run(const boost::program_options::variables_map& args) const override;
};


#endif
