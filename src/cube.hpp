#include <cstdint>
#include <iostream>
#include <random>
#include <sstream>
#include <string>

class Cube {
    uint8_t corners[8];
    uint8_t corner_orientation[8];

    uint8_t edges[12];
    uint8_t edge_orientation[12];

    const char corner_colors[8][3] = {{'W', 'R', 'G'}, {'W', 'G', 'O'}, {'W', 'O', 'B'}, {'W', 'B', 'R'},
                                      {'Y', 'G', 'R'}, {'Y', 'O', 'G'}, {'Y', 'B', 'O'}, {'Y', 'R', 'B'}};

    const char edge_colors[12][2] = {{'W', 'R'}, {'W', 'G'}, {'W', 'O'}, {'W', 'B'}, {'Y', 'R'}, {'Y', 'G'},
                                     {'Y', 'O'}, {'Y', 'B'}, {'G', 'R'}, {'G', 'O'}, {'B', 'O'}, {'B', 'R'}};

    char getCornerColor(int pos, int face) const {
        int c = corners[pos];
        int o = corner_orientation[pos];
        int orig_face = (face - o + 3) % 3;
        return corner_colors[c][orig_face];
    }

    char getEdgeColor(int pos, int face) const {
        int e = edges[pos];
        int o = edge_orientation[pos];
        int orig_face = (face ^ o);
        return edge_colors[e][orig_face];
    }

    std::string colorStr(char c) const {
        switch (c) {
        case 'W':
            return "\033[48;2;255;255;255m  \033[0m";
        case 'Y':
            return "\033[48;2;255;255;0m  \033[0m";
        case 'G':
            return "\033[48;2;0;255;0m  \033[0m";
        case 'B':
            return "\033[48;2;30;144;255m  \033[0m";
        case 'R':
            return "\033[48;2;255;50;50m  \033[0m";
        case 'O':
            return "\033[48;2;255;165;0m  \033[0m";
        }
        return ":(";
    }

    void cycle_corners(int a, int b, int c, int d, int twist_a, int twist_b, int twist_c, int twist_d) {
        uint8_t t_c = corners[d], t_o = corner_orientation[d];
        corners[d] = corners[c];
        corner_orientation[d] = corner_orientation[c];
        corners[c] = corners[b];
        corner_orientation[c] = corner_orientation[b];
        corners[b] = corners[a];
        corner_orientation[b] = corner_orientation[a];
        corners[a] = t_c;
        corner_orientation[a] = t_o;

        corner_orientation[a] = (corner_orientation[a] + twist_a) % 3;
        corner_orientation[b] = (corner_orientation[b] + twist_b) % 3;
        corner_orientation[c] = (corner_orientation[c] + twist_c) % 3;
        corner_orientation[d] = (corner_orientation[d] + twist_d) % 3;
    }

    void cycle_edges(int a, int b, int c, int d, int flip) {
        uint8_t t_c = edges[d], t_o = edge_orientation[d];
        edges[d] = edges[c];
        edge_orientation[d] = edge_orientation[c];
        edges[c] = edges[b];
        edge_orientation[c] = edge_orientation[b];
        edges[b] = edges[a];
        edge_orientation[b] = edge_orientation[a];
        edges[a] = t_c;
        edge_orientation[a] = t_o;

        if (flip) {
            edge_orientation[a] ^= 1;
            edge_orientation[b] ^= 1;
            edge_orientation[c] ^= 1;
            edge_orientation[d] ^= 1;
        }
    }

    void apply_random() {
        static std::random_device rd;
        static std::mt19937 mt(rd());
        static std::uniform_int_distribution<int> dist_m(1, 6);
        static std::uniform_int_distribution<int> dist_n(1, 3);
        int move = dist_m(mt);
        int num = dist_n(mt);
        switch (move) {
        case 0:
            R(num);
            break;
        case 1:
            L(num);
            break;
        case 2:
            F(num);
            break;
        case 3:
            B(num);
            break;
        case 4:
            U(num);
            break;
        case 5:
            D(num);
            break;
        default:
            break;
        }
    }

  public:
    Cube() {
        for (int i = 0; i < 8; ++i) {
            corners[i] = i;
            corner_orientation[i] = 0;
        }
        for (int i = 0; i < 12; ++i) {
            edges[i] = i;
            edge_orientation[i] = 0;
        }
    }

    void print() const {
        char f[6][9];

        f[0][0] = getCornerColor(2, 0);
        f[0][1] = getEdgeColor(3, 0);
        f[0][2] = getCornerColor(3, 0);
        f[0][3] = getEdgeColor(2, 0);
        f[0][4] = 'W';
        f[0][5] = getEdgeColor(0, 0);
        f[0][6] = getCornerColor(1, 0);
        f[0][7] = getEdgeColor(1, 0);
        f[0][8] = getCornerColor(0, 0);

        f[1][0] = getCornerColor(2, 1);
        f[1][1] = getEdgeColor(2, 1);
        f[1][2] = getCornerColor(1, 2);
        f[1][3] = getEdgeColor(10, 1);
        f[1][4] = 'O';
        f[1][5] = getEdgeColor(9, 1);
        f[1][6] = getCornerColor(6, 2);
        f[1][7] = getEdgeColor(6, 1);
        f[1][8] = getCornerColor(5, 1);

        f[2][0] = getCornerColor(1, 1);
        f[2][1] = getEdgeColor(1, 1);
        f[2][2] = getCornerColor(0, 2);
        f[2][3] = getEdgeColor(9, 0);
        f[2][4] = 'G';
        f[2][5] = getEdgeColor(8, 0);
        f[2][6] = getCornerColor(5, 2);
        f[2][7] = getEdgeColor(5, 1);
        f[2][8] = getCornerColor(4, 1);

        f[3][0] = getCornerColor(0, 1);
        f[3][1] = getEdgeColor(0, 1);
        f[3][2] = getCornerColor(3, 2);
        f[3][3] = getEdgeColor(8, 1);
        f[3][4] = 'R';
        f[3][5] = getEdgeColor(11, 1);
        f[3][6] = getCornerColor(4, 2);
        f[3][7] = getEdgeColor(4, 1);
        f[3][8] = getCornerColor(7, 1);

        f[4][0] = getCornerColor(3, 1);
        f[4][1] = getEdgeColor(3, 1);
        f[4][2] = getCornerColor(2, 2);
        f[4][3] = getEdgeColor(11, 0);
        f[4][4] = 'B';
        f[4][5] = getEdgeColor(10, 0);
        f[4][6] = getCornerColor(7, 2);
        f[4][7] = getEdgeColor(7, 1);
        f[4][8] = getCornerColor(6, 1);

        f[5][0] = getCornerColor(5, 0);
        f[5][1] = getEdgeColor(5, 0);
        f[5][2] = getCornerColor(4, 0);
        f[5][3] = getEdgeColor(6, 0);
        f[5][4] = 'Y';
        f[5][5] = getEdgeColor(4, 0);
        f[5][6] = getCornerColor(6, 0);
        f[5][7] = getEdgeColor(7, 0);
        f[5][8] = getCornerColor(7, 0);

        std::string sp = "      ";

        for (int i = 0; i < 3; ++i)
            std::cout << sp << colorStr(f[0][i * 3]) << colorStr(f[0][i * 3 + 1]) << colorStr(f[0][i * 3 + 2]) << "\n";

        for (int i = 0; i < 3; ++i) {
            for (int j = 1; j <= 4; ++j)
                std::cout << colorStr(f[j][i * 3]) << colorStr(f[j][i * 3 + 1]) << colorStr(f[j][i * 3 + 2]);
            std::cout << "\n";
        }

        for (int i = 0; i < 3; ++i)
            std::cout << sp << colorStr(f[5][i * 3]) << colorStr(f[5][i * 3 + 1]) << colorStr(f[5][i * 3 + 2]) << "\n";
    }

    void U() {
        cycle_corners(0, 1, 2, 3, 0, 0, 0, 0);
        cycle_edges(0, 1, 2, 3, 0);
    }
    void D() {
        cycle_corners(4, 7, 6, 5, 0, 0, 0, 0);
        cycle_edges(4, 7, 6, 5, 0);
    }
    void R() {
        cycle_corners(0, 3, 7, 4, 2, 1, 2, 1);
        cycle_edges(0, 11, 4, 8, 0);
    }
    void L() {
        cycle_corners(2, 1, 5, 6, 2, 1, 2, 1);
        cycle_edges(2, 9, 6, 10, 0);
    }
    void F() {
        cycle_corners(1, 0, 4, 5, 2, 1, 2, 1);
        cycle_edges(1, 8, 5, 9, 1);
    }

    void B() {
        cycle_corners(3, 2, 6, 7, 2, 1, 2, 1);
        cycle_edges(3, 10, 7, 11, 1);
    }
    void apply_move(std::string s) {
        int num = 1;
        if (s.size() == 2) {
            if (s[1] == '2')
                num = 2;
            else if (s[1] == '\'')
                num = 3;
        }
        switch (s[0]) {
        case 'R':
            R(num);
            break;
        case 'L':
            L(num);
            break;
        case 'F':
            F(num);
            break;
        case 'B':
            B(num);
            break;
        case 'U':
            U(num);
            break;
        case 'D':
            D(num);
            break;
        default:
            break;
        }
    }
    void scramble(int n) {
        for (int i = 0; i < n; i++)
            apply_random();
    }
    void apply_moves(std::string s) {
        std::stringstream ss;
        ss << s;
        std::string st;
        while (ss >> st) {
            apply_move(st);
        }
    }
    void U(int n) {
        for (int i = 0; i < n; i++)
            U();
    }
    void D(int n) {
        for (int i = 0; i < n; i++)
            D();
    }
    void R(int n) {
        for (int i = 0; i < n; i++)
            R();
    }
    void L(int n) {
        for (int i = 0; i < n; i++)
            L();
    }
    void F(int n) {
        for (int i = 0; i < n; i++)
            F();
    }
    void B(int n) {
        for (int i = 0; i < n; i++)
            B();
    }
    void R_2() {
        R();
        R();
    }
    void R_prime() {
        R();
        R();
        R();
    }
    void L_2() {
        L();
        L();
    }
    void L_prime() {
        L();
        L();
        L();
    }
    void U_2() {
        U();
        U();
    }
    void U_prime() {
        U();
        U();
        U();
    }
    void D_2() {
        D();
        D();
    }
    void D_prime() {
        D();
        D();
        D();
    }
    void F_2() {
        F();
        F();
    }
    void F_prime() {
        F();
        F();
        F();
    }
    void B_2() {
        B();
        B();
    }
    void B_prime() {
        B();
        B();
        B();
    }
};
