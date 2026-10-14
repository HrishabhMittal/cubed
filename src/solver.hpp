#pragma once
#include "cube.hpp"
#include <string>

class Solver {
  public:
    explicit Solver(const Cube &cube);

    std::string solve(int max_depth = 30);

  private:
    Cube cube_;
};
