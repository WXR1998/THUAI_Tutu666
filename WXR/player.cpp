#include "stdafx.h"
#include "communication.h"
#include <vector>
#include <iostream>
#include <queue>
#include <ctime>
#include <cstdlib>
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

class GenRandom {
	vector <pair<int, int> > t;
	int sum;
public:
	void clear() {
		t.clear();
		sum = 0;
	}
	GenRandom() {
		clear();
	}
	void addItem(pair<int, int> d) {// <id, prob>
		if (d.second <= 0) return;
		t.push_back(d);
		sum += d.second;
	}
	int rand() {//����prob�Ŀ�����  ����id
		if (sum == 0) return -1;
		int tmp = rand() % sum + 1;
		for (auto i = t.begin(); i != t.end(); ++i) {
			tmp -= (*i).second;
			if (tmp <= 0)
				return (*i).first;
		}
	}
};

struct heapComp {
	double val;
	int typ, rnum;
	heapComp(double _val, int _typ, int _rnum) {
		val = _val;
		typ = _typ;
		rnum = _rnum;
	}
	bool operator < (const struct heapComp b) const{
		return val < b.val;
	}
};

//#############################################################################################
//��������
const int BUILDING_RESOURCE[18] = {0, 150, 160, 160, 200, 250, 300, 600, 600, 150, 200, 225, 200, 250, 450, 500, 500, 100};//������Ҫ������Դ
const int BUILDING_BUILDINGCOST[18] = {0, 15, 16, 16, 20, 25, 30, 60, 60, 15, 20, 22, 20, 25, 45, 50, 50, 10};//������Ҫ���ٽ����� TODO
const int BUILDING_UNLOCK_AGE[18] = {0, 0, 1, 1, 2, 4, 4, 5, 5, 0, 1, 2, 3, 4, 4, 5, 5, 0};

const int SOLDIER_ATTACK[8] = { 10, 16,	160,15,	300,15, 10, 500 };
const int SOLDIER_ATTACKRANGE[8] = { 16, 24,6,	10, 6,	40, 12, 20 };
const int SOLDIER_SPEED[8] = { 16, 12,	20,	6,	24, 16, 4,	12 };
const int _SOLDIER_TYPE[8] = { 1,	0,	0,	0,	1,	0,	1,	0 };
const int SOLDIER_MOVETYPE[8] = { 0,	0,	1,	2,	1,	0,	2,	0 };
const int SOLDIER_MOVETYPE_CRISIS_FACTOR[3] = { 10, 20, 5 };//���� ��� ����

const int BUILDING_DEFENCE[17] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 8 * 6, 20 * 2, 4 * 6, 25 * 3, 8 * 6, 10 * 6, 15 * 6, 100 };
const int _BUILDING_TYPE[17] = { 3, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 2, 1, 2, 2 };//�����������ͷ���(1)����ʵ���ͷ���(0)����ȫ��(2)
const int BUILDING_ATTACK_RANGE[17] = { 0, 10, 5, 5, 15, 20, 15, 15, 10, 36, 30, 60, 40, 50, 36, 24, 20 };//�����Ĺ�����Χ/������Χ
const int BUILDING_LEVEL_FACTOR[6] = { 2, 3, 4, 5, 6, 7 };
const int BUILDING_HEAL[18] = { 10000, 150, 200, 180, 200, 150, 160, 300, 250, 300, 280, 225, 300, 180, 450, 1000, 400, 100 };

const double SOLDIER_ATTACK_FACTOR = 1;	//�Ա��Ĺ���ֵ����һ������ �Ժͽ�������вֵ����ƽ��

const int dir[4][2] = { 0, 1, 1, 0, 0, -1, -1, 0 };
const int MAX_OPERATION_PER_TURN = 50;
const double MAX_CRISIS = 0;	//������·��crisisֵ��С�����ֵ��ʱ�� ���Կ�ʼ�������� �������
const double MIN_ATTACK = 0;	//������·��attackֵ���������ֵ��ʱ�� ���Կ�ʼ��չ �������
const double PROGRAMMER_RATIO = 0.3;
const int MAX_TRIAL = 10;

//#############################################################################################
//��������

int operation_count;	//ÿ�غϵĲ�����
double my_building_credits;	//ÿ�غϵĽ�����
int my_resource;	//���ֵ���Դ
bool _construct(BuildingType a, Position b, Position c=Position(-1, -1)) {
	if (operation_count <= 0) return false;
	if (BUILDING_BUILDINGCOST[a] * (1 + AGE_INCREASE * state->age[flag]) > my_building_credits ||
		BUILDING_RESOURCE[a] * (1 + AGE_INCREASE * state->age[flag]) > my_resource)
		return false;
	--operation_count;
	printf("%d %d %d\n", a, b.x, b.y);
	construct(BuildingType(a), b, c);
	return true;
}
bool _UpdateAge() {
	if (operation_count <= 0) return false;
	if (my_resource < UPDATE_COST + UPDATE_COST_PLUS * state->age[flag])
		return false;
	--operation_count;
	updateAge();
	return true;
}

int distance(Position a, Position b) {
	return abs(a.x - b.x) + abs(a.y - b.y);
}
Position inversePosition(Position p) {
	return Position(MAP_SIZE - 1 - p.x, MAP_SIZE - 1 - p.y);
}
Position Pos(Position p) {
	/*
	���ر任֮�������
	*/
	return !flag ? inversePosition(p) : p;
}
bool positionIsValid(Position p) {
	return p.x >= 0 && p.x < MAP_SIZE && p.y >= 0 && p.y < MAP_SIZE;
}

int road_number[MAP_SIZE][MAP_SIZE];
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
	for (int i = 0; i < 7; ++i)
		if (map[i][7] == 1 && road_number[i][7] == 0)
			getRoadNumberDfs(Position(i, 7), ++road_count);
	for (int i = 6; i >= 0; --i)
		if (map[7][i] == 1 && road_number[7][i] == 0)
			getRoadNumberDfs(Position(7, i), ++road_count);
}

bool can_construct[MAP_SIZE][MAP_SIZE];
void canConstructUpdate() {
	memset(can_construct, 0, sizeof can_construct);
	for (int i = 0; i < 7 + BD_RANGE_FROM_BASE; ++i)
		for (int j = 0; j < 7 + BD_RANGE_FROM_BASE; ++j)
			if (i < 7 && j < 7);
			else
				can_construct[i][j] = true;
	for (auto i = state->building[flag].begin(); i != state->building[flag].end(); ++i)
		for (int j = Pos((*i).pos).x - BD_RANGE; j <= Pos((*i).pos).x + BD_RANGE; ++j)
			for (int k = Pos((*i).pos).y - (BD_RANGE - abs(j - Pos((*i).pos).x)); k <= Pos((*i).pos).y + (BD_RANGE - abs(j - Pos((*i).pos).x)); ++k)
				if (positionIsValid(Position(j, k)))
					can_construct[j][k] = true;
	for (int i = 0; i < MAP_SIZE; ++i)
		for (int j = 0; j < MAP_SIZE; ++j)
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
int posCoverGrid(Position p, int range, int roadnum) {
	/*
	����һ��λ�ú;��룬�����λ�����ܸ��Ƕ��ٸ�roadnum·�ϵĵ�
	*/
	int ans = 0;
	for (int i = -range; i <= range; ++i)
		for (int j = -range + abs(i); j <= -abs(i) + range; ++j) {
			int x = i + p.x, y = j + p.y;
			if (positionIsValid(Position(x, y)))
				if (road_number[x][y] == roadnum)
					++ans;
		}
	return ans;
}
double buildingCrisisValue(Building b, int t, int roadnum) {
	/*
	���ؽ�����ĳһ·����вֵ tΪ0��1
	*/
	int type = b.building_type, grid = 0, range = BUILDING_ATTACK_RANGE[b.building_type], typeFactor;
	typeFactor = (_BUILDING_TYPE[type] == 2) ? 1 : (t == _BUILDING_TYPE[type]);
	return log(b.heal) * BUILDING_DEFENCE[type] * BUILDING_LEVEL_FACTOR[b.level] * posCoverGrid(Pos(b.pos), range, roadnum) * typeFactor;

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


void _maintain() {
	/*
	����ά�޽���ֵ�͵Ľ���
	*/
	priority_queue <pair<double, vector<Building>::iterator> > h;
	for (auto i = state->building[flag].begin(); i != state->building[flag].end(); ++i) {
		double full_hp = BUILDING_HEAL[(*i).building_type] * (1 + AGE_INCREASE*(*i).level);
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
	_UpdateAge();
}
void _build_programmer() {
	for (int i = 7; i < 15; ++i) 
		for (int j = 0; j <= i; ++j) {
			if (canConstruct(Position(j, i))) {
				if (_construct(Programmer, Pos(Position(j, i)))) 
					can_construct[j][i] = false;
				return;
			}
			if (canConstruct(Position(i, j))) {
				if (_construct(Programmer, Pos(Position(i, j)))) 
					can_construct[i][j] = false;
				return;
			}
		}
}

Position nearestRoad(Position p, int roadnum) {
	for (int range = 1; range <= 20; ++ range)
		for (int i = -range; i <= range; ++i) 
			for (int j = -range + abs(i); j <= -abs(i) + range; ++j)
				if (positionIsValid(Position(i, j)))
					if (road_number[i][j] == roadnum)
						return Position(i, j);
	return Position(-1, -1);
}

void _defend() {
	priority_queue < heapComp > h;
	GenRandom gr;
	for (int i = 0; i < 2; ++i)
		for (int j = 1; j <= road_count; ++j)
			h.push(heapComp(crisis_value[0][i][j], i, j));
	int fail_trial = 0;
	while (h.top().val > MAX_CRISIS && operation_count > 0 && fail_trial < MAX_TRIAL) {
		heapComp t = h.top(); h.pop();
		gr.clear();
		for (int i = 9; i < 17; ++i)
			if (_BUILDING_TYPE[i] == 2 || _BUILDING_TYPE[i] == t.typ)
				gr.addItem(make_pair(i, (i - 8)));
		int bdtype = gr.rand();//�ն����콨��������
		gr.clear();
		for (int i = 0; i < MAP_SIZE; ++i)
			for (int j = 0; j < MAP_SIZE; ++j)
				if (canConstruct(Position(i, j)))
					gr.addItem(make_pair(i * MAP_SIZE + j, posCoverGrid(Position(i, j), BUILDING_ATTACK_RANGE[bdtype], t.rnum)));
		int tmppos = gr.rand();
		Position bdpos = Position(tmppos / MAP_SIZE, tmppos % MAP_SIZE);
		if (_construct(BuildingType(bdtype), Pos(bdpos))) {
			can_construct[bdpos.x][bdpos.y] = false;
			Building tmpbd = Building(BuildingType(bdtype), BUILDING_HEAL[bdtype] * (1 + AGE_INCREASE * state->age[flag]), Pos(bdpos), flag, 0, state->age[flag]);
			t.val -= buildingCrisisValue(tmpbd, t.typ, t.rnum);
		}
		h.push(t);
	}
}
void _attack() {
	priority_queue < heapComp > h;
	GenRandom gr;
	for (int i = 0; i < 2; ++i)
		for (int j = 1; j <= road_count; ++j)
			h.push(heapComp(crisis_value[1][i][j], i, j));
	while (h.top().val < MIN_ATTACK && operation_count > 0) {
		heapComp t = h.top(); h.pop();
		gr.clear();
		for (int i = 1; i < 9; ++i)
			if (_BUILDING_TYPE[i] == t.typ)
				gr.addItem(make_pair(i, i));
		int bdtype = gr.rand();//�ն����콨��������
		gr.clear();
		for (int i = 0; i < MAP_SIZE; ++i)
			for (int j = 0; j < MAP_SIZE; ++j)
				if (canConstruct(Position(i, j)))
					gr.addItem(make_pair(i * MAP_SIZE + j, posCoverGrid(Position(i, j), BUILDING_ATTACK_RANGE[bdtype], t.rnum)));
		int tmppos = gr.rand();
		Position bdpos = Position(tmppos / MAP_SIZE, tmppos % MAP_SIZE);
		if (_construct(BuildingType(bdtype), Pos(bdpos), nearestRoad(bdpos, t.rnum))) {
			can_construct[bdpos.x][bdpos.y] = false;
			Building tmpbd = Building(BuildingType(bdtype), BUILDING_HEAL[bdtype] * (1 + AGE_INCREASE * state->age[flag]), Pos(bdpos), flag, 0, state->age[flag]);
			t.val -= buildingCrisisValue(tmpbd, t.typ, t.rnum);
		}
		h.push(t);
	}
}



//#############################################################################################
//������
int srand_flag = 0;
void f_player() {
	if (!srand_flag) {
		srand_flag = 1;
		srand((flag + 1) * time(NULL));
	}
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

	int tot_programmer = 0;
	for (auto i = state->building[flag].begin(); i != state->building[flag].end(); ++i)
		if ((*i).building_type == Programmer)
			++tot_programmer;
	while (tot_programmer < min(PROGRAMMER_RATIO * state->building[flag].size(), 5)) 
		_build_programmer();
	_defend();
	_attack();
	_maintain();
}