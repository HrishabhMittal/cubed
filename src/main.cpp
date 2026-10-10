#include "cube.hpp"

#define T_PERM "R U R' U' R' F R2 U' R' U' R U R' F'"
#define J_PERM "R U R' F' R U R' U' R' F R2 U' R' U'"
#define A_PERM "R' F R' B2 R F' R' B2 R2"
int main() {
    Cube c;
    for (int i=0;i<18;i++) {
        c.apply_move(static_cast<Move>(i));
        c.print();
        c.apply_move(static_cast<Move>(i));
        c.apply_move(static_cast<Move>(i));
        c.apply_move(static_cast<Move>(i));
    }
    c.apply_moves(T_PERM);
    c.print();
    c.apply_moves(T_PERM);
    c.apply_moves(J_PERM);
    c.print();
    c.apply_moves(J_PERM);

    Cube scrambled;
    std::string s = scramble(20);
    scrambled.apply_moves(s);
    std::cout << "scramble: " << s << std::endl;
    scrambled.print();
    return 0;
}
