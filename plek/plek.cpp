//bass
//license: ISC
//project started: 2013-09-27

#include "plek.hpp"
#include "core/core.cpp"
//#include "architecture/table/table.cpp"

// global instance that we all going to use
Bass::Plek plek;  

#include <nall/main.hpp>
auto nall::main(Arguments arguments) -> void {
  string targetFilename;
  string loglevel;
  bool create = false;

  if(!arguments) {
    print(stderr, "plek ", plek.appLabel,"\n");
    print(stderr, "\n");
    print(stderr, "usage:\n");
    print(stderr, "  plek [options] source [source ...]\n");
    print(stderr, "\n");
    print(stderr, "options:\n");
    print(stderr, "  -o target        specify default output filename [overwrite]\n");
    print(stderr, "  -m target        specify default output filename [modify]\n");
    print(stderr, "  -c name[=value]  create constant with optional value\n");
    print(stderr, "  -l level         set log level (All=0, Debug, Notice, Warn, Error)\n");
    exit(EXIT_FAILURE);
  }

  //todo: handle options
  if(arguments.take("-o", targetFilename)) create = true;
  if(arguments.take("-m", targetFilename)) create = false;
  if(arguments.take("-l", loglevel)) {
    Bass::LogLevel lvl;

    if(loglevel.equals("0")) lvl = Bass::LogLevel::LEVEL_ALL;
    else if(loglevel.equals("1")) lvl = Bass::LogLevel::LEVEL_DEBUG;
    else if(loglevel.equals("2")) lvl = Bass::LogLevel::LEVEL_NOTICE;
    else if(loglevel.equals("3")) lvl = Bass::LogLevel::LEVEL_WARN;
    else if(loglevel.equals("4")) lvl = Bass::LogLevel::LEVEL_ERROR;
    else if(loglevel.equals("5")) lvl = Bass::LogLevel::LEVEL_FATAL;
    else if(loglevel.equals("6")) lvl = Bass::LogLevel::LEVEL_OFF;
    else lvl = Bass::LogLevel::LEVEL_NOTICE;

    plek.log_level = lvl;
  }
  

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

    if(plek.execute() == false) {
      exit(EXIT_FAILURE);
    }
  } catch(string e) {
    plek.error(e);
    exit(EXIT_FAILURE);
  }

  clock_t clockFinish = clock();
}