#include "run.hpp"

#include <cse/observer/file_observer.hpp>
#include <cse/orchestration.hpp>
#include <cse/ssp_parser.hpp>

#include <boost/filesystem.hpp>


void run_subcommand::setup_options(
    boost::program_options::options_description& options,
    boost::program_options::options_description& positionalOptions,
    boost::program_options::positional_options_description& positions)
    const noexcept
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
        ("output-dir",
            boost::program_options::value<std::string>()->default_value("."),
            "The path to a directory for storing simulation results.")
        ("real-time",
            boost::program_options::value<double>()->value_name("target_rtf")->implicit_value(1),
            "Enables real-time-synchronised simulations.  A target RTF may "
            "optionally be specified, with a default value of 1.");
    positionalOptions.add_options()
        ("ssp_dir",
            boost::program_options::value<std::string>()->required(),
            "The path to an SSP directory, i.e., a directory that contains "
            "an SSD file named 'SystemStructure.ssd'.");
    // clang-format on
    positions.add("ssp_dir", 1);
}


namespace
{
class progress_monitor : public cse::observer
{
    using double_duration = std::chrono::duration<double>;

public:
    progress_monitor(
        cse::time_point startTime,
        cse::duration duration,
        int percentIncrement = 10)
        : startTime_(startTime)
        , fullDuration_(std::chrono::duration_cast<double_duration>(duration))
        , percentIncrement_(percentIncrement)
    {}

private:
    void simulator_added(cse::simulator_index, cse::observable*, cse::time_point) {}
    void simulator_removed(cse::simulator_index, cse::time_point) {}
    void variables_connected(cse::variable_id, cse::variable_id, cse::time_point) {}
    void variable_disconnected(cse::variable_id, cse::time_point) {}

    void step_complete(
        cse::step_number /*lastStep*/,
        cse::duration /*lastStepSize*/,
        cse::time_point currentTime)
    {
        const auto currentDuration =
            std::chrono::duration_cast<double_duration>(currentTime - startTime_);
        const auto progress =
            100.0 * (currentDuration.count() / fullDuration_.count());
        while (nextPercentage_ <= progress) {
            // NOTE: The use of std::clog here is a temporary solution while
            // we wait for cse-core issue #110. We should use the same logging
            // mechanism as cse-core, so all output can be filtered by the user.
            std::clog
                << nextPercentage_ << "% complete, t="
                << std::fixed << cse::to_double_time_point(currentTime)
                << std::defaultfloat << std::endl;
            nextPercentage_ += percentIncrement_;
        }
    }

    void simulator_step_complete(
        cse::simulator_index,
        cse::step_number,
        cse::duration,
        cse::time_point)
    {}

    const cse::time_point startTime_;
    const std::chrono::duration<double> fullDuration_;
    const int percentIncrement_;
    int nextPercentage_ = 10;
};
} // namespace


int run_subcommand::run(const boost::program_options::variables_map& args) const
{
    const auto beginTime = cse::to_time_point(args["begin-time"].as<double>());
    cse::time_point endTime;
    if (args.count("end-time")) {
        if (!args["duration"].defaulted()) {
            throw boost::program_options::error(
                "Options '--duration' and '--end-time' cannot be used simultaneously");
        }
        endTime = cse::to_time_point(args["end-time"].as<double>());
    } else {
        endTime = beginTime + cse::to_duration(args["duration"].as<double>());
    }

    const auto uriResolver = cse::default_model_uri_resolver();
    // NOTE: The use of absolute() here is a workaround for cse-core issue #309.
    const auto sspDir = boost::filesystem::absolute(args["ssp_dir"].as<std::string>());
    auto [execution, simulatorMap] = cse::load_ssp(*uriResolver, sspDir, beginTime);

    // NOTE: The use of absolute() here is a workaround for cse-core issue #310.
    const auto outputDir =
        boost::filesystem::absolute(args["output-dir"].as<std::string>());
    execution.add_observer(std::make_shared<cse::file_observer>(outputDir));

    if (args.count("real-time")) {
        execution.set_real_time_factor_target(args["real-time"].as<double>());
        execution.enable_real_time_simulation();
    }

    execution.add_observer(
        std::make_shared<progress_monitor>(beginTime, endTime - beginTime));

    execution.simulate_until(endTime).get();
    return 0;
}
