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
����Լ�����λ�õ����� ���flag==1 ��Ҫ�����е���Ϣ�е�positionȫ��ȡ�� �ò�����Ϊ����任
����任ֻ��Ҫ��state�ӿڵõ���������ʹ��һ�μ���
��Ϊmap�����ĶԳƵ� ���Բ���Ҫ��map��������任
��Ҫ��·�����б�� �������·ֱ�Ϊ1��road_count

ÿ��·���Ϸֱ���һ��ʵ����вֵ(0)��������вֵ(1)
ÿ��·���Ϸֱ���һ��ʵ�幥��ֵ(0)�����ݹ���ֵ(1)
ÿ��ȡ��вֵ��ߵ�һ·���� ȡ����ֵ��ߵ�һ·����
������·����вֵ������ĳ��ֵʱ ���Կ�ʼ ����ũ ���� �����Ƽ�ʱ�� ���� ��������
��ũ��������Ϊ��ǰʱ���������޳���һ��ϵ��
�Ƽ�ʱ������Ϊ�Է�ʱ������һ��ϵ��

����ά�޽���
����ʱ��֮�� ���Ȱѽ�����������ǰʱ��

������·����pair<int, double>(roadnum, value)���������ȶ���(�󶥶�)
ȡ��вֵ����Ԫ�� ����Ӧ��·�Ͻ����������� ͬʱ��value��ȥһ��ֵ �ٷŻ����ȶ���
ȡ����ֵ����Ԫ�� ����Ӧ��·�Ͻ�����������(��������) ͬʱ��value����һ��ֵ �ٷŻ����ȶ���
*/

//#############################################################################################
//��������
const int BUILDING_RESOURCE[18] =		{};//������Ҫ������Դ
const int BUILDING_BUILDINGCOST[18] = {};//������Ҫ���ٽ����� TODO

const int SOLDIER_ATTACK[8] =					{	10, 16,	160,15,	300,15, 10, 500 };
const int SOLDIER_ATTACKRANGE[8] =				{	16, 24,	6,	10, 6,	40, 12, 20 };
const int SOLDIER_SPEED[8] =					{	16, 12,	20,	6,	24, 16, 4,	12 };
const int SOLDIER_TYPE[8] =						{	1,	0,	0,	0,	1,	0,	1,	0 };
const int SOLDIER_MOVETYPE[8] =					{	0,	0,	1,	2,	1,	0,	2,	0 };
const int SOLDIER_MOVETYPE_CRISIS_FACTOR[3] = { 10, 20, 5 };//���� ��� ����

const int BUILDING_DEFENCE[17] =		{0, 0, 0, 0, 0, 0, 0, 0, 0, 8*6, 20*2, 4*6, 25*3, 8*6, 10*6, 15*6, 100};
const int BUILDING_TYPE[17] =			{3, 3, 3, 3, 3, 3, 3, 3, 3, 1, 0, 0, 0, 2, 1, 2, 2};//�����������ͷ���(1)����ʵ���ͷ���(0)����ȫ��(2)
const int BUILDING_ATTACK_RANGE[17] =	{0, 0, 0, 0, 0, 0, 0, 0, 0, 36, 30, 60, 40, 50, 36, 24, 20};
const int BUILDING_LEVEL_FACTOR[6] =	{2, 3, 4, 5, 6, 7};

const double SOLDIER_ATTACK_FACTOR = 1;	//�Ա��Ĺ���ֵ����һ������ �Ժͽ�������вֵ����ƽ��

const int dir[4][2] = {0, 1, 1, 0, 0, -1, -1, 0};

//#############################################################################################
//��������

int distance(Position a, Position b) {
	return abs(a.x - b.x) + abs(a.y - b.y);
}
Position inversePosition(Position p) {
	return Position(199 - p.x, 199 - p.y);
}
Position Pos(Position p) {
	/*
		���ر任֮�������
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
	/*
		�ú������ڸ�·���  ֻ������һ��
	*/
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
void canConstructUpdate() {
	memset(can_construct, 0, sizeof can_construct);
	for (int i = 0; i < 11; ++i)
		for (int j = 0; j < 11; ++j)
			if (i < 7 && j < 7);
			else 
				can_construct[i][j] = true;
	for (auto i = state->building[flag].begin(); i != state->building[flag].end(); ++i) 
		for (int j = 0; j < 4; ++j) {
			int tx = Pos((*i).pos).x + dir[j][0], ty = Pos((*i).pos).y + dir[j][1];
			if (positionIsValid(Position(tx, ty)))
				can_construct[tx][ty] = true;
		}
	for (int i = 0; i < 200; ++i)
		for (int j = 0; j < 200; ++j)
			if (map[i][j] == 1)
				can_construct[i][j] = false;
	for (int j = 0; j < 2; ++j)
		for (auto i = state->building[j].begin(); i != state->building[j].end(); ++i)
			can_construct[Pos((*i).pos).x][Pos((*i).pos).y] = false;
}
bool canConstruct(Position p) {
	/*
		����һ��λ���Ƿ��ܹ����콨��
	*/
	return can_construct[p.x][p.y];
}

double crisis_value[2][2][10];//0Ϊ�Է������ǵ���вֵ 1Ϊ���ǵĹ���ֵ
double soldierCrisisValue(Soldier s, int t) {
	/*
		���ر�����вֵ tΪ0��1 Ϊ���ݻ�ʵ����вֵ
	*/
	int type = s.soldier_name;
	return s.heal * SOLDIER_MOVETYPE_CRISIS_FACTOR[SOLDIER_MOVETYPE[type]] * SOLDIER_ATTACK[type] * 
		log(SOLDIER_ATTACKRANGE[type]) * SOLDIER_SPEED[type] * (t == SOLDIER_TYPE[type]) * SOLDIER_ATTACK_FACTOR;
}
double buildingCrisisValue(Building b, int t, int roadnum) {
	/*
		���ؽ�����ĳһ·����вֵ tΪ0��1 
	*/
	int type = b.building_type, grid = 0, range = BUILDING_ATTACK_RANGE[b.building_type], typeFactor;
	for (int i = -range; i <= range; ++i)
		for (int j = -range + abs(i); j <= -abs(i) + range; ++j) {
			int x = i + Pos(b.pos).x, y = j + Pos(b.pos).y;
			if (positionIsValid(Position(x, y)))
				if (road_number[x][y] == roadnum)
					++grid;
		}
	typeFactor = (BUILDING_TYPE[type] == 2) ? 1 : (t == BUILDING_TYPE[type]);
	return log(b.heal) * BUILDING_DEFENCE[type] * BUILDING_LEVEL_FACTOR[b.level] * grid * typeFactor;

}
void calcCriAttValue() {
	/*
		��вֵ�ļ��㷽ʽ����·�� �з�������вֵ֮�� - ��·���ҷ����߱�����вֵ֮�� - ��·���ҷ�����������вֵ֮��
		����ֵ�ļ��㷽ʽ����·�� �ҷ�������вֵ֮�� - ��·�ϵз����߱�����вֵ֮�� - ��·�ϵз�����������вֵ֮��
	*/
	for (int PLAYER = 0; PLAYER < 2; ++PLAYER) {
		memset(crisis_value, 0, sizeof crisis_value);
		for (auto i = state->soldier[!PLAYER].begin(); i != state->soldier[!PLAYER].end(); ++i)
			for (int j = 0; j < 2; ++j)
				crisis_value[PLAYER][j][road_number[Pos((*i).pos).x][Pos((*i).pos).y]] += soldierCrisisValue(*i, j);
		for (auto i = state->building[PLAYER].begin(); i != state->building[PLAYER].end(); ++i)
			for (int j = 0; j < 2; ++j)
				for (int k = 1; k <= road_count; ++k)
					crisis_value[PLAYER][j][k] -= buildingCrisisValue(*i, j, k);
		for (auto i = state->soldier[PLAYER].begin(); i != state->soldier[PLAYER].end(); ++i)
			if (SOLDIER_MOVETYPE[(*i).soldier_name] == 2)
				for (int j = 0; j < 2; ++j)
					crisis_value[PLAYER][j][road_number[Pos((*i).pos).x][Pos((*i).pos).y]] -= soldierCrisisValue(*i, j);
	}
}

//#############################################################################################
//������
void f_player() {
	getRoadNumber();
	canConstructUpdate();
	calcCriAttValue();
}