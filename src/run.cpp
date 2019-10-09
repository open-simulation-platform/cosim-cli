#include "run.hpp"

#include "run_common.hpp"

#include <boost/filesystem.hpp>
#include <cse/cse_config_parser.hpp>
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
        ("system_structure_path",
            boost::program_options::value<std::string>()->required(),
            "The path to the system structure definition file/directory.  "
            "If this is a file with .ssd extension, or a directory that "
            "contains a file named SystemStructure.ssd, "
            "it will be interpreted as an SSP system structure definition.  "
            "Otherwise, "
            "it will be interpreted as a CSE system structure definition.");
    // clang-format on
    positions.add("system_structure_path", 1);
}


namespace
{
cse::execution load_system_structure(
    const boost::filesystem::path& path,
    cse::model_uri_resolver& uriResolver,
    cse::time_point startTime)
{
    if (path.extension() == ".ssd" ||
        (boost::filesystem::is_directory(path) &&
            boost::filesystem::exists(path / "SystemStructure.ssd"))) {
        return cse::load_ssp(uriResolver, path, startTime).first;
    } else {
        return cse::load_cse_config(uriResolver, path, startTime).first;
    }
}


class progress_monitor : public cse::observer
{
public:
    progress_monitor(
        cse::time_point startTime,
        cse::duration duration,
        int percentIncrement,
        std::optional<int> mrProgressResolution)
        : logger_(startTime, duration, percentIncrement, mrProgressResolution)
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

    const auto systemStructurePath =
        boost::filesystem::path(args["system_structure_path"].as<std::string>());
    const auto systemStructureDir =
        boost::filesystem::is_directory(systemStructurePath)
        ? systemStructurePath
        : systemStructurePath.parent_path();

    const auto uriResolver = cse::default_model_uri_resolver();
    auto execution = load_system_structure(
        systemStructurePath,
        *uriResolver,
        runOptions.begin_time);
    if (runOptions.rtf_target) {
        execution.set_real_time_factor_target(*runOptions.rtf_target);
        execution.enable_real_time_simulation();
    }

    execution.add_observer(
        std::make_shared<cse::file_observer>(
            args["output-dir"].as<std::string>()));
    execution.add_observer(
        std::make_shared<progress_monitor>(
            runOptions.begin_time,
            runOptions.end_time - runOptions.begin_time,
            10,
            runOptions.mr_progress_resolution));

    execution.simulate_until(runOptions.end_time).get();
    return 0;
}
