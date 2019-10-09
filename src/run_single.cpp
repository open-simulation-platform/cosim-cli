#if defined(_WIN32) && !defined(NOMINMAX)
#    define NOMINMAX
#endif
#include "run_single.hpp"

#include "run_common.hpp"
#include "tools.hpp"

#include <boost/container/vector.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <cse/model.hpp>
#include <cse/orchestration.hpp>
#include <cse/timer.hpp>
#include <gsl/span>

#include <algorithm>
#include <cassert>
#include <fstream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <unordered_map>
#include <vector>


void run_single_subcommand::setup_options(
    boost::program_options::options_description& options,
    boost::program_options::options_description& positionalOptions,
    boost::program_options::positional_options_description& positions)
    const noexcept
{
    setup_common_run_options(options);
    // clang-format off
    options.add_options()
        ("output-file",
            boost::program_options::value<std::string>()->default_value("./model-output.csv"),
            "The file to which simulation results should be written.")
        ("step-size,s",
            boost::program_options::value<double>()->default_value(0.01),
            "The co-simulation step size.");
    positionalOptions.add_options()
        ("uri_or_path",
            boost::program_options::value<std::string>()->required(),
            "A model URI or FMU path.")
        ("initial_value",
            boost::program_options::value<std::vector<std::string>>(),
            "Initial values for model variables, on the form <name>=<value>.  "
            "Allowed values for boolean variables are 'true' or 'false'.");
    // clang-format on
    positions.add("uri_or_path", 1);
    positions.add("initial_value", -1);
}


namespace
{

struct variable_values
{
    boost::container::vector<cse::value_reference> realVariables;
    boost::container::vector<double> realValues;
    boost::container::vector<cse::value_reference> integerVariables;
    boost::container::vector<int> integerValues;
    boost::container::vector<cse::value_reference> booleanVariables;
    boost::container::vector<bool> booleanValues;
    boost::container::vector<cse::value_reference> stringVariables;
    boost::container::vector<std::string> stringValues;
};

variable_values parse_initial_values(
    const std::vector<std::string>& args,
    const cse::model_description& modelDescription)
{
    std::unordered_map<std::string_view, const cse::variable_description*> fastVarLookup;
    for (const auto& var : modelDescription.variables) {
        fastVarLookup.emplace(var.name, &var);
    }

    variable_values values;
    for (std::string_view arg : args) {
        const auto equalsPos = arg.find('=');
        if (equalsPos == std::string_view::npos) {
            throw boost::program_options::error(
                "Invalid initial value specification: '" + std::string(arg) +
                "' (correct syntax: name=value)");
        }
        const auto name = arg.substr(0, equalsPos);
        const auto value = arg.substr(equalsPos + 1);

        const auto varIt = fastVarLookup.find(name);
        if (varIt == fastVarLookup.end()) {
            throw std::runtime_error("No such variable: " + std::string(name));
        }
        const auto& var = *(varIt->second);

        if (var.causality != cse::variable_causality::parameter &&
            var.causality != cse::variable_causality::input) {
            throw std::runtime_error(
                "Cannot initialise variable: " + std::string(name) +
                " (only parameter and input variables can be set)");
        }

        try {
            switch (var.type) {
                case cse::variable_type::real:
                    values.realVariables.push_back(var.reference);
                    values.realValues.push_back(boost::lexical_cast<double>(value));
                    break;
                case cse::variable_type::integer:
                    values.integerVariables.push_back(var.reference);
                    values.integerValues.push_back(boost::lexical_cast<int>(value));
                    break;
                case cse::variable_type::boolean:
                    values.booleanVariables.push_back(var.reference);
                    if (value == "true") {
                        values.booleanValues.push_back(true);
                    } else if (value == "false") {
                        values.booleanValues.push_back(false);
                    } else {
                        throw std::runtime_error("");
                    }
                    break;
                case cse::variable_type::string:
                    values.stringVariables.push_back(var.reference);
                    values.stringValues.push_back(std::string(value));
                    break;
                default:
                    assert(false);
            }
        } catch (const std::exception&) {
            throw boost::program_options::error(
                "Invalid value for variable '" + std::string(name) + "': " +
                std::string(value));
        }
    }
    return values;
}


// This reimplements some of the functionality in `cse::file_observer` in order
// to write a CSV file with (almost) the same format.  This is rather
// unsatisfactory, but the alternative was to do a lot more work to refactor and
// expose CSE internals.
class csv_output_writer
{
public:
    csv_output_writer(
        std::shared_ptr<cse::async_slave> simulator,
        const boost::filesystem::path& outputFile)
        : simulator_(simulator)
    {
        assert(simulator);

        outputStream_.exceptions(std::ofstream::badbit | std::ofstream::failbit);
        outputStream_.open(outputFile.string());

        std::stringstream realVarHeader;
        std::stringstream integerVarHeader;
        std::stringstream booleanVarHeader;
        std::stringstream stringVarHeader;
        for (const auto& var : simulator_->model_description().get().variables) {
            switch (var.type) {
                case cse::variable_type::real:
                    realVarHeader << ',' << var.name << " [" << var.reference << ' ' << var.type << ' ' << var.causality << ']';
                    realVariables_.push_back(var.reference);
                    break;
                case cse::variable_type::integer:
                    integerVarHeader << ',' << var.name << " [" << var.reference << ' ' << var.type << ' ' << var.causality << ']';
                    integerVariables_.push_back(var.reference);
                    break;
                case cse::variable_type::boolean:
                    booleanVarHeader << ',' << var.name << " [" << var.reference << ' ' << var.type << ' ' << var.causality << ']';
                    booleanVariables_.push_back(var.reference);
                    break;
                case cse::variable_type::string:
                    stringVarHeader << ',' << var.name << " [" << var.reference << ' ' << var.type << ' ' << var.causality << ']';
                    stringVariables_.push_back(var.reference);
                    break;
                default:
                    assert(false);
            }
        }
        outputStream_ << "Time";
        if (realVarHeader.rdbuf()->in_avail()) outputStream_ << realVarHeader.rdbuf();
        if (integerVarHeader.rdbuf()->in_avail()) outputStream_ << integerVarHeader.rdbuf();
        if (booleanVarHeader.rdbuf()->in_avail()) outputStream_ << booleanVarHeader.rdbuf();
        if (stringVarHeader.rdbuf()->in_avail()) outputStream_ << stringVarHeader.rdbuf();
        outputStream_ << '\n';
    }

    void update(cse::time_point t)
    {
        const auto values = simulator_
                                ->get_variables(
                                    gsl::make_span(realVariables_),
                                    gsl::make_span(integerVariables_),
                                    gsl::make_span(booleanVariables_),
                                    gsl::make_span(stringVariables_))
                                .get();
        outputStream_ << std::fixed << cse::to_double_time_point(t) << std::defaultfloat;
        for (const auto& v : values.real) {
            outputStream_ << ',' << v;
        }
        for (const auto& v : values.integer) {
            outputStream_ << ',' << v;
        }
        for (const auto& v : values.boolean) {
            outputStream_ << ',' << (v ? "true" : "false");
        }
        for (const auto& v : values.string) {
            outputStream_ << ',' << v;
        }
        outputStream_ << '\n';
    }

private:
    std::shared_ptr<cse::async_slave> simulator_;
    std::ofstream outputStream_;

    boost::container::vector<cse::value_reference> realVariables_;
    boost::container::vector<cse::value_reference> integerVariables_;
    boost::container::vector<cse::value_reference> booleanVariables_;
    boost::container::vector<cse::value_reference> stringVariables_;
};


} // namespace


int run_single_subcommand::run(const boost::program_options::variables_map& args) const
{
    const auto runOptions = get_common_run_options(args);
    const auto stepSize = cse::to_duration(args["step-size"].as<double>());
    if (stepSize <= cse::duration(0)) {
        throw boost::program_options::error("Invalid step size (must be >0)");
    }

    progress_logger progress(
        runOptions.begin_time,
        runOptions.end_time - runOptions.begin_time,
        10,
        runOptions.mr_progress_resolution);

    cse::real_time_timer timer;
    if (runOptions.rtf_target) {
        timer.set_real_time_factor_target(*runOptions.rtf_target);
        timer.enable_real_time_simulation();
    }

    auto currentPath = boost::filesystem::current_path();
    currentPath += boost::filesystem::path::preferred_separator;
    const auto baseUri = cse::path_to_file_uri(currentPath);
    const auto uriReference = to_uri(args["uri_or_path"].as<std::string>());
    const auto uriResolver = cse::default_model_uri_resolver();
    const auto model = uriResolver->lookup_model(baseUri, uriReference);

    std::optional<variable_values> initialValues;
    if (args.count("initial_value") > 0) {
        initialValues = parse_initial_values(
            args["initial_value"].as<std::vector<std::string>>(),
            *model->description());
    }

    const auto simulator = model->instantiate("simulator");
    simulator->setup(runOptions.begin_time, runOptions.end_time, {}).get();
    if (initialValues) {
        simulator
            ->set_variables(
                gsl::make_span(initialValues->realVariables),
                gsl::make_span(initialValues->realValues),
                gsl::make_span(initialValues->integerVariables),
                gsl::make_span(initialValues->integerValues),
                gsl::make_span(initialValues->booleanVariables),
                gsl::make_span(initialValues->booleanValues),
                gsl::make_span(initialValues->stringVariables),
                gsl::make_span(initialValues->stringValues))
            .get();
    }

    auto output = csv_output_writer(
        simulator,
        args["output-file"].as<std::string>());

    simulator->start_simulation().get();
    output.update(runOptions.begin_time);
    for (auto t = runOptions.begin_time; t < runOptions.end_time;) {
        const auto dt = std::min(runOptions.end_time - t, stepSize);
        const auto stepResult = simulator->do_step(t, stepSize).get();
        if (stepResult != cse::step_result::complete) {
            simulator->end_simulation().get();
            throw std::runtime_error(
                "Simulator was unable to complete time step at t=" +
                std::to_string(cse::to_double_time_point(t)));
        }
        t += dt;
        output.update(t);
        timer.sleep(t);
        progress.update(t);
    }
    simulator->end_simulation().get();
    return 0;
}
