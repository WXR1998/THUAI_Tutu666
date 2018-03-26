#include"communication.h"
#include<vector>
using namespace std;

extern	bool _updateAge;
extern vector<command1> c1;
extern vector<command2>c2;
extern State* state ;
extern vector<State* > all_state;
extern int** map;
extern bool flag;
void player0();
void player1();



int dist(Position p1, Position p2) {
    return (p1.x - p2.x)>0?p1.x-p2.x:p2.x-p1.x + (p1.y - p2.y)?p1.y-p2.y:p2.y-p1.y;
}
bool near_a_road(Position p) {
    int x = p.x;
    int y = p.y;
    if (map[x][y] == 0) {
        if (map[x - 1][y] == 1)
            return 1;
        if (map[x + 1][y] == 1)
            return 1;
        if (map[x][y - 1] == 1)
            return 1;
        if (map[x][y + 1] == 1)
            return 1;
    }
    return 0;
}
Position find_road(Position p) {
    int x = p.x;
    int y = p.y;
    if(map[x][y] == 0){
        if (map[x - 1][y] == 1)
            return Position(x - 1, y);
        if (map[x + 1][y] == 1)
            return Position(x + 1, y);
        if (map[x][y - 1] == 1)
            return Position(x, y - 1);
        if (map[x][y + 1] == 1)
            return Position(x, y + 1);
    }
    return Position(-1, -1);
}
bool near_my_base(Position p) {
    if (flag == 0) {
        if (p.x < 11 && p.y < 11)
            return 1;
        else
            return 0;
    }
    else {
        if (p.x > 188 && p.y > 18)
            return 1;
        else
            return 0;
    }
}
bool near_my_building(Position p) {
    bool yes = 0;
    for (int i = 0; i < state->building[flag].size(); i++) {
        if (dist(p, state->building[flag][i].pos) <= 8)
            yes = 1;
        if (p.x == state->building[flag][i].pos.x && p.y == state->building[flag][i].pos.y)
            yes = 0;
    }
    return yes;
}
bool can_cons(Position p) {
    if (map[p.x][p.y] == 0 && (near_my_base(p) || near_my_building(p)))
        return 1;
    else
        return 0;
}
void f_player()
{
    if (flag == 0)
        player0();
    else
        player1();

    
    
};
void player0() {
    int ins_num = 0;
    if (state->turn < 100) {
        for (int i = 6; i < 20; i++) {
            for (int j = 6; j < 20; j++) {
                if (can_cons(Position(i, j))) {
                    if (near_a_road(Position(i, j)) && state->resource[flag].resource >= 1000)
                        construct(Shannon, Position(i, j), find_road(Position(i,j)));
                    else
                        construct(Programmer, Position(i, j), Position(0, 0));
                    ins_num++;
                    if (ins_num > 50)
                        return;
                }
            }
        }
    }
}
void player1() {
    int ins_num = 0;
    if (state->turn < 100) {
        for (int i = 193; i > 180; i--) {
            for (int j = 193; j > 180; j--) {
                if (can_cons(Position(i, j))) {
                    if (near_a_road(Position(i, j)) && state->resource[flag].resource >= 1000)
                        construct(Shannon, Position(i, j), find_road(Position(i, j)));
                    else
                        construct(Programmer, Position(i, j), Position(0, 0));
                    ins_num++;
                    if (ins_num > 50)
                        return;
                }
            }
        }
    }
}
