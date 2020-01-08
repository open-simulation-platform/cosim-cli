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
        ("output-config",
            boost::program_options::value<std::string>()->default_value("auto"),
            "The path to an XML file that contains configuration settings "
            "for simulation output, "
            "or one of the special values 'auto', 'all' and 'none'.  "
            "The default is 'auto', "
            "which causes the program to look for a file named 'LogConfig.xml' "
            "in the same directory as the system structure definition file.  "
            "If no such file is found, the effect is the same as for 'all', "
            "meaning that the values of all variables are stored for each "
            "time step.  "
            "'none' disables file output altogether.")
        ("output-dir",
            boost::program_options::value<std::string>()->default_value("."),
            "The path to a directory for storing simulation results.");
    positionalOptions.add_options()
        ("system_structure_path",
            boost::program_options::value<std::string>()->required(),
            "The path to the system structure definition file/directory.  "
            "If this is a file with .xml extension, or a directory that "
            "contains a file named OspSystemStructure.xml, "
            "it will be interpreted as a OSP system structure definition.  "
            "Otherwise, "
            "it will be interpreted as an SSP system structure definition.");
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
    if (path.extension() == ".xml" ||
        (boost::filesystem::is_directory(path) &&
            boost::filesystem::exists(path / "OspSystemStructure.xml"))) {
        return cse::load_cse_config(uriResolver, path, startTime).first;
    } else {
        return cse::load_ssp(uriResolver, path, startTime).first;
    }
}


std::unique_ptr<cse::observer> make_file_observer(
    const boost::filesystem::path& outputDir,
    const std::string& outputConfigArg,
    const boost::filesystem::path& systemStructurePath)
{
    if (outputConfigArg == "auto") {
        const auto systemStructureDir =
            boost::filesystem::is_directory(systemStructurePath)
            ? systemStructurePath
            : systemStructurePath.parent_path();
        const auto autoConfigFile = systemStructureDir / "LogConfig.xml";
        if (boost::filesystem::exists(autoConfigFile)) {
            return std::make_unique<cse::file_observer>(outputDir, autoConfigFile);
        } else {
            return std::make_unique<cse::file_observer>(outputDir);
        }
    } else if (outputConfigArg == "all") {
        return std::make_unique<cse::file_observer>(outputDir);
    } else if (outputConfigArg == "none") {
        return nullptr;
    } else {
        return std::make_unique<cse::file_observer>(outputDir, outputConfigArg);
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
    void simulator_added(cse::simulator_index, cse::observable*, cse::time_point) override {}
    void simulator_removed(cse::simulator_index, cse::time_point) override {}
    void variables_connected(cse::variable_id, cse::variable_id, cse::time_point) override {}
    void variable_disconnected(cse::variable_id, cse::time_point) override {}

    void step_complete(
        cse::step_number /*lastStep*/,
        cse::duration /*lastStepSize*/,
        cse::time_point currentTime)
        override
    {
        logger_.update(currentTime);
    }

    void simulator_step_complete(
        cse::simulator_index,
        cse::step_number,
        cse::duration,
        cse::time_point)
        override
    {}

    progress_logger logger_;
};
} // namespace


int run_subcommand::run(const boost::program_options::variables_map& args) const
{
    const auto runOptions = get_common_run_options(args);
    const auto systemStructurePath =
        boost::filesystem::path(args["system_structure_path"].as<std::string>());

    const auto uriResolver = cse::default_model_uri_resolver();
    auto execution = load_system_structure(
        systemStructurePath,
        *uriResolver,
        runOptions.begin_time);
    if (runOptions.rtf_target) {
        execution.set_real_time_factor_target(*runOptions.rtf_target);
        execution.enable_real_time_simulation();
    }

    auto outputObserver = make_file_observer(
        args["output-dir"].as<std::string>(),
        args["output-config"].as<std::string>(),
        systemStructurePath);
    if (outputObserver) execution.add_observer(std::move(outputObserver));

    execution.add_observer(
        std::make_shared<progress_monitor>(
            runOptions.begin_time,
            runOptions.end_time - runOptions.begin_time,
            10,
            runOptions.mr_progress_resolution));

    execution.simulate_until(runOptions.end_time).get();
    return 0;
}
