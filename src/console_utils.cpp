#define NOMINMAX
#include "console_utils.hpp"

#include <algorithm>
#include <cassert>
#include <cctype>

#ifdef _WIN32
#    include <windows.h>
#else
#    include <stdio.h>
#    include <sys/ioctl.h>
#    include <unistd.h>
#endif


namespace
{

std::string_view consume(std::string_view& text, std::size_t nchars)
{
    const auto consumed = text.substr(0, nchars);
    text.remove_prefix(std::min(nchars, text.size()));
    return consumed;
}

template<typename Predicate>
std::string_view consume_while(std::string_view& text, Predicate&& pred)
{
    std::size_t i = 0;
    while (i < text.size() && pred(text[i])) ++i;
    return consume(text, i);
}

std::string_view consume_paragraph(std::string_view& text)
{
    assert(!text.empty());
    return consume(text, std::min(text.find('\n'), text.size()) + 1);
}

std::string_view consume_word(std::string_view& text)
{
    return consume_while(text, [](unsigned char c) { return !std::isspace(c); });
}

std::string_view consume_space(std::string_view& text)
{
    return consume_while(text, [](unsigned char c) { return std::isspace(c); });
}

} // namespace


void print_wrapped_text(
    std::ostream& ostream,
    std::string_view string,
    int width,
    int indent)
{
    assert(indent >= 0);
    assert(width > indent);

    // Iterate over paragraphs
    while (!string.empty()) {
        auto paragraph = consume_paragraph(string);
        auto space = consume_space(paragraph);
        auto word = consume_word(paragraph);

        // Iterate over visual lines
        if (word.empty()) {
            ostream << '\n';
        } else do {
            int linePos = 0;
            for (; linePos < indent; ++linePos) ostream << ' ';

            // The first word may overflow to the next line.
            const auto firstWord = consume(word, width - linePos);
            ostream << firstWord;
            linePos += static_cast<int>(firstWord.size());

            if (word.empty()) {
                // Iterate over words following the first one
                for (;;) {
                    space = consume_space(paragraph);
                    word = consume_word(paragraph);
                    const auto wordWidth = static_cast<int>(space.size() + word.size());
                    if (word.empty() || linePos + wordWidth > width) {
                        break;
                    } else {
                        ostream << space << word;
                        linePos += wordWidth;
                    }
                }
            }
            ostream << '\n';
        } while (!word.empty());
    }
}


int get_console_width(int defaultWidth)
{
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        return csbi.srWindow.Right - csbi.srWindow.Left + 1;
    }
#else
    winsize size;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &size) == 0) {
        return size.ws_col;
    }
#endif
    return defaultWidth;
}
