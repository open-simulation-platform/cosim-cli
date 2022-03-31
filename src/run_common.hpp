/*
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this
 *  file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#ifndef COSIM_RUN_COMMON_HPP
#define COSIM_RUN_COMMON_HPP

#include <boost/program_options.hpp>
#include <cosim/execution.hpp>
#include <cosim/time.hpp>

#include <chrono>
#include <optional>


/// Adds the command-line options that are common to all 'run' subcommands.
void setup_common_run_options(
    boost::program_options::options_description& options);


/// Values of common 'run' subcommand options.
struct common_run_option_values
{
    cosim::time_point begin_time;
    cosim::time_point end_time;
    std::optional<double> rtf_target;
    std::optional<int> mr_progress_resolution;
    std::optional<unsigned int> worker_thread_count;
};


/// Reads the values of common 'run' subcommand options from `args`.
common_run_option_values get_common_run_options(
    const boost::program_options::variables_map& args);


/// Simulation progress logger.
class progress_logger
{
public:
    progress_logger(
        cosim::time_point startTime,
        cosim::duration duration,
        int percentIncrement,
        std::optional<int> mrProgressResolution);

    void update(cosim::time_point currentTime);

private:
    const cosim::time_point startTime_;
    const double fullDuration_;
    const int percentIncrement_;
    const std::optional<int> mrProgressResolution_;
    int nextPercentage_;
    int nextMRProgress_ = 1;
};


#endif
