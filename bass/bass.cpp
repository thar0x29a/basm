//bass
//license: ISC
//project started: 2013-09-27

#include "bass.hpp"
#include "core/core.cpp"
//#include "architecture/table/table.cpp"

// global instance that we all going to use
Bass::Plek plek;  

#include <nall/main.hpp>
auto nall::main(Arguments arguments) -> void {
  string targetFilename;
  bool create = false;

  if(!arguments) {
    print(stderr, "bass v20 - plek\n");
    print(stderr, "\n");
    print(stderr, "usage:\n");
    print(stderr, "  bass [options] source [source ...]\n");
    print(stderr, "\n");
    print(stderr, "options:\n");
    print(stderr, "  -o target        specify default output filename [overwrite]\n");
    print(stderr, "  -m target        specify default output filename [modify]\n");
    print(stderr, "  -c name[=value]  create constant with optional value\n");
    exit(EXIT_FAILURE);
  }

  //todo: handle options
  if(arguments.take("-o", targetFilename)) create = true;
  if(arguments.take("-m", targetFilename)) create = false;

  vector<string> sourceFilenames;
  for(auto& argument : arguments) sourceFilenames.append(argument);

  vector<string> constants;
  string constant;
  while(arguments.take("-c", constant)) constants.append(constant);

  clock_t clockStart = clock();

  // core application
  for(auto& sourceFilename : sourceFilenames) {
    plek.load(sourceFilename);
  }
  plek.target(targetFilename, create);
  
  try {
    plek.execute();
  } catch(...) {
    
  }

  clock_t clockFinish = clock();
}