//bass
//license: ISC
//project started: 2013-09-27

#include "bass.hpp"
#include "core/core.cpp"
//#include "architecture/table/table.cpp"

#include <nall/main.hpp>
auto nall::main(Arguments arguments) -> void {
  if(!arguments) {
    print(stderr, "bass v20\n");
    print(stderr, "\n");
    print(stderr, "usage:\n");
    print(stderr, "  bass [options] source [source ...]\n");
    print(stderr, "\n");
    print(stderr, "options:\n");
    exit(EXIT_FAILURE);
  }

  print("Hello World\n");
}
