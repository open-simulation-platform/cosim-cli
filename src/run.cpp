#include "run.hpp"

#include "run_common.hpp"

#include <boost/filesystem.hpp>
#include <cse/observer/file_observer.hpp>
#include <cse/observer/observer.hpp>
#include <cse/orchestration.hpp>
#include <cse/ssp_parser.hpp>

#include <chrono>
#include <memory>


void run_subcommand::setup_options(
    boost::program_options::options_description& options,
    boost::program_options::options_description& positionalOptions,
    boost::program_options::positional_options_description& positions)
    const noexcept
{
    setup_common_run_options(options);
    // clang-format off
    options.add_options()
        ("output-dir",
            boost::program_options::value<std::string>()->default_value("."),
            "The path to a directory for storing simulation results.");
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
public:
    progress_monitor(
        cse::time_point startTime,
        cse::duration duration,
        int percentIncrement = 10)
        : logger_(startTime, duration, percentIncrement)
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
        logger_.update(currentTime);
    }

    void simulator_step_complete(
        cse::simulator_index,
        cse::step_number,
        cse::duration,
        cse::time_point)
    {}

    progress_logger logger_;
};
} // namespace


int run_subcommand::run(const boost::program_options::variables_map& args) const
{
    const auto runOptions = get_common_run_options(args);

    const auto uriResolver = cse::default_model_uri_resolver();
    // NOTE: The use of absolute() here is a workaround for cse-core issue #309.
    const auto sspDir = boost::filesystem::absolute(args["ssp_dir"].as<std::string>());
    auto [execution, simulatorMap] =
        cse::load_ssp(*uriResolver, sspDir, runOptions.begin_time);
    (void)simulatorMap; // Get rid of unused compiler warning.

    // NOTE: The use of absolute() here is a workaround for cse-core issue #310.
    const auto outputDir =
        boost::filesystem::absolute(args["output-dir"].as<std::string>());
    execution.add_observer(std::make_shared<cse::file_observer>(outputDir));

    if (runOptions.rtf_target) {
        execution.set_real_time_factor_target(*runOptions.rtf_target);
        execution.enable_real_time_simulation();
    }

    execution.add_observer(
        std::make_shared<progress_monitor>(
            runOptions.begin_time,
            runOptions.end_time - runOptions.begin_time));

    execution.simulate_until(runOptions.end_time).get();
    return 0;
}
