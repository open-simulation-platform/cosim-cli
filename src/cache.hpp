#ifndef COSIM_CACHE_HPP
#define COSIM_CACHE_HPP

#include <cosim/orchestration.hpp>

#include <memory>


/// Returns a caching model URI resolver.
std::shared_ptr<cosim::model_uri_resolver> caching_model_uri_resolver();


/// Removes unused data from the application cache directory.
void clean_cache();


#endif // header guard
