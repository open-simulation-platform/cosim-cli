/*
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this
 *  file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "clean_cache.hpp"

#include "cache.hpp"


void clean_cache_subcommand::setup_options(
    boost::program_options::options_description& /*options*/,
    boost::program_options::options_description& /*positionalOptions*/,
    boost::program_options::positional_options_description& /*positions*/)
    const noexcept
{
    // no options
}


int clean_cache_subcommand::run(const boost::program_options::variables_map& /*args*/) const
{
    clean_cache();
    return 0;
}
