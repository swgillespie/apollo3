#pragma once

#include <iostream>
#include <mutex>

#include "evaluators/shannon_evaluator.h"
#include "position.h"
#include "search/searcher.h"

namespace apollo {

class UciServer {
 public:
  UciServer(std::istream& in, std::ostream& out, std::ostream& log)
      : in_(in),
        out_(out),
        log_(log),
        position_lock_(),
        pos_(),
        searcher_(std::make_unique<evaluators::ShannonEvaluator>()) {}

  void Run();

 private:
  void HandleUci();
  void HandleDebug();
  void HandlePosition(const std::string& line);
  void HandleGo(const std::string& line);

  std::istream& in_;
  std::ostream& out_;
  std::ostream& log_;

  std::mutex position_lock_;
  Position pos_;
  search::Searcher searcher_;
};

}  // namespace apollo