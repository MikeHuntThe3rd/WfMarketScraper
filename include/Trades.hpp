#pragma once
#include "VARS.hpp"
#include <optional>
#include <string>
#include <vector>

using namespace VARS;
class Trades {
public:
  std::vector<Remotetrade> Read();
  void Add(Remotetrade trade);
  void Remove(std::string id);
  void Edit(std::string id, Remotetrade trade);

private:
  std::optional<std::vector<VARS::Remotetrade>::iterator>
  FindById(std::string id);
  std::vector<VARS::Remotetrade> Trades_inner;
  std::mutex thread_lock_key;
  std::condition_variable cv;
};

inline Trades trades{};
