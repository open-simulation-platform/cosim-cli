#include "cache.hpp"

#include <boost/filesystem.hpp>
#include <cse/file_cache.hpp>
#include <cse/log/logger.hpp>

#include <cstdlib>
#include <optional>
#include <stdexcept>


namespace
{

// A wrapper for `std::getenv()` that checks for both null and emptiness,
// and logs a debug message and returns null in either case.
const char* getenv(const char* variableName)
{
    const auto e = std::getenv(variableName);
    if (e && *e) return e;
    BOOST_LOG_SEV(cse::log::logger(), cse::log::debug)
        << "Environment variable '" << variableName << "' not set.";
    return nullptr;
}

// Obtain the (platform-specific) application cache path for the current user.
std::optional<boost::filesystem::path> user_cache_directory_path()
{
#if defined(_WIN32)
    if (const auto localAppData = getenv("LocalAppData")) {
        return boost::filesystem::path(localAppData);
    }
    if (const auto userProfile = getenv("UserProfile")) {
        return boost::filesystem::path(userProfile) / "AppData" / "Local";
    }

#elif defined(__APPLE__)
    if (const auto home = getenv("HOME")) {
        return boost::filesystem::path(home) / "Library" / "Caches";
    }

#else // some other UNIX-like system
    if (const auto xdgCacheHome = getenv("XDG_CACHE_HOME")) {
        return boost::filesystem::path(xdgCacheHome);
    }
    if (const auto home = getenv("HOME")) {
        return boost::filesystem::path(home) / ".cache";
    }

#endif
    return std::nullopt;
}

// Returns the (platform-specific) user cache path for this application.
std::optional<boost::filesystem::path> cache_directory_path()
{
    if (const auto userCache = user_cache_directory_path()) {
        return *userCache / "cse";
    } else {
        return std::nullopt;
    }
}

} // namespace


std::shared_ptr<cse::model_uri_resolver> caching_model_uri_resolver()
{
    if (const auto cachePath = cache_directory_path()) {
        const auto cache = std::make_shared<cse::persistent_file_cache>(*cachePath);
        BOOST_LOG_SEV(cse::log::logger(), cse::log::info)
            << "Using cache directory: " << *cachePath;
        return cse::default_model_uri_resolver(cache);
    } else {
        BOOST_LOG_SEV(cse::log::logger(), cse::log::warning)
            << "Unable to determine user cache directory; caching is disabled.";
        return cse::default_model_uri_resolver();
    }
}


void clean_cache()
{
    // NOTE: This makes some assumptions about how the resolver returned by
    // `cse::default_model_uri_resolver()` implements caching.  In particular,
    // it assumes that it uses `cse::fmi::importer`, even though this is not
    // documented.
    if (const auto cachePath = cache_directory_path()) {
        const auto cache = std::make_shared<cse::persistent_file_cache>(*cachePath);
        BOOST_LOG_SEV(cse::log::logger(), cse::log::info)
            << "Cleaning cache directory: " << *cachePath;
        cache->cleanup();
    } else {
        throw std::runtime_error(
            "Unable to determine user cache directory; cannot delete it.");
    }
}
