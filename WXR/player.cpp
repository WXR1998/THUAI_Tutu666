#include "stdafx.h"
#include "communication.h"
#include <vector>
#include <iostream>
#include <queue>
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

���Ƚ����µĽ���
����ʱ��֮�� ���������굱ǰturn�Ľ����� ������Ȱѽ�����������ǰʱ�� 

������·����pair<int, double>(roadnum, value)���������ȶ���(�󶥶�)
ȡ��вֵ����Ԫ�� ����Ӧ��·�Ͻ����������� ͬʱ��value��ȥһ��ֵ �ٷŻ����ȶ���
ȡ����ֵ����Ԫ�� ����Ӧ��·�Ͻ�����������(��������) ͬʱ��value����һ��ֵ �ٷŻ����ȶ���

��ũ��Ҫһ������
*/

//#############################################################################################
//��������
const int BUILDING_RESOURCE[18] =		{};//������Ҫ������Դ
const int BUILDING_BUILDINGCOST[18] = {};//������Ҫ���ٽ����� TODO

const int SOLDIER_ATTACK[8] =					{	10, 16,	160,15,	300,15, 10, 500 };
const int SOLDIER_ATTACKRANGE[8] =				{	16, 24,	6,	10, 6,	40, 12, 20 };
const int SOLDIER_SPEED[8] =					{	16, 12,	20,	6,	24, 16, 4,	12 };
const int _SOLDIER_TYPE[8] =					{	1,	0,	0,	0,	1,	0,	1,	0 };
const int SOLDIER_MOVETYPE[8] =					{	0,	0,	1,	2,	1,	0,	2,	0 };
const int SOLDIER_MOVETYPE_CRISIS_FACTOR[3] = { 10, 20, 5 };//���� ��� ����

const int BUILDING_DEFENCE[17] =		{0, 0, 0, 0, 0, 0, 0, 0, 0, 8*6, 20*2, 4*6, 25*3, 8*6, 10*6, 15*6, 100};
const int _BUILDING_TYPE[17] =			{3, 3, 3, 3, 3, 3, 3, 3, 3, 1, 0, 0, 0, 2, 1, 2, 2};//�����������ͷ���(1)����ʵ���ͷ���(0)����ȫ��(2)
const int BUILDING_ATTACK_RANGE[17] =	{0, 0, 0, 0, 0, 0, 0, 0, 0, 36, 30, 60, 40, 50, 36, 24, 20};
const int BUILDING_LEVEL_FACTOR[6] =	{2, 3, 4, 5, 6, 7};
const int BUILDING_HEAL[18] =			{10000, 150, 200, 180, 200, 150, 160, 300, 250, 300, 280, 225, 300, 180, 450, 1000, 400, 100};

const double SOLDIER_ATTACK_FACTOR = 1;	//�Ա��Ĺ���ֵ����һ������ �Ժͽ�������вֵ����ƽ��

const int dir[4][2] = {0, 1, 1, 0, 0, -1, -1, 0};
const int MAX_OPERATION_PER_TURN = 50;
const double MAX_CRISIS = 0;	//������·��crisisֵ��С�����ֵ��ʱ�� ���Կ�ʼ�������� �������
const double MIN_ATTACK = 0;	//������·��attackֵ���������ֵ��ʱ�� ���Կ�ʼ��չ �������
const double PROGRAMMER_RATIO = 0.3;

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
		log(SOLDIER_ATTACKRANGE[type]) * SOLDIER_SPEED[type] * (t == _SOLDIER_TYPE[type]) * SOLDIER_ATTACK_FACTOR;
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
	typeFactor = (_BUILDING_TYPE[type] == 2) ? 1 : (t == _BUILDING_TYPE[type]);
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

int operation_count;	//ÿ�غϵĲ�����
double my_building_credits;	//ÿ�غϵĽ�����
int my_resource;	//���ֵ���Դ
void _maintain() {
	/*
		����ά�޽���ֵ�͵Ľ���
	*/
	priority_queue <pair<double, vector<Building>::iterator> > h;
	for (auto i = state->building[flag].begin(); i != state->building[flag].end(); ++i) {
		double full_hp = BUILDING_HEAL[(*i).building_type] * (1 + 0.5*(*i).level);
		h.push(make_pair(-(*i).heal / full_hp, i));
	}
	while (!h.empty() && operation_count > 0) {
		auto t = h.top(); h.pop();
		if ((*(t.second)).level != state->age[flag])
			upgrade((*(t.second)).unit_id);
		else
			toggleMaintain((*(t.second)).unit_id);
		--operation_count;
	}
	if (operation_count > 0)
		updateAge();
}
void _build_programmer() {
	for (int i = 7; i < 20; ++i) {
		for (int j = 0; j <= i; ++j) {
			if (canConstruct(Position(j, i))) {
				construct(Programmer, Pos(Position(j, i)));
				return;
			}
			if (canConstruct(Position(i, j))) {
				construct(Programmer, Pos(Position(j, i)));
				return;
			}
		}
	}
}
void _defend() {
	/*
		����
	*/
}
void _attack() {
	/*
		����
	*/
}

//#############################################################################################
//������
void f_player() {
	operation_count = MAX_OPERATION_PER_TURN;
	getRoadNumber();
	canConstructUpdate();
	calcCriAttValue();
	my_resource = state->resource[flag].resource;
	my_building_credits = state->resource[flag].building_point;
	pair<double, pair<int, int> > max_crisis = make_pair(0, make_pair(-1, -1));
	pair<double, pair<int, int> > max_attack = make_pair(0, make_pair(-1, -1));
	for (int i = 0; i < 2; ++i)
		for (int j = 1; j <= road_count; ++j)
			max_crisis = max(max_crisis, make_pair(crisis_value[0][i][j], make_pair(i, j)));
	for (int i = 0; i < 2; ++i)
		for (int j = 1; j <= road_count; ++j)
			max_attack = max(max_attack, make_pair(crisis_value[1][i][j], make_pair(i, j)));
	if (max_crisis.first > MAX_CRISIS)
		_defend();
	else {
		int tot_programmer = 0;
		for (auto i = state->building[flag].begin(); i != state->building[flag].end(); ++i)
			if ((*i).building_type == Programmer)
				++tot_programmer;
		while (tot_programmer < min(PROGRAMMER_RATIO * state->building[flag].size(), 5))
			_build_programmer();
		if (max_attack.first < MIN_ATTACK)
			_attack();
		else
			_maintain();
	}
}