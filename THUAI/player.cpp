//#include "stdafx.h"
/*
	Author:
		Tutu666
	Version:
		0.0509.02
	Instructions:
		When upload code to the server, please comment the first line.
		To enable debugging print, add '/D "LOCAL"' in complie settings.
*/
#include <vector>
#include <string>
#include <iostream>
#include <queue>
#include <ctime>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <algorithm>
#include "communication.h"

#ifdef LOCAL
#define debug(format, ...) printf(format, __VA_ARGS__)
#else
#define debug(format, ...) ;
#endif

using namespace std;

extern bool _updateAge;
extern vector<command1> c1;
extern vector<command2> c2;
extern State* state;
extern int** ts19_map;
extern bool ts19_flag;
/*
Aim to unify the expression of position while holding either flag, we define an operation called
position transformation. When holding flag 1, we should inverse all the position informations
from 'state'.
Position transformation should be used ONLY ONCE when getting data from state.
We don't need to transform ts19_map informations because it is central-symmetrical.

We number the roads by certain order, to organize defence and attack. On every road there are
values: Realbody Crisis Value(RCV), Data Crisis Value(DCV), Realbody Attack Value(RAV), Data
Attack Value(DAV).

Take the road which has the highest CV to defend function, while take the road which has the
highest AV to attack function.

The quantity of programmers is controlled by a series of factors and current building limit.

New issue: While enemy soldier are distributed too average, we have to recalculate the crisis value
of defensive buildings.
*/


//#############################################################################################
//const values definitions

const char SOLDIER_NAME[10][20] = { "BIT_STREAM", "VOLTAGE_SOURCE", "CURRENT_SOURCE", "ENIAC", "PACKET", "OPTICAL_FIBER", "TURING_MACHINE", "ULTRON" };
const char BUILDING_NAME[18][20] = { "__Base", "Shannon", "Thevenin", "Norton", "Von_Neumann", "Berners_Lee", "Kuen_Kao", "Turing", "Tony_Stark", "Bool", "Ohm",
"Mole", "Monte_Carlo", "Larry_Roberts", "Robert_Kahn", "Musk", "Hawkin", "Programmer" };
const int BUILDING_RESOURCE[18] = { 0, 150, 160, 160, 200, 250, 400, 600, 600, 120, 160, 225, 200, 250, 450, 500, 500, 100 };
const int BUILDING_BUILDINGCOST[18] = { 0, 15, 16, 16, 20, 25, 40, 60, 60, 15, 20, 22, 20, 25, 45, 50, 50, 10 };
const int BUILDING_UNLOCK_AGE[18] = { 0, 0, 1, 1, 2, 4, 4, 5, 5, 0, 1, 2, 3, 4, 4, 5, 5, 0 };
const int BUILDING_BIAS[17] = { 0, 
	2,		//Bit stream
	8,		//Voltage source
	0,		//Current source
	8,		//ENIAC
	8,		//Packet
	40,		//Optical fiber
	10,		//Turing machine
	50,		//Ultron

	5,		//Bool
	8,		//Ohm
	8,		//Mole
	30,		//Monte Carlo
	30,		//Larry Roborts
	20,		//Robort Kahn
	30,		//Musk
	20,		//Hawkin
};//The probability of build the building

const double SOLDIER_CRISIS_FACTOR[8] = {
	8,		//BIT_STREAM
	2,		//VOLTAGE_SOURCE
	4,		//CURRENT_SOURCE
	8,		//ENIAC
	1.4,	//PACKAGE
	3,		//OPTICAL_FIBER
	30,		//TURING_MACHINE
	3e-1	//ULTRON
};
const int BUILDING_DEFENCE[17] = { 0, 0, 0, 0, 0, 0, 0, 0, 0,
	70,		//Bool
	120,	//Ohm
	50,		//Mole
	40,		//Monte Carlo
	50,		//Larry Roborts
	80,		//Robort Kahn
	50,		//Musk
	200		//Hawkin
};

const double SOLDIER_RATIO_FACTOR[8] = {
	1,		//BIT_STREAM
	2e-1,	//VOLTAGE_SOURCE
	5e-1,	//CURRENT_SOURCE
	1,		//ENIAC
	2,		//PACKAGE
	1,		//OPTICAL_FIBER
	1,		//TURING_MACHINE
	1		//ULTRON
};

const int SOLDIER_ATTACK[8] = { 10, 18,	160,12,	300,20, 8, 500 };
const int SOLDIER_ATTACKRANGE[8] = { 12, 20,3,	5, 3,	32, 6, 15 };
const int SOLDIER_SPEED[8] = { 12, 8,	15,	4,	16, 12, 6,	8 };
const int _SOLDIER_TYPE[8] = { 1,	0,	0,	0,	1,	0,	1,	0 };
const int SOLDIER_MOVETYPE[8] = { 0,	0,	1,	2,	1,	0,	2,	0 };
const double SOLDIER_MOVETYPE_CRISIS_FACTOR[3] = { 1e1, 4e0, 2e0 };//push tower / charge / tank
const int SOLDIER_BUSTER_BUILDING[8] = { Bool, Ohm, Ohm, Hawkin, Larry_Roberts, Monte_Carlo, Robert_Kahn, Musk};

const int _BUILDING_TYPE[17] = { 3, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 2, 1, 2, 2 };//realbody(0) data(1) all(2)
const int BUILDING_ATTACK_RANGE[17] = { 0, 10, 12, 8, 15, 10, 12, 15, 8, 32, 30, 36, 50, 40, 35, 15, 20 };
const int BUILDING_LEVEL_FACTOR[6] = { 2, 3, 4, 5, 6, 7 };
const int BUILDING_HEAL[18] = { 10000, 120, 160, 150, 200, 150, 160, 250, 220, 220, 320, 250, 350, 220, 520, 750, 360, 100 };

const double SOLDIER_ATTACK_FACTOR = 3e0;	//Adjust the power of soldier, to balance the power of buildings

const int dir[4][2] = { 0, 1, 1, 0, 0, -1, -1, 0 };
const int MAX_OPERATION_PER_TURN = 50;
const double MAX_CRISIS[2] = { 0, 0 };
const double MIN_ATTACK[6] = { 1e10, 2e10, 4e10, 8e10, 16e10, 32e10 };
const double PROGRAMMER_RATIO[6] = { 1, 0.95, 0.8, 0.6, 0.5, 0.45 };
const double PROGRAMMER_MIN_PARTITION[6] = { 1, 0.85, 0.7, 0.6, 0.5, 0.45 };
const int UPDATE_AGE_BIAS[6] = { 99, 90, 80, 70, 70, 70 };
const int DEFEND_BUILDING_TO_ROAD_DISTANCE = 16;

/*
Probablity per 20 turns.
*/
const int ATTACK_PROB[] = {20, 20, 100, 200, 250, 250, 150, 100, 80, 80, 100};
const int DEFEND_PROB[] = {50, 50, 50, 120, 130, 140, 150, 140, 120, 100, 100};

const int FRENZY_LIMIT = 50000;
int frenzy_flag = 0;
int cqc_defend_flag = 0;
int cqc_attack_flag = 0;  //0 normal   1 building programmer   2 attack
const int CQC_ATTACK_LIMIT = 2000;
const int CQC_PROGRAMMER_LIMIT = 50;
const int CQC_PROGRAMMER_LEAST_LIMIT = 40;
const double FRENZY_FACTOR = 0.3;
const double ROAD_DEFENCE_FACTOR = 4e-1;	//When not on major road, the crisis factor should mult.
const int CQC_MODE = 0;
const int CQC_MODE_OPERATION_BIAS[4] = {
	20,		//Attack
	7,		//Defend
	6,		//Maintain
	1		//Build Programmer
};
const int MAX_PROGRAMMER_POS = 120;

//#############################################################################################
//Aux. class definition
class GenRandom {
	/*
	To generate random number by certain probablity
	_rand()							Generate random number
	addItem(pair<int, int>)			Insert a data element <id, prob>
	*/
	vector <pair<int, int> > t;
	long long sum;
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
	int _rand() {
		if (sum == 0) return -1;
		long long tmp = ((rand() % 10000)*(rand() % 10000)) % sum + 1;
		for (auto i = t.begin(); i != t.end(); ++i) {
			tmp -= (*i).second;
			if (tmp <= 0)
				return (*i).first;
		}
		return -1;
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
	bool operator < (const struct heapComp b) const {
		return val < b.val;
	}
};

int timer_count;
class Timer {
	/*
	Used for timing.
	time()		Output the time interval from last calling of this method
	*/
	int tim, step;
public:
	Timer() {
		tim = clock();
		step = 0;
		++timer_count;
	}
	void time(const char *s = "") {
		debug("Timer #%d:  Step %2d: %4d ms   -- %s\n", timer_count, ++step, clock() - tim, s);
		tim = clock();
	}
	~Timer() {
		--timer_count;
	}
};

//#############################################################################################
//function definitions

int operation_count;	//Operation limit per turn
double my_building_credits;	//Building credits per turn
int my_resource;	//My resource now
int my_build_request;	//Building requests this turn
/*
When initializing: allow = false, deny = true   can_const = allow && deny
if this grid is covered by one other building's build range, allow = true
if this grid is covered by one other building's forbid range, deny = false
*/
bool allow_construct[MAP_SIZE][MAP_SIZE];
bool deny_construct[MAP_SIZE][MAP_SIZE];

int buildingLimit() {
	return MAX_BD_NUM + MAX_BD_NUM_PLUS * state->age[ts19_flag];
}
bool positionIsValid(Position p);
bool canConstruct(Position p) {
	if (positionIsValid(p))
		return allow_construct[p.x][p.y] && deny_construct[p.x][p.y];
	else return false;
}

int distance(Position a, Position b) {
	return abs(a.x - b.x) + abs(a.y - b.y);
}
Position inversePosition(Position p) {
	return Position(MAP_SIZE - 1 - p.x, MAP_SIZE - 1 - p.y);
}
Position Pos(Position p, int typ = 0) {
	/*
	Returns the position after inversion
	*/
	return (typ ? !ts19_flag : ts19_flag) ? inversePosition(p) : p;
}
void updateConstruct(Position p);
bool _construct(BuildingType a, Position b, Position c = Position(0, 0)) {
	if (operation_count <= 0) return false;
	if (BUILDING_BUILDINGCOST[a] > my_building_credits || BUILDING_RESOURCE[a] > my_resource)
		return false;
	if (int(state->building[ts19_flag].size() - 1 + my_build_request) >= buildingLimit())
		return false;
	debug("Build %s %2d [%s] at (%3d,%3d) while pro=(%3d,%3d)\n", (a<9) ? "ATT" : ((a == 17) ? "PGM" : "DEF"), a, BUILDING_NAME[a], b.x, b.y, c.x, c.y);
	--operation_count;
	++my_build_request;
	my_resource -= BUILDING_RESOURCE[a];
	my_building_credits -= BUILDING_BUILDINGCOST[a];
	construct(a, b, c);
	updateConstruct(Pos(b));
	return true;
}
bool _UpdateAge() {
#define sqr(x) (x * x)
	if (operation_count <= 0) return false;
	if (state->age[ts19_flag] == 5) return false;
	if (my_resource < UPDATE_COST + UPDATE_COST_SQUARE * sqr(state->age[ts19_flag])) return false;
	my_resource -= UPDATE_COST + UPDATE_COST_SQUARE * sqr(state->age[ts19_flag]);
	--operation_count;
	updateAge();
	return true;
}
void _update_age() {
	GenRandom gr;
	gr.addItem(make_pair(0, UPDATE_AGE_BIAS[state->age[ts19_flag]]));
	gr.addItem(make_pair(1, 100 - UPDATE_AGE_BIAS[state->age[ts19_flag]]));
	if (gr._rand() == 0)
		_UpdateAge();
}

bool positionIsValid(Position p) {
	return p.x >= 0 && p.x < MAP_SIZE && p.y >= 0 && p.y < MAP_SIZE;
}

int road_number[MAP_SIZE][MAP_SIZE] = { 0 };
vector <Position> road_grid[10];//The grid of every road TODO
bool road_number_flag = false;
int road_count = 0;
void getRoadNumberDfs(Position p, int number) {
	if (!positionIsValid(p)) return;
	if (ts19_map[p.x][p.y] != 1) return;
	road_number[p.x][p.y] = number;
	getRoadNumberDfs(Position(p.x + 1, p.y), number);
	getRoadNumberDfs(Position(p.x, p.y + 1), number);
}
void getRoadNumber() {
	/*
	Run only one time.
	Used of number the roads.
	*/
	if (road_number_flag) return;
	road_number_flag = true;
	for (int i = 0; i < 7; ++i)
		if (ts19_map[i][7] == 1 && road_number[i][7] == 0)
			getRoadNumberDfs(Position(i, 7), ++road_count);
	for (int i = 7; i >= 0; --i)
		if (ts19_map[7][i] == 1 && road_number[7][i] == 0)
			getRoadNumberDfs(Position(7, i), ++road_count);
	for (int i = 0; i < MAP_SIZE; ++i)
		for (int j = 0; j < MAP_SIZE; ++j)
			if (road_number[i][j] != 0)
				road_grid[road_number[i][j]].push_back(Position(i, j));
}

void updateAllow(Position p) {
	for (int i = p.x - MAX_BD_RANGE; i <= p.x + MAX_BD_RANGE; ++i)
		for (int j = p.y - MAX_BD_RANGE; j <= p.y + MAX_BD_RANGE; ++j)
			if (positionIsValid(Position(i, j)) && distance(Position(i, j), p) <= MAX_BD_RANGE)
				allow_construct[i][j] = true;
}
void updateDeny(Position p) {
	for (int i = p.x - MIN_BD_RANGE; i <= p.x + MIN_BD_RANGE; ++i)
		for (int j = p.y - MIN_BD_RANGE; j <= p.y + MIN_BD_RANGE; ++j)
			if (positionIsValid(Position(i, j)) && distance(Position(i, j), p) < MIN_BD_RANGE)
				deny_construct[i][j] = false;
}
void updateConstruct(Position p) {
	debug("update %d %d\n", p.x, p.y);
	updateAllow(p);
	updateDeny(p);
}

void canConstructUpdate() {
	for (int i = 0; i < MAP_SIZE; ++i)
		for (int j = 0; j < MAP_SIZE; ++j) {
			allow_construct[i][j] = false;
			deny_construct[i][j] = true;
		}
	for (int i = 0; i < 7 + BD_RANGE_FROM_BASE; ++i)
		for (int j = 0; j < 7 + BD_RANGE_FROM_BASE; ++j)
			if (i < 7 && j < 7)
				deny_construct[i][j] = false;
			else
				allow_construct[i][j] = true;
	for (auto i = state->building[ts19_flag].begin(); i != state->building[ts19_flag].end(); ++i) {
		if (i->building_type == Base) continue;
		updateAllow(Pos(i->pos));
		updateDeny(Pos(i->pos));
	}
	for (auto i = state->building[!ts19_flag].begin(); i != state->building[!ts19_flag].end(); ++i) {
		if (i->building_type == Base) continue;
		updateDeny(Pos(i->pos));
	}
	for (int i = 0; i < MAP_SIZE; ++i)
		for (int j = 0; j < MAP_SIZE; ++j)
			if (ts19_map[i][j] != 0)
				deny_construct[i][j] = false;
}

int pos_cover_grid[MAP_SIZE][MAP_SIZE][60][8] = { 0 };
bool pos_cover_grid_vis[MAP_SIZE][MAP_SIZE][60][8] = { false };
int posCoverGrid(Position p, int range, int roadnum) {
	/*
	For a certain position and distance, how many positions of roadnum can it cover?
	Due to heavy calculation, we use memorization to accelerate.
	*/
	if (pos_cover_grid_vis[p.x][p.y][range][roadnum])
		return pos_cover_grid[p.x][p.y][range][roadnum];
	int ans = 0;
	for (int i = -range; i <= range; ++i)
		for (int j = -range + abs(i); j <= -abs(i) + range; ++j) {
			int x = i + p.x, y = j + p.y;
			if (positionIsValid(Position(x, y)))
				if (road_number[x][y] == roadnum)
					++ans;
		}
	pos_cover_grid_vis[p.x][p.y][range][roadnum] = true;
	return pos_cover_grid[p.x][p.y][range][roadnum] = ans;
}

double crisis_value[2][2][10];//[0] indicates crisis value. [1] indicates attack value.
double soldierCrisisValue(Soldier s, int t) {
	/*
	Returns the power of a soldier. t=0/1 indicates data or realbody.
	*/
	int dis_enemy = distance(s.pos, Position(199, 199)), dis_mybase = distance(s.pos, Position(0, 0));
	//if (dis_enemy >= 60 && dis_enemy <= 140 || dis_mybase >= 60 && dis_mybase <= 140); else return 0;
	int type = s.soldier_name;
	return s.heal * SOLDIER_MOVETYPE_CRISIS_FACTOR[SOLDIER_MOVETYPE[type]] * SOLDIER_ATTACK[type] *
		SOLDIER_ATTACKRANGE[type] * SOLDIER_SPEED[type] * (t == _SOLDIER_TYPE[type]) * SOLDIER_ATTACK_FACTOR * SOLDIER_CRISIS_FACTOR[type];
}
double buildingCrisisValue(Building b, int t, int roadnum) {
	/*
	Returns the power of a building for one road. t=0/1 indicates data or realbody.
	*/
	int type = b.building_type, grid = 0, range = BUILDING_ATTACK_RANGE[b.building_type], typeFactor;
	typeFactor = (_BUILDING_TYPE[type] == 2) ? 1 : (t == _BUILDING_TYPE[type]);
	pair<int, int> nearest_road = make_pair(0, 0);
	for (int i = 1; i <= road_count; ++i)
		if (posCoverGrid(b.pos, BUILDING_ATTACK_RANGE[b.building_type], i) > nearest_road.first)
			nearest_road = make_pair(posCoverGrid(b.pos, BUILDING_ATTACK_RANGE[b.building_type], i), i);
	//return log(b.heal + 1) * BUILDING_DEFENCE[type] * BUILDING_LEVEL_FACTOR[b.level] * posCoverGrid(Pos(b.pos), range, roadnum) * typeFactor *
	return b.heal * BUILDING_DEFENCE[type] * BUILDING_LEVEL_FACTOR[b.level] * range * typeFactor *
		((roadnum != nearest_road.second) ? ROAD_DEFENCE_FACTOR : 1);
}
void calcCriAttValue() {
	/*
	CV: Enemy soldier CV - My building CV
	AV: My soldier CV - Enemy building CV
	Every enemy defensive building have a major defend road. Not on this road it's CV
	should be decrease.
	*/
	for (int i = 0; i < 2; ++i)
		for (int j = 0; j < 2; ++j)
			for (int k = 1; k <= road_count; ++k)
				crisis_value[i][j][k] = 0.0;

	for (auto i = state->soldier[!ts19_flag].begin(); i != state->soldier[!ts19_flag].end(); ++i)
		for (int j = 0; j < 2; ++j) {
			crisis_value[0][j][road_number[Pos((*i).pos).x][Pos((*i).pos).y]] += soldierCrisisValue(*i, j);
		}
	for (auto i = state->building[ts19_flag].begin(); i != state->building[ts19_flag].end(); ++i)
		for (int j = 0; j < 2; ++j)
			for (int k = 1; k <= road_count; ++k) {
				crisis_value[0][j][k] -= buildingCrisisValue(*i, j, k);
			}

	for (auto i = state->soldier[ts19_flag].begin(); i != state->soldier[ts19_flag].end(); ++i)
		for (int j = 0; j < 2; ++j) {
			crisis_value[1][j][road_number[Pos((*i).pos).x][Pos((*i).pos).y]] += soldierCrisisValue(*i, j);
		}
	for (auto i = state->building[!ts19_flag].begin(); i != state->building[!ts19_flag].end(); ++i)
		for (int j = 0; j < 2; ++j)
			for (int k = 1; k <= road_count; ++k)
				crisis_value[1][j][k] -= buildingCrisisValue(*i, j, k);
}


void _upgradeBuilding() {
	priority_queue < pair<int, vector<Building>::iterator> > h;
	for (auto i = state->building[ts19_flag].begin(); i != state->building[ts19_flag].end(); ++i)
		if (i->level != state->age[ts19_flag])
			h.push(make_pair(-i->level - (i->building_type==Programmer? 2 : 0), i));
	while (h.size() && operation_count > 0) {
		auto t = h.top(); h.pop();
		if (t.second->building_type == __Base) continue;
		if (my_resource > int(BUILDING_RESOURCE[t.second->building_type] * AGE_INCREASE)
			&& my_building_credits > int(BUILDING_BUILDINGCOST[t.second->building_type] * AGE_INCREASE)) {
			upgrade(t.second->unit_id);
			--operation_count;
			my_resource -= int(BUILDING_RESOURCE[t.second->building_type] * AGE_INCREASE);
			my_building_credits -= int(BUILDING_BUILDINGCOST[t.second->building_type] * AGE_INCREASE);
			debug("Upgrade Building [%12s] from %d\n", BUILDING_NAME[t.second->building_type], t.second->level);
		}
		else
			break;
	}
}
void _maintain() {
	priority_queue <pair<double, vector<Building>::iterator> > h;
	for (auto i = state->building[ts19_flag].begin(); i != state->building[ts19_flag].end(); ++i) {
		double full_hp = BUILDING_HEAL[(*i).building_type] * (1 + AGE_INCREASE * (*i).level);
		if ((*i).heal < full_hp * 0.9)
			h.push(make_pair(-(*i).heal / full_hp, i));
	}

	while (!h.empty() && operation_count > 0) {
		auto t = h.top(); h.pop();
		if (t.second->building_type == __Base) continue;
		if (my_resource < int(0.5 * BUILDING_RESOURCE[(*t.second).building_type] * (1 + state->age[ts19_flag] * AGE_INCREASE))) break;
		if (t.second->heal < 0.9 * BUILDING_HEAL[t.second->building_type] * (1 + AGE_INCREASE * t.second->level)) {
			toggleMaintain(t.second->unit_id);
			my_resource -= int(BUILDING_RESOURCE[(*t.second).building_type] * (1 + state->age[ts19_flag] * AGE_INCREASE));
			--operation_count;
		}
	}
}
bool _build_a_programmer(Position p = (-1, -1)) {
	if (p.x != -1)
		return _construct(Programmer, Pos(p));
	for (int sum = 12; sum < MAX_PROGRAMMER_POS; ++ sum)
		for (int i = 0; i <= sum; ++i) {
			int j = sum - i;
			if (canConstruct(Position(i, j))) {
				return _construct(Programmer, Pos(Position(i, j)));
			}
		}
	return false;
}
void _build_programmer(int cqc_mode = 0) {
	if (state->turn == 0) {
		Position p = Position(10, 0);
		for (;;)
			if (_construct(Programmer, Pos(p)))
				p = Position(p.x + MAX_BD_RANGE, p.y);
			else
				break;
	}
	if (cqc_mode == 0) {
		int tot_programmer = 0;
		for (auto i = state->building[ts19_flag].begin(); i != state->building[ts19_flag].end(); ++i)
			if ((*i).building_type == Programmer)
				++tot_programmer;
		while (tot_programmer < max(PROGRAMMER_RATIO[state->age[ts19_flag]] * state->building[ts19_flag].size(),
			PROGRAMMER_MIN_PARTITION[state->age[ts19_flag]] * (MAX_BD_NUM + MAX_BD_NUM_PLUS * state->age[ts19_flag]))) {
			if (_build_a_programmer()) {
				++tot_programmer;
			}
			else
				break;
		}
	}
	else {
		Position head = Position(0, 0);
		for (auto i = state->building[ts19_flag].begin(); i != state->building[ts19_flag].end(); ++i)
			if (distance(Pos(i->pos), Position(0, 0)) > distance(head, Position(0, 0)))
				head = Pos(i->pos);
		debug("%d %d\n", head.x, head.y);
		int tot_programmer = 0;
		for (auto i = state->building[ts19_flag].begin(); i != state->building[ts19_flag].end(); ++i)
			if (i->building_type == Programmer)
				++tot_programmer;
		for (;tot_programmer < CQC_PROGRAMMER_LIMIT;) {
			Position thead = head;
			if (thead.x < 192)
				thead = Position(thead.x + 8, thead.y);
			else if (thead.x < 199)
				thead = Position(199, thead.y + MAX_BD_RANGE - (199 - thead.x));
			else if (thead.y < 120)
				thead = Position(thead.x, thead.y + 8);
			else break;
			for (int range = 0; range <= MAX_BD_RANGE && !canConstruct(thead); ++ range)
				for (int i = thead.x - range; i <= thead.x + range && !canConstruct(thead); ++ i)
					for (int j = thead.y - range; j <= thead.y + range && !canConstruct(thead); ++ j)
						if (positionIsValid(Position(i, j)) && canConstruct(Position(i, j)))
							thead = Position(i, j);
			if (!_build_a_programmer(thead)) break;
			head = thead;
			++tot_programmer;
		}
	}
}

Position nearest_road[MAP_SIZE][MAP_SIZE][10];
bool nearest_road_vis[MAP_SIZE][MAP_SIZE][10] = { false };
Position nearestRoad(Position p, int roadnum, int LIM = 20) {
	if (nearest_road_vis[p.x][p.y][roadnum])
		return nearest_road[p.x][p.y][roadnum];
	nearest_road_vis[p.x][p.y][roadnum] = true;
	for (int range = 1; range <= LIM; ++range)
		for (int i = -range; i <= range; ++i)
			for (int j = -range + abs(i); j <= -abs(i) + range; ++j)
				if (positionIsValid(Position(p.x + i, p.y + j)))
					if (road_number[p.x + i][p.y + j] == roadnum)
						return nearest_road[p.x][p.y][roadnum] = Position(p.x + i, p.y + j);
	return nearest_road[p.x][p.y][roadnum] = Position(-1, -1);
}

void _defend(int defend_base = 0) {
	priority_queue < heapComp > h;
	while (!h.empty()) h.pop();
	GenRandom gr;
	Timer timer;
	int soldier_count[10][10] = {}, soldier_sum[10] = {};
	for (auto i = state->soldier[!ts19_flag].begin(); i != state->soldier[!ts19_flag].end(); ++i) {
		soldier_count[road_number[Pos(i->pos).x][Pos(i->pos).y]][i->soldier_name] ++;
		soldier_sum[road_number[Pos(i->pos).x][Pos(i->pos).y]] ++;
	}
	if (defend_base) {
		int sum = 0;
		for (auto i = state->building[ts19_flag].begin(); i != state->building[ts19_flag].end(); ++i)
			if (i->building_type == Bool)
				sum++;
		if (sum > 12)
			return;
	}
	while (operation_count > 0) {
		int exit_flag = 0;
		for (int typ = 0; typ < 2 && operation_count > 0; ++typ)
			for (int r = 1; r <= road_count && operation_count > 0; ++r)
				if (crisis_value[0][typ][r] > MAX_CRISIS[typ] || defend_base && typ == 1) {
					exit_flag++;
					gr.clear();
					for (int p = 9; p < 17; ++p)
						if ((_BUILDING_TYPE[p] == 2 || _BUILDING_TYPE[p] == typ) && BUILDING_UNLOCK_AGE[p] <= state->age[ts19_flag])
							gr.addItem(make_pair(p, BUILDING_BIAS[p]));
					int bdtype = gr._rand();
					gr.clear();
					gr.addItem(make_pair(0, 50));//0 Means not change the building type
					for (int i = 1; i < 8; ++i)//Bool is too weak
						if (BUILDING_UNLOCK_AGE[SOLDIER_BUSTER_BUILDING[i]] <= state->age[ts19_flag] && soldier_sum[r] != 0)
							gr.addItem(make_pair(SOLDIER_BUSTER_BUILDING[i], double(soldier_count[r][i]) / soldier_sum[r] * SOLDIER_RATIO_FACTOR[i] * 100));
					int bdchange = gr._rand();
					if (bdchange) bdtype = bdchange;
					//To save time, defensive building can only be built in the distance of DEFEND_BUILDING_TO_ROAD_DISTANCE far from road.
					for (int i = 0; i < MAP_SIZE; ++i)
						for (int j = 0; j < MAP_SIZE; ++j)
							if (canConstruct(Position(i, j)) && distance(nearestRoad(Position(i, j), r, DEFEND_BUILDING_TO_ROAD_DISTANCE), Position(i, j)) <= DEFEND_BUILDING_TO_ROAD_DISTANCE)
								if (!defend_base)
									if (i + j <= 180)
										if (bdtype != Hawkin)
											gr.addItem(make_pair(i * MAP_SIZE + j, int((i + j)*(posCoverGrid(Position(i, j), BUILDING_ATTACK_RANGE[bdtype], r)))));//Defensive building prefers far from base
										else
											gr.addItem(make_pair(i * MAP_SIZE + j, int(4000 / (i + j)*(posCoverGrid(Position(i, j), BUILDING_ATTACK_RANGE[bdtype], r)))));//Defensive building prefers far from base
									else;
								else
									if (i < 16 && j < 16)
										gr.addItem(make_pair(i * MAP_SIZE + j, 40000 / (i + j)));//Defend Base
					int tmppos = gr._rand();
					if (tmppos == -1) {
						exit_flag--;
						continue;
					}
					Position bdpos = Position(tmppos / MAP_SIZE, tmppos % MAP_SIZE);
					if (positionIsValid(bdpos) && _construct(BuildingType(bdtype), Pos(bdpos))) {
						Building tmpbd = Building(BuildingType(bdtype), BUILDING_HEAL[bdtype], Pos(bdpos), ts19_flag, 0, 0);
						crisis_value[0][typ][r] -= buildingCrisisValue(tmpbd, typ, r);
					}
					else {
						exit_flag--;
						continue;
					}
				}
		if (!exit_flag) break;
	}
}
void _attack(int fixed_road = 0) {
	priority_queue < heapComp > h;
	GenRandom gr;
	/*
	for (int i = 0; i < 2; ++i)
		for (int j = 1; j <= road_count; ++j)
			h.push(heapComp(crisis_value[1][i][j], i, j));
	*/
	while (operation_count > 0) {
		//heapComp t = h.top(); h.pop();

		int rnum = rand() % road_count + 1;
		if (fixed_road)
			rnum = fixed_road;

		gr.clear();
		for (int i = 1; i < 9; ++i)
			if (BUILDING_UNLOCK_AGE[i] <= state->age[ts19_flag])
				gr.addItem(make_pair(i, BUILDING_BIAS[i]));
		int bdtype = gr._rand();
		if (bdtype == -1) continue;
		if (fixed_road && SOLDIER_MOVETYPE[bdtype - 1] != 0)
			bdtype = Shannon;
		gr.clear();
		for (int i = 0; i < MAP_SIZE; ++i)
			for (int j = 0; j < MAP_SIZE; ++j)
				if (i+j < 150 && canConstruct(Position(i, j)))
					if (nearestRoad(Position(i, j), rnum, BUILDING_ATTACK_RANGE[bdtype]).x != -1)
						if (!fixed_road && i + j <= 200 || (fixed_road && i < 20 && j < 20))
							gr.addItem(make_pair(i * MAP_SIZE + j, (40000 / (i + j))));//Productive building prefers close to base
		int tmppos = gr._rand();
		Position bdpos = Position(tmppos / MAP_SIZE, tmppos % MAP_SIZE);
		Position prpos = Position(-1, -1);
		if (fixed_road) {
			for (int i = 0; i < 60; ++ i)
				for (int j = 0; j < 60; ++ j)
					if (road_number[i][j] == fixed_road && distance(bdpos, Position(i, j)) < BUILDING_ATTACK_RANGE[bdtype]) {
						if (prpos.x == -1 || i + j < prpos.x + prpos.y)
							prpos = Position(i, j);
					}
		}
		else {
			prpos = nearestRoad(bdpos, rnum, BUILDING_ATTACK_RANGE[bdtype]);
		}
		if (_construct(BuildingType(bdtype), Pos(bdpos), Pos(prpos)));
		else
			break;
		//h.push(t);
	}
}

void _build_cqc_building() {
	Position head = Position(0, 0);
	for (auto i = state->building[ts19_flag].begin(); i != state->building[ts19_flag].end(); ++i)
		if (distance(Pos(i->pos), Position(0, 0)) > distance(Position(0, 0), head))
			head = Pos(i->pos);
	while (distance(head, Position(199, 199)) > 40) {
		Position thead = head;
		if (thead.y < 160)
			thead = Position(thead.x, thead.y + MAX_BD_RANGE);
		for (int range = 0; range <= MAX_BD_RANGE && !canConstruct(thead); ++range)
			for (int i = thead.x - range; i <= thead.x + range && !canConstruct(thead); ++i)
				for (int j = thead.y - range; j <= thead.y + range && !canConstruct(thead); ++j)
					if (positionIsValid(Position(i, j)) && canConstruct(Position(i, j)))
						thead = Position(i, j);
		if (!_build_a_programmer(thead)) break;
		head = thead;
	}
	if (state->building[ts19_flag].size() - 1 == buildingLimit()) {
		int tot_programmer = 0;
		for (auto i = state->building[ts19_flag].begin(); i != state->building[ts19_flag].end(); ++i)
			if (i->building_type == Programmer)
				++tot_programmer;
		for (auto i = state->building[ts19_flag].begin(); tot_programmer > CQC_PROGRAMMER_LEAST_LIMIT && i != state->building[ts19_flag].end(); ++i)
			if (i->building_type == Programmer) {
				sell(i->unit_id);
				--tot_programmer;
			}
	}
	/*
	if (state->turn > 47) {
		for (auto i = state->building[ts19_flag].begin(); i != state->building[ts19_flag].end(); ++i)
			if (i->building_type == Programmer) {
				sell(i->unit_id);
			}
	}
	*/
	for (;operation_count > 0;) {
		GenRandom gr;
		gr.addItem(make_pair(1, 1));
		gr.addItem(make_pair(2, 2));
		gr.addItem(make_pair(3, 1));
		int bdtype = gr._rand();
		int buildflag = 0;
		for (int i = MAP_SIZE - 15; i < MAP_SIZE && !buildflag; ++i)
			for (int j = MAP_SIZE - 15; j < MAP_SIZE && !buildflag; ++j)
				if (canConstruct(Position(i, j))) {
					int flag = 0;
					for (int k = 1; k <= road_count; ++k)
						if (nearestRoad(Position(i, j), k, BUILDING_ATTACK_RANGE[bdtype]).x != -1)
							flag = k;
					if (flag) 
						if (_construct(BuildingType(bdtype), Pos(Position(i, j)), Pos(nearestRoad(Position(i, j), flag, BUILDING_ATTACK_RANGE[bdtype])))) {
							buildflag = 1;
							continue;
						}
				}
		if (!buildflag) break;
	}
}

void crisisValuePrint() {
	for (int i = 1; i <= road_count; ++i)
		debug("Crisis Value of Road #%d: %6.1lf\n", i, (crisis_value[0][0][i] + crisis_value[0][1][i]) * 1e-6);
	debug("\n");
}

void _frenzy_mode() {
	if (my_resource >= FRENZY_LIMIT && !frenzy_flag && state->building[ts19_flag].size() - 1 >= buildingLimit() - 10)
		frenzy_flag = 1;
	if (my_resource < FRENZY_LIMIT / 5)
		frenzy_flag = 0;
	if (frenzy_flag == 1) {
		int tot_programmer = 0;
		for (auto i = state->building[ts19_flag].begin(); i != state->building[ts19_flag].end(); ++i)
			if (i->building_type == Programmer)
				++tot_programmer;
		int remain_programmer = int(tot_programmer * FRENZY_FACTOR) + (buildingLimit() - state->building[ts19_flag].size() + 1);
		for (auto i = state->building[ts19_flag].begin(); i != state->building[ts19_flag].end(); ++i)
			if (i->building_type == Programmer && tot_programmer > remain_programmer) {
				sell(i->unit_id);
				--tot_programmer;
				--my_build_request;
				debug("Sell Programmer\n");
			}
		frenzy_flag = 2;
	}
	if (frenzy_flag == 2) {
		_attack();
		GenRandom gr;
		gr.addItem(make_pair(0, 5));
		gr.addItem(make_pair(1, 2));
		if (gr._rand())
			_defend();
		_UpdateAge();
		_upgradeBuilding();
		_maintain();
		_defend();
	}
}

void _CQC_defend_mode() {
	int cqc_road = 0;
	pair<int, int> min_dis = make_pair(400, 0);
	for (auto i = state->building[!ts19_flag].begin(); i != state->building[!ts19_flag].end(); ++i)
		min_dis = min(min_dis, make_pair(distance(Pos(i->pos), Position(0, 0)), Pos(i->pos).x * MAP_SIZE + Pos(i->pos).y));

	if (min_dis < make_pair(150, 0)) {
		pair<int, int> min_road = make_pair(1000, 1);
		for (int i = 1; i <= road_count; ++i) {
			Position t = nearestRoad(Position(min_dis.second / MAP_SIZE, min_dis.second % MAP_SIZE), i, 30);
			if (distance(t, Position(min_dis.second / MAP_SIZE, min_dis.second % MAP_SIZE)) < min_road.first)
				min_road = make_pair(distance(t, Position(min_dis.second / MAP_SIZE, min_dis.second % MAP_SIZE)), i);
		}
		cqc_road = min_road.second;
	}

	if (cqc_road && !cqc_defend_flag)
		cqc_defend_flag = 1;
	if (!cqc_road)
		cqc_defend_flag = 0;
	if (cqc_defend_flag == 1) {
		int tot_programmer = 0;
		for (auto i = state->building[ts19_flag].begin(); i != state->building[ts19_flag].end(); ++i)
			if (i->building_type == Programmer)
				++tot_programmer;
		int remain_programmer = 25;
		for (auto i = state->building[ts19_flag].begin(); i != state->building[ts19_flag].end(); ++i)
			if (i->building_type == Programmer && tot_programmer > remain_programmer) {
				sell(i->unit_id);
				--tot_programmer;
				--my_build_request;
				debug("Sell Programmer\n");
			}
		cqc_defend_flag = 2;
	}
	if (cqc_defend_flag == 2 && cqc_road) {
		debug("CQC DEFEND\n");
		GenRandom gr;
		for (int i = 0; i < 4; ++i)
			gr.addItem(make_pair(i, CQC_MODE_OPERATION_BIAS[i]));
		switch (gr._rand()) {
		case 1:
			_defend(1);
		case 0:
			_attack(cqc_road);
			_UpdateAge();
			break;
		case 2:
			_maintain();
			_upgradeBuilding();
			break;
		case 3:
			_build_programmer();
			break;
		}
	}
}

void _CQC_attack_mode() {
	if (!cqc_attack_flag) {
		if (state->turn > 10) return;
		int min_dis = 400;
		for (auto i = state->building[!ts19_flag].begin(); i != state->building[!ts19_flag].end(); ++i)
			min_dis = min(min_dis, distance(Pos(i->pos), Position(0, 0)));
		debug("mindis = %d\n", min_dis);
		if (min_dis < 360 && state->turn < 2)
			cqc_attack_flag = 1;
		if (CQC_MODE == 1)
			cqc_attack_flag = 1;
	}
	if (cqc_attack_flag) {
		_build_programmer(1);
		_UpdateAge();
		if (cqc_attack_flag == 1 && my_resource > CQC_ATTACK_LIMIT)
			cqc_attack_flag = 2;
		if (cqc_attack_flag == 2)
			_build_cqc_building();
		_upgradeBuilding();
		_maintain();
	}
}

void printMap(int p = 30) {
	for (int i = 0; i < p; ++i) {
		for (int j = 0; j < p; ++j)
			debug("%c ", canConstruct(Position(i, j)) ? '#' : '.');
		debug("\n");
	}
	debug("\n");
}
//#############################################################################################
//Main
int srand_flag = 0;
void f_player() {
	Timer timer;
	my_resource = state->resource[ts19_flag].resource;
	my_building_credits = state->resource[ts19_flag].building_point;
	my_build_request = 0;
	debug("\nTurn=%4d\tAge=%2d\tBuildings=%4zd\tResources=%6d\n", state->turn, state->age[ts19_flag], state->building[ts19_flag].size() - 1, state->resource[ts19_flag].resource);
	if (!srand_flag) {
		srand_flag = 1;
		srand(unsigned((ts19_flag + 1) * time(NULL)));
	}
	operation_count = MAX_OPERATION_PER_TURN;
	getRoadNumber();
	canConstructUpdate();
	calcCriAttValue();


	crisisValuePrint();

	int vis[20] = { 0 };
	vector<pair<int, double> > outp;
	for (auto i = state->soldier[ts19_flag].begin(); i != state->soldier[ts19_flag].end(); ++i)
		if (!vis[i->soldier_name] && soldierCrisisValue(*i, 0) + soldierCrisisValue(*i, 1) > 1) {
			outp.push_back(make_pair(i->soldier_name, soldierCrisisValue(*i, 0) + soldierCrisisValue(*i, 1)));
			vis[i->soldier_name] = 1;
		}
	sort(outp.begin(), outp.end());
	//for (auto i = outp.begin(); i != outp.end(); ++i)
	//	debug("Sol %2d, [%16s], %6.1lf\n", i->first, SOLDIER_NAME[i->first], i->second / 1e6);
	outp.clear();
	memset(vis, 0, sizeof vis);
	for (auto i = state->building[ts19_flag].begin(); i != state->building[ts19_flag].end(); ++i)
		if (!vis[i->building_type]) {
			double ans = 0;
			for (int j = 1; j <= road_count; ++j)
				ans = max(buildingCrisisValue(*i, 0, j) + buildingCrisisValue(*i, 1, j), ans);
			if (ans > 1)
				outp.push_back(make_pair(i->building_type, ans));
			vis[i->building_type] = 1;
		}
	sort(outp.begin(), outp.end());
	//for (auto i = outp.begin(); i != outp.end(); ++i)
	//	debug("Bud %2d, [%16s], %6.1lf\n", i->first, BUILDING_NAME[i->first], i->second / 1e6);

	//_CQC_defend_mode();
	_CQC_attack_mode();
	_frenzy_mode();
	//printMap();

	if (frenzy_flag == 0 && cqc_attack_flag == 0) {
		GenRandom gr;
		_update_age();
		_build_programmer();
		_upgradeBuilding();

		int turn_index = min(state->turn / 20, 10);

		gr.addItem(make_pair(0, 100));
		gr.addItem(make_pair(1, ATTACK_PROB[turn_index]));
		if (gr._rand() == 1) _attack();
		gr.clear();

		gr.addItem(make_pair(0, 100));
		gr.addItem(make_pair(1, DEFEND_PROB[turn_index]));
		if (gr._rand() == 1) _defend();
		gr.clear();

		_maintain();
		if (state->turn > 90) _attack();
	}
}