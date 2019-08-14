#ifndef CSECLI_RUN_COMMON_HPP
#define CSECLI_RUN_COMMON_HPP

#include <boost/program_options.hpp>
#include <cse/execution.hpp>
#include <cse/model.hpp>

#include <chrono>
#include <optional>


/// Adds the command-line options that are common to all 'run' subcommands.
void setup_common_run_options(
    boost::program_options::options_description& options);


/// Values of common 'run' subcommand options.
struct common_run_option_values
{
    cse::time_point begin_time;
    cse::time_point end_time;
    std::optional<double> rtf_target;
};


/// Reads the values of common 'run' subcommand options from `args`.
common_run_option_values get_common_run_options(
    const boost::program_options::variables_map& args);


/// Simulation progress logger.
class progress_logger
{
public:
    progress_logger(
        cse::time_point startTime,
        cse::duration duration,
        int percentIncrement);

    void update(cse::time_point currentTime);

private:
    const cse::time_point startTime_;
    const std::chrono::duration<double> fullDuration_;
    const int percentIncrement_;
    int nextPercentage_ = 10;
};


#endif
