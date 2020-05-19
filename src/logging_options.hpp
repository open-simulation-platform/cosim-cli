/*
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this
 *  file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#ifndef COSIM_LOGGING_OPTIONS_HPP
#define COSIM_LOGGING_OPTIONS_HPP

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
