#include "run_common.hpp"

#include <cosim/log/logger.hpp>

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
        ("mr-progress",
            boost::program_options::value<int>()->value_name("resolution")->implicit_value(10),
            "Enables printing of machine-readable progress indicator lines on "
            "the form '@progress n t d'.  "
            "Here, n indicates the progress in 1/N-ths of the total time, "
            "where N is the resolution given as an argument to this option.  "
            "t is the current logical time and d is the amount of logical "
            "time that has passed since the start of the simulation.  "
            "t and d are floating-point numbers while n is an integer.")
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

    values.begin_time = cosim::to_time_point(args["begin-time"].as<double>());
    if (args.count("end-time")) {
        if (!args["duration"].defaulted()) {
            throw boost::program_options::error(
                "Options '--duration' and '--end-time' cannot be used simultaneously");
        }
        values.end_time = cosim::to_time_point(args["end-time"].as<double>());
    } else {
        values.end_time =
            values.begin_time + cosim::to_duration(args["duration"].as<double>());
    }

    if (args.count("mr-progress")) {
        values.mr_progress_resolution = args["mr-progress"].as<int>();
    }
    if (args.count("real-time")) {
        values.rtf_target = args["real-time"].as<double>();
    }
    return values;
}


progress_logger::progress_logger(
    cosim::time_point startTime,
    cosim::duration duration,
    int percentIncrement,
    std::optional<int> mrProgressResolution)
    : startTime_(startTime)
    , fullDuration_(cosim::to_double_duration(duration, startTime))
    , percentIncrement_(percentIncrement)
    , mrProgressResolution_(mrProgressResolution)
    , nextPercentage_(percentIncrement)
{}


void progress_logger::update(cosim::time_point currentTime)
{
    const auto currentDuration =
        cosim::to_double_duration(currentTime - startTime_, startTime_);
    const auto progress = currentDuration / fullDuration_;

    const auto percentProgress = 100.0 * progress;
    while (nextPercentage_ <= percentProgress) {
        BOOST_LOG_SEV(cosim::log::logger(), cosim::log::info)
            << nextPercentage_ << "% complete, t="
            << std::fixed << cosim::to_double_time_point(currentTime)
            << std::defaultfloat;
        nextPercentage_ += percentIncrement_;
    }

    if (mrProgressResolution_) {
        const auto mrProgress = *mrProgressResolution_ * progress;
        while (nextMRProgress_ <= mrProgress) {
            std::cout
                << "@progress "
                << nextMRProgress_ << ' '
                << std::fixed
                << cosim::to_double_time_point(currentTime) << ' '
                << currentDuration
                << std::defaultfloat << std::endl;
            ++nextMRProgress_;
        }
    }
}
