#ifndef GLOBAL_HPP
#define GLOBAL_HPP

#ifdef __cplusplus
#include <string>
#include <util/Logger.hpp>

namespace boost
{
    namespace filesystem
    {
    }
}
namespace fs = boost::filesystem;

#include <jsoncons/json.hpp>
using namespace jsoncons; // I'm such a terrible person

#include "Util.hpp"
#endif

#endif // GLOBAL_HPP
