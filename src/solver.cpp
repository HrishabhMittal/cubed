#include "solver.hpp"

#include <algorithm>
#include <cstring>
#include <queue>
#include <vector>

namespace {

constexpr int N_TWIST = 2187;
constexpr int N_FLIP = 2048;
constexpr int N_SLICE = 495;
constexpr int N_CPERM = 40320;
constexpr int N_EPERM = 40320;
constexpr int N_MPERM = 24;

constexpr int N_MOVES = 18;
constexpr int N_P2_MOVES = 10;

constexpr Move P2_MOVES[N_P2_MOVES] = {U1, U2, U3, D1, D2, D3, R2, L2, F2, B2};

int binom(int n, int k) {
    if (k < 0 || n < k)
        return 0;
    int r = 1;
    for (int i = 1; i <= k; ++i)
        r = r * (n - k + i) / i;
    return r;
}

int fact(int n) {
    int r = 1;
    for (int i = 2; i <= n; ++i)
        r *= i;
    return r;
}

int perm_coord(const int *p, int n) {
    int x = 0;
    for (int i = 0; i < n; ++i) {
        int s = 0;
        for (int j = i + 1; j < n; ++j)
            if (p[j] < p[i])
                ++s;
        x = x * (n - i) + s;
    }
    return x;
}

void set_perm(int *p, int n, int idx) {
    std::vector<int> unused(n);
    for (int i = 0; i < n; ++i)
        unused[i] = i;
    for (int i = 0; i < n; ++i) {
        int f = fact(n - 1 - i);
        int sel = idx / f;
        idx %= f;
        p[i] = unused[sel];
        unused.erase(unused.begin() + sel);
    }
}

int twist_coord(const Cube &c) {
    int t = 0;
    for (int i = 0; i < 7; ++i)
        t = 3 * t + c.corner_ori(i);
    return t;
}

void set_twist(Cube &c, int twist) {
    int sum = 0;
    for (int i = 6; i >= 0; --i) {
        int o = twist % 3;
        c.set_corner(i, c.corner(i), static_cast<uint8_t>(o));
        sum += o;
        twist /= 3;
    }
    c.set_corner(7, c.corner(7), static_cast<uint8_t>((3 - sum % 3) % 3));
}

int flip_coord(const Cube &c) {
    int f = 0;
    for (int i = 0; i < 11; ++i)
        f = 2 * f + c.edge_ori(i);
    return f;
}

void set_flip(Cube &c, int flip) {
    int sum = 0;
    for (int i = 10; i >= 0; --i) {
        int o = flip & 1;
        c.set_edge(i, c.edge(i), static_cast<uint8_t>(o));
        sum ^= o;
        flip >>= 1;
    }
    c.set_edge(11, c.edge(11), static_cast<uint8_t>(sum));
}

int slice_coord(const Cube &c) {
    bool occupied[12] = {};
    for (int i = 0; i < 12; ++i)
        if (c.edge(i) >= 8)
            occupied[i] = true;
    int s = 0, k = 3, n = 11;
    while (k >= 0) {
        if (occupied[n])
            --k;
        else
            s += binom(n, k);
        --n;
    }
    return s;
}

void set_slice(Cube &c, int slice) {
    bool occupied[12] = {};
    int k = 3, n = 11;
    while (k >= 0) {
        int cnk = binom(n, k);
        if (slice >= cnk) {
            slice -= cnk;
            --n;
        } else {
            occupied[n] = true;
            --k;
            --n;
        }
    }
    int slice_edge = 8;
    int other = 0;
    for (int i = 0; i < 12; ++i) {
        if (occupied[i])
            c.set_edge(i, static_cast<uint8_t>(slice_edge++), 0);
        else
            c.set_edge(i, static_cast<uint8_t>(other++), 0);
    }
}

int cperm_coord(const Cube &c) {
    int p[8];
    for (int i = 0; i < 8; ++i)
        p[i] = c.corner(i);
    return perm_coord(p, 8);
}

void set_cperm(Cube &c, int idx) {
    int p[8];
    set_perm(p, 8, idx);
    for (int i = 0; i < 8; ++i)
        c.set_corner(i, static_cast<uint8_t>(p[i]), c.corner_ori(i));
}

int eperm_coord(const Cube &c) {
    int p[8];
    for (int i = 0; i < 8; ++i)
        p[i] = c.edge(i);
    return perm_coord(p, 8);
}

void set_eperm(Cube &c, int idx) {
    int p[8];
    set_perm(p, 8, idx);
    for (int i = 0; i < 8; ++i)
        c.set_edge(i, static_cast<uint8_t>(p[i]), c.edge_ori(i));
    for (int i = 8; i < 12; ++i)
        c.set_edge(i, static_cast<uint8_t>(i), c.edge_ori(i));
}

int mperm_coord(const Cube &c) {
    int arr[4];
    int j = 0;
    for (int i = 0; i < 12; ++i) {
        int e = c.edge(i);
        if (e >= 8)
            arr[j++] = e - 8;
    }
    return perm_coord(arr, 4);
}

void set_mperm(Cube &c, int idx) {
    int arr[4];
    set_perm(arr, 4, idx);
    for (int i = 0; i < 4; ++i)
        c.set_edge(8 + i, static_cast<uint8_t>(arr[i] + 8), 0);
}

uint16_t twist_move[N_TWIST][N_MOVES];
uint16_t flip_move[N_FLIP][N_MOVES];
uint16_t slice_move[N_SLICE][N_MOVES];
uint16_t cperm_move[N_CPERM][N_P2_MOVES];
uint16_t eperm_move[N_EPERM][N_P2_MOVES];
uint16_t mperm_move[N_MPERM][N_P2_MOVES];

uint8_t twist_slice_prun[N_TWIST][N_SLICE];
uint8_t flip_slice_prun[N_FLIP][N_SLICE];
uint8_t cperm_prun[N_CPERM];
uint8_t eperm_prun[N_EPERM];
uint8_t mperm_prun[N_MPERM];

bool tables_ready = false;

void init_move_tables() {
    for (int i = 0; i < N_TWIST; ++i) {
        Cube c;
        set_twist(c, i);
        for (int m = 0; m < N_MOVES; ++m) {
            Cube t = c;
            t.apply_move(static_cast<Move>(m));
            twist_move[i][m] = static_cast<uint16_t>(twist_coord(t));
        }
    }
    for (int i = 0; i < N_FLIP; ++i) {
        Cube c;
        set_flip(c, i);
        for (int m = 0; m < N_MOVES; ++m) {
            Cube t = c;
            t.apply_move(static_cast<Move>(m));
            flip_move[i][m] = static_cast<uint16_t>(flip_coord(t));
        }
    }
    for (int i = 0; i < N_SLICE; ++i) {
        Cube c;
        set_slice(c, i);
        for (int m = 0; m < N_MOVES; ++m) {
            Cube t = c;
            t.apply_move(static_cast<Move>(m));
            slice_move[i][m] = static_cast<uint16_t>(slice_coord(t));
        }
    }
    for (int i = 0; i < N_CPERM; ++i) {
        Cube c;
        set_cperm(c, i);
        for (int m = 0; m < N_P2_MOVES; ++m) {
            Cube t = c;
            t.apply_move(P2_MOVES[m]);
            cperm_move[i][m] = static_cast<uint16_t>(cperm_coord(t));
        }
    }
    for (int i = 0; i < N_EPERM; ++i) {
        Cube c;
        set_eperm(c, i);
        for (int m = 0; m < N_P2_MOVES; ++m) {
            Cube t = c;
            t.apply_move(P2_MOVES[m]);
            eperm_move[i][m] = static_cast<uint16_t>(eperm_coord(t));
        }
    }
    for (int i = 0; i < N_MPERM; ++i) {
        Cube c;
        set_mperm(c, i);
        for (int m = 0; m < N_P2_MOVES; ++m) {
            Cube t = c;
            t.apply_move(P2_MOVES[m]);
            mperm_move[i][m] = static_cast<uint16_t>(mperm_coord(t));
        }
    }
}

void bfs_twist_slice() {
    std::memset(twist_slice_prun, 0xFF, sizeof(twist_slice_prun));
    std::queue<std::pair<int, int>> q;
    twist_slice_prun[0][0] = 0;
    q.push({0, 0});
    while (!q.empty()) {
        auto [tw, sl] = q.front();
        q.pop();
        int d = twist_slice_prun[tw][sl];
        for (int m = 0; m < N_MOVES; ++m) {
            int ntw = twist_move[tw][m];
            int nsl = slice_move[sl][m];
            if (twist_slice_prun[ntw][nsl] == 0xFF) {
                twist_slice_prun[ntw][nsl] = static_cast<uint8_t>(d + 1);
                q.push({ntw, nsl});
            }
        }
    }
}

void bfs_flip_slice() {
    std::memset(flip_slice_prun, 0xFF, sizeof(flip_slice_prun));
    std::queue<std::pair<int, int>> q;
    flip_slice_prun[0][0] = 0;
    q.push({0, 0});
    while (!q.empty()) {
        auto [fl, sl] = q.front();
        q.pop();
        int d = flip_slice_prun[fl][sl];
        for (int m = 0; m < N_MOVES; ++m) {
            int nfl = flip_move[fl][m];
            int nsl = slice_move[sl][m];
            if (flip_slice_prun[nfl][nsl] == 0xFF) {
                flip_slice_prun[nfl][nsl] = static_cast<uint8_t>(d + 1);
                q.push({nfl, nsl});
            }
        }
    }
}

void bfs_phase2_prun() {
    std::memset(cperm_prun, 0xFF, sizeof(cperm_prun));
    std::memset(eperm_prun, 0xFF, sizeof(eperm_prun));
    std::memset(mperm_prun, 0xFF, sizeof(mperm_prun));

    std::queue<int> q;
    cperm_prun[0] = 0;
    q.push(0);
    while (!q.empty()) {
        int cp = q.front();
        q.pop();
        int d = cperm_prun[cp];
        for (int m = 0; m < N_P2_MOVES; ++m) {
            int ncp = cperm_move[cp][m];
            if (cperm_prun[ncp] == 0xFF) {
                cperm_prun[ncp] = static_cast<uint8_t>(d + 1);
                q.push(ncp);
            }
        }
    }

    eperm_prun[0] = 0;
    q.push(0);
    while (!q.empty()) {
        int ep = q.front();
        q.pop();
        int d = eperm_prun[ep];
        for (int m = 0; m < N_P2_MOVES; ++m) {
            int nep = eperm_move[ep][m];
            if (eperm_prun[nep] == 0xFF) {
                eperm_prun[nep] = static_cast<uint8_t>(d + 1);
                q.push(nep);
            }
        }
    }

    mperm_prun[0] = 0;
    q.push(0);
    while (!q.empty()) {
        int mp = q.front();
        q.pop();
        int d = mperm_prun[mp];
        for (int m = 0; m < N_P2_MOVES; ++m) {
            int nmp = mperm_move[mp][m];
            if (mperm_prun[nmp] == 0xFF) {
                mperm_prun[nmp] = static_cast<uint8_t>(d + 1);
                q.push(nmp);
            }
        }
    }
}

void init_tables() {
    if (tables_ready)
        return;
    init_move_tables();
    bfs_twist_slice();
    bfs_flip_slice();
    bfs_phase2_prun();
    tables_ready = true;
}

bool same_axis(int f1, int f2) { return f1 / 2 == f2 / 2; }

bool move_allowed(int move, int last_move) {
    if (last_move < 0)
        return true;
    int f = move / 3, lf = last_move / 3;
    if (f == lf)
        return false;
    if (same_axis(f, lf) && f < lf)
        return false;
    return true;
}

bool move_allowed_p2(int p2_idx, int last_p2_idx) {
    if (last_p2_idx < 0)
        return true;
    int f = P2_MOVES[p2_idx] / 3;
    int lf = P2_MOVES[last_p2_idx] / 3;
    if (f == lf)
        return false;
    if (same_axis(f, lf) && f < lf)
        return false;
    return true;
}

std::vector<Move> cancel_moves(std::vector<Move> moves) {
    std::vector<Move> out;
    for (Move m : moves) {
        if (out.empty() || get_face(out.back()) != get_face(m)) {
            out.push_back(m);
            continue;
        }
        int turns = (out.back() % 3 + 1 + m % 3 + 1) % 4;
        out.pop_back();
        if (turns != 0)
            out.push_back(static_cast<Move>(get_face(m) * 3 + (turns - 1)));
    }
    return out;
}

std::string path_to_string(const std::vector<Move> &a, const std::vector<Move> &b) {
    std::vector<Move> all = a;
    all.insert(all.end(), b.begin(), b.end());
    all = cancel_moves(std::move(all));
    std::string out;
    for (Move m : all) {
        if (!out.empty())
            out += ' ';
        out += move_to_str(m);
    }
    return out;
}

struct SearchContext {
    Cube start;
    int max_depth = 30;
    std::vector<Move> sol1;
    std::vector<Move> sol2;

    Cube apply_path(const std::vector<Move> &path) const {
        Cube c = start;
        for (Move m : path)
            c.apply_move(m);
        return c;
    }

    bool search2(int cp, int ep, int mp, int depth, int last_p2) {
        if (depth == 0)
            return cp == 0 && ep == 0 && mp == 0;
        int h = std::max({(int)cperm_prun[cp], (int)eperm_prun[ep], (int)mperm_prun[mp]});
        if (h > depth)
            return false;
        for (int m = 0; m < N_P2_MOVES; ++m) {
            if (!move_allowed_p2(m, last_p2))
                continue;
            sol2.push_back(P2_MOVES[m]);
            if (search2(cperm_move[cp][m], eperm_move[ep][m], mperm_move[mp][m], depth - 1, m))
                return true;
            sol2.pop_back();
        }
        return false;
    }

    bool try_phase2(int remaining) {
        Cube g1 = apply_path(sol1);
        int cp = cperm_coord(g1);
        int ep = eperm_coord(g1);
        int mp = mperm_coord(g1);
        sol2.clear();
        int h = std::max({(int)cperm_prun[cp], (int)eperm_prun[ep], (int)mperm_prun[mp]});
        if (h > remaining)
            return false;
        for (int d = h; d <= remaining; ++d)
            if (search2(cp, ep, mp, d, -1))
                return true;
        return false;
    }

    bool search1(int tw, int fl, int sl, int depth, int last_move) {
        if (depth == 0) {
            if (tw != 0 || fl != 0 || sl != 0)
                return false;
            return try_phase2(max_depth - (int)sol1.size());
        }
        int h = std::max((int)twist_slice_prun[tw][sl], (int)flip_slice_prun[fl][sl]);
        if (h > depth)
            return false;
        for (int m = 0; m < N_MOVES; ++m) {
            if (!move_allowed(m, last_move))
                continue;
            sol1.push_back(static_cast<Move>(m));
            if (search1(twist_move[tw][m], flip_move[fl][m], slice_move[sl][m], depth - 1, m))
                return true;
            sol1.pop_back();
        }
        return false;
    }

    std::string solve() {
        init_tables();
        if (start.is_solved())
            return "";

        int tw = twist_coord(start);
        int fl = flip_coord(start);
        int sl = slice_coord(start);

        if (tw == 0 && fl == 0 && sl == 0) {
            sol1.clear();
            if (try_phase2(max_depth))
                return path_to_string(sol1, sol2);
            return "";
        }

        for (int depth1 = 0; depth1 <= max_depth; ++depth1) {
            sol1.clear();
            if (search1(tw, fl, sl, depth1, -1))
                return path_to_string(sol1, sol2);
        }
        return "";
    }
};

} // namespace

Solver::Solver(const Cube &cube) : cube_(cube) {}

std::string Solver::solve(int max_depth) {
    SearchContext ctx;
    ctx.start = cube_;
    ctx.max_depth = max_depth;
    return ctx.solve();
}
