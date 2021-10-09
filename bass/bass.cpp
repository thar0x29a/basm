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

  //todo: handle options

  vector<string> sourceFilenames;
  for(auto& argument : arguments) sourceFilenames.append(argument);

  clock_t clockStart = clock();
  Bass::Plek plek;
  //plek.target(targetFilename, create);

  // loat sources:
  for(auto& sourceFilename : sourceFilenames) {
    plek.load(sourceFilename);
  }
}