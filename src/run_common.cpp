#include "run_common.hpp"

#include <cse/log/logger.hpp>

#include <ios>
#include <iostream>


void setup_common_run_options(
    boost::program_options::options_description& options)
{
    // clang-format off
    options.add_options()
        ("begin-time,b",
            boost::program_options::value<double>()->default_value(0.0),
            "The logical time at which the simulation should start.")
        ("duration,d",
            boost::program_options::value<double>()->default_value(1.0),
            "The duration of the simulation, in logical time.  "
            "Excludes -e/--end-time.")
        ("end-time,e",
            boost::program_options::value<double>(),
            "The logical end time of the simulation.  "
            "Excludes -d/--duration.")
        ("real-time",
            boost::program_options::value<double>()->value_name("target_rtf")->implicit_value(1),
            "Enables real-time-synchronised simulations.  A target RTF may "
            "optionally be specified, with a default value of 1.");
    // clang-format on
}


common_run_option_values get_common_run_options(
    const boost::program_options::variables_map& args)
{
    common_run_option_values values;

    values.begin_time = cse::to_time_point(args["begin-time"].as<double>());
    if (args.count("end-time")) {
        if (!args["duration"].defaulted()) {
            throw boost::program_options::error(
                "Options '--duration' and '--end-time' cannot be used simultaneously");
        }
        values.end_time = cse::to_time_point(args["end-time"].as<double>());
    } else {
        values.end_time =
            values.begin_time + cse::to_duration(args["duration"].as<double>());
    }

    if (args.count("real-time")) {
        values.rtf_target = args["real-time"].as<double>();
    }
    return values;
}


using double_duration = std::chrono::duration<double>;


progress_logger::progress_logger(
    cse::time_point startTime,
    cse::duration duration,
    int percentIncrement)
    : startTime_(startTime)
    , fullDuration_(std::chrono::duration_cast<double_duration>(duration))
    , percentIncrement_(percentIncrement)
{}


void progress_logger::update(cse::time_point currentTime)
{
    const auto currentDuration =
        std::chrono::duration_cast<double_duration>(currentTime - startTime_);
    const auto progress =
        100.0 * (currentDuration.count() / fullDuration_.count());
    while (nextPercentage_ <= progress) {
        BOOST_LOG_SEV(cse::log::logger(), cse::log::info)
            << nextPercentage_ << "% complete, t="
            << std::fixed << cse::to_double_time_point(currentTime)
            << std::defaultfloat;
        nextPercentage_ += percentIncrement_;
    }
}
