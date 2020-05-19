/*
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this
 *  file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#ifndef COSIM_CLI_APPLICATION_HPP
#define COSIM_CLI_APPLICATION_HPP

#include <boost/program_options.hpp>

#include <memory>
#include <optional>
#include <string>
#include <string_view>


/// A base class for `cli_application` global option handlers.
class cli_option_set
{
public:
    /**
     *  Defines the command-line options in this set.
     *
     *  Overriding functions are not allowed to overwrite the `options`
     *  object, only add to it.
     *
     *  \param [in] options
     *      Object to be filled with option descriptions.
     */
    virtual void setup_options(
        boost::program_options::options_description& options) = 0;

    /**
     *  Handles options passed on the command line when the program is run.
     *
     *  If this function throws an exception or returns an exit code (i.e.,
     *  a non-empty object), the program will terminate immediately.
     *  No further option handlers will be called, and no subcommand will be
     *  invoked.
     *
     *  \param [in] args
     *      Command-line arguments, parsed according to the options
     *      descriptions produced by `setup_options()`.
     *      May contain other global options, which should be ignored.
     *
     *  \returns
     *      An exit code for the program if the given options should cause
     *      the program to terminate immediately, or an empty object otherwise.
     */
    virtual std::optional<int> handle_options(
        const boost::program_options::variables_map& args) = 0;
};


/**
 *  A base class for `cli_application` subcommands.
 */
class cli_subcommand
{
public:
    /// Returns the subcommand name.
    virtual std::string name() const noexcept = 0;

    /**
     *  Defines the command-line options for the subcommand.
     *
     *  Overriding functions are not allowed to overwrite the `options`,
     *  `positionalOptions` and `positions` objects, only add to them.
     *
     *  \param [in] options
     *      Object to be filled with descriptions of non-positional options.
     *  \param [in] positionalOptions
     *      Object to be filled with descriptions of positional options.
     *  \param [in] positions
     *      Object to be filled with the positions of the positional options.
     */
    virtual void setup_options(
        boost::program_options::options_description& options,
        boost::program_options::options_description& positionalOptions,
        boost::program_options::positional_options_description& positions)
        const noexcept = 0;

    /**
     *  Returns a brief description of the subcommand, typically a single
     *  phrase or sentence.
     */
    virtual std::string brief_description() const noexcept = 0;

    /// Returns a longer description of the subcommand (optional).
    virtual std::string long_description() const noexcept { return {}; }

    /**
     *  Runs the subcommand.
     *
     *  If an exception is thrown from this function, it will be caught by
     *  `cli_application::run()` and converted to a nonzero program exit
     *  code.
     *
     *  \param [in] args
     *      Command-line arguments, parsed according to the options
     *      descriptions produced by `setup_options()`.
     *      May contain some built-in options which should be ignored.
     *
     *  \returns
     *      An exit code for the program.
     */
    virtual int run(const boost::program_options::variables_map& args) const = 0;
};


/**
 *  A command parser and dispatcher for CLI applications with a Git-style
 *  subcommand structure.
 *
 *  Only one level of subcommands is supported.  Subcommands and their
 *  options are defined by `cli_subcommand` objects, while global options
 *  are defined by `cli_option_set` objects.  The `help` subcommand
 *  and the equivalent `--help` option are built in and may not be redefined.
 */
class cli_application
{
public:
    /**
     *  Constructor.
     *
     *  The constructor parameters are currently only used to provide
     *  human-readable output for the `help` subcommand/option.
     *
     *  \param [in] command
     *      The command that gets called to run the application (typically
     *      the executable base name without an extension).
     *  \param [in] briefDescription
     *      A brief description of the application, typically one or two
     *      sentences.
     *  \param [in] longDescription
     *      A longer description of the application.
     */
    cli_application(
        std::string_view command,
        std::string_view briefDescription,
        std::string_view longDescription = {});

    ~cli_application() noexcept;

    cli_application(cli_application&&) noexcept;
    cli_application& operator=(cli_application&&) noexcept;

    cli_application(const cli_application&) = delete;
    cli_application& operator=(const cli_application&) = delete;

    /**
     *  Adds a set of global options to the application.
     *
     *  Options may not conflict with options from other global option sets,
     *  nor with subcommand-specific options.
     *
     *  Options will be handled by the `run()` command in the order
     *  they were added.
     */
    void add_global_options(std::unique_ptr<cli_option_set> optionSet);

    /**
     *  Adds a subcommand to the application.
     *
     *  It is not allowed to add two subcommands with the same name.
     */
    void add_subcommand(std::unique_ptr<cli_subcommand> subcommand);

    /**
     *  Parses the given command line argument, runs global option handlers,
     *  and invokes the requested subcommand.
     *
     *  \returns
     *      An exit code that can be returned from the program's `main()`
     *      function.
     */
    int run(int argc, char const* const* argv) const noexcept;

private:
    class impl;
    std::unique_ptr<impl> impl_;
};


#endif
