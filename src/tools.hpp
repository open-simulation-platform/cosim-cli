#ifndef CSECLI_TOOLS_HPP
#define CSECLI_TOOLS_HPP

#include <cse/uri.hpp>

#include <string_view>


/**
 *  Converts the given string, which may contain a URI or a filesystem path,
 *  to a `cse::uri` object.
 */
cse::uri to_uri(std::string_view str);


#endif
