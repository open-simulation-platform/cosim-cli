#ifndef CSECLI_CACHE_HPP
#define CSECLI_CACHE_HPP

#include <cse/orchestration.hpp>

#include <memory>


/// Returns a caching model URI resolver.
std::shared_ptr<cse::model_uri_resolver> caching_model_uri_resolver();


/// Deletes the application cache directory.
void delete_cache_directory();


#endif // header guard
