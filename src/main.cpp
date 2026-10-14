#include "cube.hpp"
#include "solver.hpp"

#include <iostream>
#include <string>

int main() {
    std::string s = scramble(20);
    Cube cube;
    cube.apply_moves(s);

    std::cout << "scramble: " << s << std::endl;
    cube.print();

    Solver solver(cube);
    std::string solution = solver.solve(25);
    std::cout << "solution: " << solution << std::endl;

    if (!solution.empty())
        cube.apply_moves(solution);
    else if (!cube.is_solved()) {
        std::cout << "no solution found\n";
        return 1;
    }
    cube.print();
}
