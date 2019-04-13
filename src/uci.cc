#include <iterator>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "uci.h"

namespace apollo {

template <typename TOut>
void Split(const std::string& str, char delimiter, TOut out) {
  std::stringstream ss(str);
  std::string token;
  while (std::getline(ss, token, delimiter)) {
    *(out++) = token;
  }
}

void UciServer::Run() {
  std::string line;
  while (true) {
    std::getline(in_, line);
    if (line.empty()) {
      return;
    }
    log_ << "command: " << line << std::endl;
    if (line == "uci") {
      HandleUci();
      continue;
    }
    if (line.rfind("debug ") == 0) {
      HandleDebug();
      continue;
    }
    if (line.rfind("position ") == 0) {
      HandlePosition(line);
      continue;
    }
    if (line.rfind("go") == 0) {
      HandleGo(line);
      continue;
    }
    if (line == "isready") {
      out_ << "readyok" << std::endl;
      continue;
    }
    if (line == "ucinewgame") {
      continue;
    }
    if (line == "stop") {
      continue;
    }
    if (line == "quit") {
      return;
    }
    if (line == "dump") {
      out_ << pos_ << std::endl;
      continue;
    }
    if (line == "dumpfen") {
      out_ << pos_.AsFen() << std::endl;
      continue;
    }
  }
}

void UciServer::HandleUci() {
  // According to the UCI protocol, sending "uci" to us is an invitation to
  // identify ourselves and send "uciok" to acknowledge that we're ready to
  // roll.
  out_ << "id name "
       << "apollo3 0.1" << std::endl;
  out_ << "id author Sean Gillespie" << std::endl;
  out_ << "uciok" << std::endl;
}

void UciServer::HandleDebug() {
  // Don't do anything in particular (yet).
}

void UciServer::HandlePosition(const std::string& line) {
  std::lock_guard lock(position_lock_);
  size_t move_idx = line.find("moves ");
  log_ << "moves: " << move_idx << std::endl;
  if (line.find("startpos") != std::string::npos) {
    log_ << "position: startpos" << std::endl;
    pos_ = Position();
  } else {
    size_t fen_idx = line.find("fen ");
    size_t end_idx = move_idx == std::string::npos ? line.size() : move_idx - 1;
    std::string sub = line.substr(fen_idx + 4, end_idx - fen_idx - 4);
    log_ << "position: fen " << sub << std::endl;
    pos_ = Position(sub);
  }

  if (move_idx != std::string::npos) {
    std::string moves_str = line.substr(move_idx + 6);
    std::vector<std::string> moves;
    Split(moves_str, ' ', std::back_inserter(moves));
    for (auto move : moves) {
      auto parsed_move = pos_.MoveFromUci(move);
      if (parsed_move) {
        log_ << "position: applying move: " << *parsed_move << std::endl;
        pos_.MakeMove(*parsed_move);
        log_ << pos_ << std::endl;
        log_ << pos_.AsFen() << std::endl;
      } else {
        log_ << "position: move " << move << " is invalid" << std::endl;
      }
    }
  }

  log_ << pos_ << std::endl;
}

void UciServer::HandleGo(const std::string& line) {
  // The correct thing to do here is to launch this in another thread.
  // We're being lazy here as we bootstrap the UCI interface.
  std::lock_guard lock(position_lock_);
  search::SearchResult result = searcher_.Search(pos_, 5);
  out_ << "info score cp " << result.score << std::endl;
  out_ << "bestmove " << result.best_move.AsUci() << std::endl;
}

}  // namespace apollo