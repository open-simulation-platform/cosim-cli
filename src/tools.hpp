#ifndef COSIM_TOOLS_HPP
#define COSIM_TOOLS_HPP

#include <cosim/uri.hpp>

#include <string_view>


/**
 *  Converts the given string, which may contain a URI or a filesystem path,
 *  to a `cosim::uri` object.
 */
cosim::uri to_uri(std::string_view str);


#endif
