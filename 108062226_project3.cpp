#include <iostream>
#include <fstream>
#include <array>
#include <vector>
#include <cstdlib>
#include <ctime>

#define max(a, b) a > b ? a : b
#define min(a, b) a < b ? a : b
#define MAX_DEPTH 7

struct Point {
    int x, y;
    Point() : Point(0, 0) {}
	Point(float x, float y) : x(x), y(y) {}
	bool operator==(const Point& rhs) const {
		return x == rhs.x && y == rhs.y;
	}
	bool operator!=(const Point& rhs) const {
		return !operator==(rhs);
	}
	Point operator+(const Point& rhs) const {
		return Point(x + rhs.x, y + rhs.y);
	}
	Point operator-(const Point& rhs) const {
		return Point(x - rhs.x, y - rhs.y);
	}
};

int player;
const int SIZE = 8;
std::array<std::array<int, SIZE>, SIZE> board;
std::vector<Point> NEXT_valid_spots;


class OthelloBoard {
public:
    enum SPOT_STATE {
        EMPTY = 0,
        BLACK = 1,
        WHITE = 2
    };
    static const int SIZE = 8;
    const std::array<Point, 8> directions{{
        Point(-1, -1), Point(-1, 0), Point(-1, 1),
        Point(0, -1), /*{0, 0}, */Point(0, 1),
        Point(1, -1), Point(1, 0), Point(1, 1)
    }};
    std::array<std::array<int, SIZE>, SIZE> board;
    std::vector<Point> next_valid_spots;
    // counters for black/white/total
    std::array<int, 3> disc_count;
    int cur_player;
    bool done;
    int winner;
private:
    int get_next_player(int player) const {
        return 3 - player;
    }
    bool is_spot_on_board(Point p) const {
        return 0 <= p.x && p.x < SIZE && 0 <= p.y && p.y < SIZE;
    }
    int get_disc(Point p) const {
        return board[p.x][p.y];
    }
    void set_disc(Point p, int disc) {
        board[p.x][p.y] = disc;
    }

    bool is_disc_at(Point p, int disc) const {
        if (!is_spot_on_board(p))
            return false;
        if (get_disc(p) != disc)
            return false;
        return true;
    }

    bool is_spot_valid(Point center) const {
        if (get_disc(center) != EMPTY)
            return false;
        for (Point dir: directions) {
            // Move along the direction while testing.
            Point p = center + dir;
            if (!is_disc_at(p, get_next_player(cur_player)))
                continue;
            p = p + dir;
            while (is_spot_on_board(p) && get_disc(p) != EMPTY) {
                if (is_disc_at(p, cur_player))
                    return true;
                p = p + dir;
            }
        }
        return false;
    }

    void flip_discs(Point center) {
        for (Point dir: directions) {
            // Move along the direction while testing.
            Point p = center + dir;
            if (!is_disc_at(p, get_next_player(cur_player)))
                continue;
            std::vector<Point> discs({p});
            p = p + dir;
            while (is_spot_on_board(p) && get_disc(p) != EMPTY) {
                if (is_disc_at(p, cur_player)) {
                    for (Point s: discs) {
                        set_disc(s, cur_player);
                    }
                    disc_count[cur_player] += discs.size();
                    disc_count[get_next_player(cur_player)] -= discs.size();
                    break;
                }
                discs.push_back(p);
                p = p + dir;
            }
        }
    }
public:
    OthelloBoard(std::array<std::array<int, SIZE>, SIZE> &b, int player){
        cur_player = player;
        for(int i = 0; i < SIZE; i++){
            for(int j = 0; j < SIZE; j++){
                board[i][j] = b[i][j];
            }
        }
    }

    OthelloBoard(std::array<std::array<int, SIZE>, SIZE> &b, Point p){
        for(int i = 0; i < SIZE; i++){
            for(int j = 0; j < SIZE; j++){
                board[i][j] = b[i][j];
            }
        }
        put_disc(p);
    }

    std::vector<Point> get_valid_spots() const {
        std::vector<Point> valid_spots;
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                Point p = Point(i, j);
                if (board[i][j] != EMPTY)
                    continue;
                if (is_spot_valid(p))
                    valid_spots.push_back(p);
            }
        }
        return valid_spots;
    }

    void put_disc(Point p) {
        set_disc(p, cur_player);
        disc_count[cur_player]++;
        disc_count[EMPTY]--;
        flip_discs(p);
        // Give control to the other player.
        cur_player = get_next_player(cur_player);
        next_valid_spots = get_valid_spots();
        // Check Win
    }

};

int MyValue[8][8] = {   {500, -25, 10,  5, 5, 10, -25, 500},
                        {-25, -45,  1,  1, 1,  1, -45, -25},
                        { 10,   1,  3,  2, 2,  3,   1,  10},
                        {  5,   1,  2,  1, 1,  2,   1,   5},
                        {  5,   1,  2,  1, 1,  2,   1,   5},
                        { 10,   1,  3,  2, 2,  3,   1,  10},
                        {-25, -45,  1,  1, 1,  1, -45, -25},
                        {500, -25, 10,  5, 5, 10, -25, 500}};

enum MAX_MIN {
    maximize = 0,
    minimize = 1
};

// calculate state values
int calaulate_state_value (OthelloBoard cur_OB, Point newPoint) {
    int value;
    int i = newPoint.x;
    int j = newPoint.y;
    value = MyValue[i][j];

    OthelloBoard *cur_board = new OthelloBoard(cur_OB.board, cur_OB.cur_player);
    OthelloBoard *nxt_board = new OthelloBoard(cur_OB.board, newPoint);

    std::vector<Point> cur_next_valid_spots = (*cur_board).get_valid_spots();
    std::vector<Point> nxt_next_valid_spots = (*nxt_board).get_valid_spots();
    int move_difference = cur_next_valid_spots.size() - nxt_next_valid_spots.size();

    value += 5 * move_difference;

    return value;
}

int alpha_beta_pruning(OthelloBoard OB, Point newPoint, int depth, int alpha, int beta, int which_player){
    // basic steps
    if (depth == MAX_DEPTH)
        return calaulate_state_value(OB, newPoint);

    OthelloBoard *obob = new OthelloBoard(OB.board, newPoint);
    std::vector<Point> next_valid_spots = (*obob).get_valid_spots();

    for (auto p : next_valid_spots){
        // find the biggest
        if (which_player == maximize) {
            alpha = max(alpha, alpha_beta_pruning(*obob, p, depth + 1, alpha, beta, minimize));
            if (beta <= alpha) break;

            return alpha;
        }

        // find the least
        else {
            beta = min(beta, alpha_beta_pruning(*obob, p, depth + 1, alpha, beta, maximize));
            if(beta <= alpha) break;

            return beta;
        }
    }

    return 0;
}

void read_board(std::ifstream& fin) {
    fin >> player;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            fin >> board[i][j];
        }
    }
}

void read_valid_spots(std::ifstream& fin) {
    int n_valid_spots;
    fin >> n_valid_spots;
    int x, y;
    for (int i = 0; i < n_valid_spots; i++) {
        fin >> x >> y;
        NEXT_valid_spots.push_back({x, y});
    }
}

void write_valid_spot(std::ofstream& fout) {

    Point corner1(0, 0), corner2(0, 7), corner3(7, 7), corner4(7, 0);
    Point p;
    bool at_corner = false;
    int n_valid_spots = NEXT_valid_spots.size();
    for(int i = 0; i < n_valid_spots; i++){
        if (NEXT_valid_spots[i] == corner1 || NEXT_valid_spots[i] == corner2
                || NEXT_valid_spots[i] == corner3 || NEXT_valid_spots[i] == corner4)
        {
            at_corner = true;
            p = NEXT_valid_spots[i];
            break;
        }
    }
    if(!at_corner) {
        int state_value[n_valid_spots];
        for (int i = 0; i < n_valid_spots; i++) {
            Point pp = NEXT_valid_spots[i];
            OthelloBoard *ob = new OthelloBoard(board, player);
            int sv = alpha_beta_pruning((*ob), pp, 0, -1000, 1000, minimize);
            state_value[i] = sv;
        }

        int idx = 0;
        for (int i = 1; i < n_valid_spots; i++) {
            if (state_value[i] > state_value[idx]) {
                idx = i;
            }
        }

        p = NEXT_valid_spots[idx];
    }

    fout << p.x << " " << p.y << std::endl;
    fout.flush();
}

int main(int, char** argv) {
    std::ifstream fin(argv[1]);
    std::ofstream fout(argv[2]);
    read_board(fin);
    read_valid_spots(fin);
    write_valid_spot(fout);
    fin.close();
    fout.close();
    return 0;
}

