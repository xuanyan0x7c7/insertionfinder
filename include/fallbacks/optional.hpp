#pragma once
#include <config.h>

#ifdef HAS_OPTIONAL
    #include <optional>
#else
    #include <boost/optional.hpp>

    namespace std {
        template<class T> using optional = boost::optional<T>;
        using nullopt_t = boost::none_t;
        const nullopt_t nullopt = boost::none;
    };
#endif
