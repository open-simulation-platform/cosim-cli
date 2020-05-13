/*
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this
 *  file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "tools.hpp"

#include <boost/filesystem.hpp>

#ifdef _WIN32
#    include <cctype>
#endif


cosim::uri to_uri(std::string_view str)
{
    auto uri = cosim::uri(str);
#ifdef _WIN32
    // On Windows, we treate anything that starts with a single letter followed
    // by a colon as a path, even though it could also be interpreted as an URI
    // with a single-character scheme.
    const auto schemeLooksLikeDriveLetter =
        uri.scheme() &&
        uri.scheme()->size() == 1 &&
        std::isalpha(static_cast<unsigned char>(uri.scheme()->front()));
    if (schemeLooksLikeDriveLetter || !uri.scheme()) {
        uri = cosim::path_to_file_uri(boost::filesystem::path(str.begin(), str.end()));
    }
#endif
    return uri;
}
