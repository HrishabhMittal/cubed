#pragma once
#include <cstdint>
#include <iostream>
#include <random>
#include <sstream>
#include <string>

enum Move : uint8_t { U1 = 0, U2, U3, D1, D2, D3, R1, R2, R3, L1, L2, L3, F1, F2, F3, B1, B2, B3 };

inline Move get_inverse(Move m) {
    uint8_t face = m / 3;
    uint8_t turns = m % 3;
    if (turns == 0)
        return static_cast<Move>(face * 3 + 2);
    if (turns == 1)
        return m;
    return static_cast<Move>(face * 3 + 0);
}

inline int get_face(Move m) { return m / 3; }

inline Move str_to_move(const std::string &s) {
    int face = 0;
    switch (s[0]) {
    case 'U':
        face = 0;
        break;
    case 'D':
        face = 1;
        break;
    case 'R':
        face = 2;
        break;
    case 'L':
        face = 3;
        break;
    case 'F':
        face = 4;
        break;
    case 'B':
        face = 5;
        break;
    }
    int turns = 0;
    if (s.size() > 1) {
        if (s[1] == '2')
            turns = 1;
        else if (s[1] == '\'')
            turns = 2;
    }
    return static_cast<Move>(face * 3 + turns);
}

inline std::string move_to_str(Move m) {
    const char *faces = "UDRLFB";
    std::string s = "";
    s += faces[m / 3];
    if (m % 3 == 1)
        s += "2";
    if (m % 3 == 2)
        s += "'";
    return s;
}
inline std::string scramble(int num) {
    static std::random_device rd;
    static std::mt19937 mt(rd());
    static std::uniform_int_distribution<int> dist(0, 17);
    std::string out;
    // Face order UDRLFB: opposite faces share an axis (face/2).
    int prev_face = -1;
    int prev_prev_face = -1;
    for (int i = 0; i < num; i++) {
        Move m;
        int face;
        do {
            m = static_cast<Move>(dist(mt));
            face = get_face(m);
            if (face == prev_face)
                continue; // no "... U U2 ..."
            // no "... U D U ..." (three moves on the same axis)
            if (prev_face >= 0 && prev_prev_face >= 0 && face / 2 == prev_face / 2 &&
                prev_face / 2 == prev_prev_face / 2)
                continue;
            break;
        } while (true);
        if (i)
            out += ' ';
        out += move_to_str(m);
        prev_prev_face = prev_face;
        prev_face = face;
    }
    return out;
}
class Cube {
    uint8_t corners[8];
    uint8_t corner_orientation[8];

    uint8_t edges[12];
    uint8_t edge_orientation[12];

    static constexpr char corner_colors[8][3] = {{'W', 'R', 'G'}, {'W', 'G', 'O'}, {'W', 'O', 'B'}, {'W', 'B', 'R'},
                                                 {'Y', 'G', 'R'}, {'Y', 'O', 'G'}, {'Y', 'B', 'O'}, {'Y', 'R', 'B'}};

    static constexpr char edge_colors[12][2] = {{'W', 'R'}, {'W', 'G'}, {'W', 'O'}, {'W', 'B'}, {'Y', 'R'}, {'Y', 'G'},
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

    uint8_t corner(int i) const { return corners[i]; }
    uint8_t corner_ori(int i) const { return corner_orientation[i]; }
    uint8_t edge(int i) const { return edges[i]; }
    uint8_t edge_ori(int i) const { return edge_orientation[i]; }

    void set_corner(int i, uint8_t perm, uint8_t ori) {
        corners[i] = perm;
        corner_orientation[i] = ori;
    }
    void set_edge(int i, uint8_t perm, uint8_t ori) {
        edges[i] = perm;
        edge_orientation[i] = ori;
    }

    bool is_solved() const {
        for (int i = 0; i < 8; ++i)
            if (corners[i] != i || corner_orientation[i] != 0)
                return false;
        for (int i = 0; i < 12; ++i)
            if (edges[i] != i || edge_orientation[i] != 0)
                return false;
        return true;
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

    void apply_move(Move m) {
        switch (m) {
        case U1:
            U();
            break;
        case U2:
            U_2();
            break;
        case U3:
            U_prime();
            break;
        case D1:
            D();
            break;
        case D2:
            D_2();
            break;
        case D3:
            D_prime();
            break;
        case R1:
            R();
            break;
        case R2:
            R_2();
            break;
        case R3:
            R_prime();
            break;
        case L1:
            L();
            break;
        case L2:
            L_2();
            break;
        case L3:
            L_prime();
            break;
        case F1:
            F();
            break;
        case F2:
            F_2();
            break;
        case F3:
            F_prime();
            break;
        case B1:
            B();
            break;
        case B2:
            B_2();
            break;
        case B3:
            B_prime();
            break;
        }
    }

    void apply_move(const std::string &s) { apply_move(str_to_move(s)); }

    void apply_moves(const std::string &s) {
        std::stringstream ss(s);
        std::string st;
        while (ss >> st)
            apply_move(st);
    }

    void apply_random() {
        static std::random_device rd;
        static std::mt19937 mt(rd());
        static std::uniform_int_distribution<int> dist(0, 17);
        apply_move(static_cast<Move>(dist(mt)));
    }

    void scramble(int n) {
        for (int i = 0; i < n; i++)
            apply_random();
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

    // Kept for backward compatibility
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
};
