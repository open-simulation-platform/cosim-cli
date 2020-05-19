/*
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this
 *  file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#ifndef COSIM_RUN_SINGLE_HPP
#define COSIM_RUN_SINGLE_HPP

#include "cli_application.hpp"


/// The `run-single` subcommand.
class run_single_subcommand : public cli_subcommand
{
public:
    std::string name() const noexcept override
    {
        return "run-single";
    }

    std::string brief_description() const noexcept override
    {
        return "Runs a simulation with a single subsimulator";
    }

    std::string long_description() const noexcept override
    {
        return "This command runs a simulation consisting of a single subsimulator.  "
               "This is mainly meant for testing and debugging.  "
               "The simulator is therefore run directly, "
               "without the overhead and extra machinery of a full co-simulation.\n"
               "\n"
               "The model to simulate can be specified with a URI, or, "
               "if it's a local FMU, by its path.  "
               "Using a path is equivalent to using a 'file' URI.\n"
               "\n"
               "Initial variable values may be specified after the model URI/path, "
               "by supplying a list of name=value pairs as individual command-line arguments.\n"
               "\n"
               "The simulation can be synchronised with real time by using the '--real-time' option, "
               "optionally specifying a target real-time factor (RTF).  "
               "The RTF is defined as 'elapsed logical time divided by elapsed real time in seconds'.  "
               "In other words:\n"
               "\n"
               "* RTF < 1 means slower than real time\n"
               "* RTF = 1 means real time\n"
               "* RTF > 1 means faster than real time\n"
               "\n"
               "Whether the target RTF can actually be reached depends on the simulation setup, "
               "most importantly the models being simulated, "
               "and the machine(s) on which the simulation is being executed.\n"
               "\n"
               "If '--real-time' is not specified, "
               "the default is to run as fast as possible, unconstrained by real time.";
    }

    void setup_options(
        boost::program_options::options_description& options,
        boost::program_options::options_description& positionalOptions,
        boost::program_options::positional_options_description& positions)
        const noexcept override;

    int run(const boost::program_options::variables_map& args) const override;
};


#endif
