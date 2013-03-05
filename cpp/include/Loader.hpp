#ifndef __ponyC__Loader__
#define __ponyC__Loader__

#include <boost/filesystem.hpp>
#include "CompilationUnit.hpp"

namespace fs = boost::filesystem;

class Loader {

  public:
    static CompilationUnit* Load(std::string,std::string);
    static CompilationUnit* Load(std::string, int stage);
};

#endif