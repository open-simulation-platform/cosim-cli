#include "cache.hpp"

#include <boost/filesystem.hpp>
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
    if (const auto cache = cache_directory_path()) {
        BOOST_LOG_SEV(cse::log::logger(), cse::log::info)
            << "Using cache directory: " << *cache;
        return cse::default_model_uri_resolver(&cache.value());
    } else {
        BOOST_LOG_SEV(cse::log::logger(), cse::log::warning)
            << "Unable to determine user cache directory; caching is disabled.";
        return cse::default_model_uri_resolver();
    }
}


void delete_cache_directory()
{
    if (const auto cache = cache_directory_path()) {
        BOOST_LOG_SEV(cse::log::logger(), cse::log::info)
            << "Removing cache directory: " << *cache;
        const auto n = boost::filesystem::remove_all(*cache);
        if (n == 0) {
            BOOST_LOG_SEV(cse::log::logger(), cse::log::info)
                << "Cache directory empty or nonexistent; no files deleted.";
        }
    } else {
        throw std::runtime_error(
            "Unable to determine user cache directory; cannot delete it.");
    }
}
