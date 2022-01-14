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
    print(stderr, "bass ", plek.appLabel,"\n");
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

  vector<string> constants;
  string constant;
  while(arguments.take("-c", constant)) constants.append(constant);

  vector<string> sourceFilenames;
  for(auto& argument : arguments) sourceFilenames.append(argument);

  clock_t clockStart = clock();

  // core application
  plek.init();
  try {
    for(auto& c : constants) {
      auto p = c.split("=", 1L);
      plek.constant(p(0), p(1,"1"));
    }

    for(auto& sourceFilename : sourceFilenames) {
      plek.load(sourceFilename);
    }
    plek.target(targetFilename, create);

    plek.execute();
  } catch(string e) {
    plek.error(e);
    exit(EXIT_FAILURE);
  }

  clock_t clockFinish = clock();
}