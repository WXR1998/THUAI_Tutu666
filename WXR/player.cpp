#include "stdafx.h"
#include "communication.h"
#include <vector>
#include <iostream>
using namespace std;

extern bool _updateAge;
extern vector<command1> c1;
extern vector<command2> c2;
extern State* state;
extern int** map;
extern bool flag;

/*
针对自己所在位置的问题 如果flag==1 需要把所有的信息中的position全部取反
因为map是中心对称的 所以不需要对map进行坐标变换
需要对路径进行编号 从上往下分别为1到road_count
每条路径上分别有一个实体威胁值(0)和数据威胁值(1)
*/
int distance(Position a, Position b) {
	return abs(a.x - b.x) + abs(a.y - b.y);
}
Position inversePosition(Position p) {
	return Position(199 - p.x, 199 - p.y);
}
Position Pos(Position p) {
	/*
		返回变换之后的坐标
	*/
	return !flag ? inversePosition(p) : p;
}
bool positionIsValid(Position p) {
	return p.x >= 0 && p.x < 200 && p.y >= 0 && p.y < 200;
}

int road_number[200][200];
bool road_number_flag = false;
int road_count = 0;
void getRoadNumberDfs(Position p, int number) {
	if (!positionIsValid(p)) return;
	if (map[p.x][p.y] != 1) return;
	road_number[p.x][p.y] = number;
	getRoadNumberDfs(Position(p.x + 1, p.y), number);
	getRoadNumberDfs(Position(p.x, p.y + 1), number);
}
void getRoadNumber() {
	if (road_number_flag) return;
	road_number_flag = true;
	for (int i = 0; i < 7; ++ i)
		if (map[i][7] == 1 && road_number[i][7] == 0)
			getRoadNumberDfs(Position(i, 7), ++ road_count);
	for (int i = 6; i >= 0; -- i)
		if (map[7][i] == 1 && road_number[7][i] == 0)
			getRoadNumberDfs(Position(7, i), ++ road_count);
}

bool can_construct[200][200];
const int dir[4][2] = {0, 1, 1, 0, 0, -1, -1, 0};
void canConstructUpdate() {
	for (int i = 0; i < 11; ++i)
		for (int j = 0; j < 11; ++j)
			if (i < 7 && j < 7);
			else 
				can_construct[i][j] = true;
	for (auto i = state->building[flag].begin(); i != state->building[flag].end(); ++i) 
		for (int j = 0; j < 4; ++j) {
			int tx = (*i).pos.x + dir[j][0], ty = (*i).pos.y + dir[j][1];
			if (positionIsValid(Position(tx, ty)))
				can_construct[tx][ty] = true;
		}
	for (int i = 0; i < 200; ++i)
		for (int j = 0; j < 200; ++j)
			if (map[i][j] == 1)
				can_construct[i][j] = false;
	for (int j = 0; j < 2; ++j)
		for (auto i = state->building[j].begin(); i != state->building[j].end(); ++i)
			can_construct[(*i).pos.x][(*i).pos.y] = false;
}
bool canConstruct(Position p) {
	/*
		返回一个位置是否能够建造建筑
	*/
	return can_construct[p.x][p.y];
}

int crisis_value[2][10];
int attack_value[2][10];
void calcCrisisValue() {
	/*
		计算威胁值 威胁值如下计算：
		在该路上有一个兵
		有一个建筑的出兵点在该路上
	*/
	memset(crisis_value, 0, sizeof crisis_value);
}

void f_player() {
	getRoadNumber();
	canConstructUpdate();
	calcCrisisValue();
}
