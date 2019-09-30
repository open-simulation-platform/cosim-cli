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
    delete_cache_directory();
    return 0;
}
