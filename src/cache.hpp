/*
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this
 *  file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#ifndef COSIM_CACHE_HPP
#define COSIM_CACHE_HPP

#include <cosim/orchestration.hpp>

#include <memory>


/// Returns a caching model URI resolver.
std::shared_ptr<cosim::model_uri_resolver> caching_model_uri_resolver();


/// Removes unused data from the application cache directory.
void clean_cache();


#endif // header guard
