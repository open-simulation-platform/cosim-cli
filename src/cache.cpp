#include "cache.hpp"

#include <boost/filesystem.hpp>
#include <cosim/file_cache.hpp>
#include <cosim/log/logger.hpp>

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
    BOOST_LOG_SEV(cosim::log::logger(), cosim::log::debug)
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
        return *userCache / "cosim";
    } else {
        return std::nullopt;
    }
}

} // namespace


std::shared_ptr<cosim::model_uri_resolver> caching_model_uri_resolver()
{
    if (const auto cachePath = cache_directory_path()) {
        const auto cache = std::make_shared<cosim::persistent_file_cache>(*cachePath);
        BOOST_LOG_SEV(cosim::log::logger(), cosim::log::info)
            << "Using cache directory: " << *cachePath;
        return cosim::default_model_uri_resolver(cache);
    } else {
        BOOST_LOG_SEV(cosim::log::logger(), cosim::log::warning)
            << "Unable to determine user cache directory; caching is disabled.";
        return cosim::default_model_uri_resolver();
    }
}


void clean_cache()
{
    if (const auto cachePath = cache_directory_path()) {
        const auto cache = std::make_shared<cosim::persistent_file_cache>(*cachePath);
        BOOST_LOG_SEV(cosim::log::logger(), cosim::log::info)
            << "Cleaning cache directory: " << *cachePath;
        cache->cleanup();
    } else {
        throw std::runtime_error(
            "Unable to determine user cache directory; cannot delete it.");
    }
}
