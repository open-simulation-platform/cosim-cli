/*
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this
 *  file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
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
