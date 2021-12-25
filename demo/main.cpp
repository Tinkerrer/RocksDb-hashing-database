// Copyright 2021 Tinkerrer

#include <boost/log/expressions.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/program_options.hpp>
#include <boost/shared_ptr.hpp>

#include "RockDatabase.h"

using namespace boost::program_options;

namespace logging = boost::log;
namespace sinks = boost::log::sinks;

void ConfiguringLog(const std::string& trivial_level) {
  typedef sinks::synchronous_sink<sinks::text_ostream_backend> sink_console;
  // adding console-sink to core
  boost::shared_ptr<sink_console> c_sink = logging::add_console_log();
  // setting format and filter
  if (trivial_level == "info") {
    c_sink->set_filter(logging::trivial::severity >= logging::trivial::info);
  }
  if (trivial_level == "error") {
    c_sink->set_filter(logging::trivial::severity == logging::trivial::error);
  }
}

int main(int argc, char* argv[]) {
  // Process args
  options_description desc{"Options"};
  desc.add_options()("help", "produce help message")(
      "log_level", value<std::string>()->default_value("error"),
      "\"error\" | \"info\"")("output",
                              value<std::string>()->default_value("dbcs"),
                              "name of output DB");

  variables_map vm;
  store(parse_command_line(argc, argv, desc), vm);
  notify(vm);

  if (vm.count("help")) {
    std::cout << desc << "\n";
    return 1;
  }

  // Configure log
  ConfiguringLog(vm["log_level"].as<std::string>());

  // vm["output"].as<std::string>() - имя файла для записи хэшей
  // argv[1] - имя файла из которого берутся данные для хэшей
  RockDatabase db1(argv[1]);
  RockDatabase db2("vm[\"output\"].as<std::string>()");

  db1.copy_hashed(db2);
  db2.close();
  db2.open("../test2.db");
  db2.print();
}
