#ifndef CSECLI_CACHE_HPP
#define CSECLI_CACHE_HPP

#include <cse/orchestration.hpp>

#include <memory>


/// Returns a caching model URI resolver.
std::shared_ptr<cse::model_uri_resolver> caching_model_uri_resolver();


/// Removes unused data from the application cache directory.
void clean_cache();


#endif // header guard
