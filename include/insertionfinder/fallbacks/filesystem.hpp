#pragma once
#include <config.h>

#ifdef HAVE_FILESYSTEM
    #include <filesystem>
#else
    #include <boost/filesystem.hpp>

    namespace std {
        namespace filesystem = boost::filesystem;
    };
#endif
