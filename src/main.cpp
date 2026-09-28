#include "cube.hpp"

#define T_PERM "R U R' U' R' F R2 U' R' U' R U R' F'"
#define J_PERM "R U R' F' R U R' U' R' F R2 U' R' U'"
#define A_PERM "R' F R' B2 R F' R' B2 R2"
// D2 R2 L' R F2 R B2 R' B2 R L2 D2 F' B' U' D' B2 F2 L2 U'
// some problem in scrambling in this scramble
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
    std::string s = scramble(20);
    c.apply_moves(s);
    std::cout<<"scramble: "<<s<<std::endl;
    c.print();
    return 0;
}
