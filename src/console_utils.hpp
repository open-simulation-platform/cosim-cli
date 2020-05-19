/*
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this
 *  file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#ifndef COSIM_CONSOLE_UTILS_HPP
#define COSIM_CONSOLE_UTILS_HPP

#include <ostream>
#include <string_view>


/**
 *  Prints one or more paragraphs of text, word-wrapped to a given line
 *  width and optionally indented.
 *
 *  \param [inout] ostream
 *      The output stream to which text should be written.
 *  \param [in] string
 *      The text.
 *  \param [in] width
 *      The maximum number of columns in a line, including indentation.
 *  \param [in] indent
 *      The indent width in columns.
 */
void print_wrapped_text(
    std::ostream& ostream,
    std::string_view string,
    int width,
    int indent = 0);


/**
 *  Detects the width of the user's console/terminal window.
 *
 *  Returns `defaultWidth` if the width could not be determined.
 */
int get_console_width(int defaultWidth = 80);


#endif
