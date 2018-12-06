#pragma once
#include <insertionfinder/config.h>

#ifdef INSERTIONFINDER_HAVE_FILESYSTEM
    #include <filesystem>
#else
    #include <boost/filesystem.hpp>

    namespace std {
        namespace filesystem = boost::filesystem;
    };
#endif
