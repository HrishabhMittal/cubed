#include "cube.hpp"
#define T_PERM "R U R' U' R' F R2 U' R' U' R U R' F'"
#define J_PERM "R U R' F' R U R' U' R' F R2 U' R' U'"
#define A_PERM "R' F R' B2 R F' R' B2 R2"
int main() {
    Cube c;
    c.apply_moves(T_PERM);
    std::cout<<"T_PERM:"<<std::endl;
    c.print();
    c.apply_moves(T_PERM);
    c.apply_moves(J_PERM);
    std::cout<<"J_PERM:"<<std::endl;
    c.print();
    c.apply_moves(J_PERM);
    c.apply_moves(A_PERM);
    std::cout<<"A_PERM:"<<std::endl;
    c.print();
    c.apply_moves(A_PERM);
    c.apply_moves(A_PERM);
    std::cout<<"ORIG:"<<std::endl;
    c.print();
}
