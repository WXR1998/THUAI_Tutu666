#include "stdafx.h"
/*
	Author:
		Tutu666
	Version:
		0.0415.14
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

const char BUILDING_NAME[18][20] = { "__Base", "Shannon", "Thevenin", "Norton", "Von_Neumann", "Berners_Lee", "Kuen_Kao", "Turing", "Tony_Stark", "Bool", "Ohm",
"Mole", "Monte_Carlo", "Larry_Roberts", "Robert_Kahn", "Musk", "Hawkin", "Programmer" };
const int BUILDING_RESOURCE[18] = { 0, 150, 160, 160, 200, 250, 400, 600, 600, 150, 200, 225, 200, 250, 450, 500, 500, 100 };
const int BUILDING_BUILDINGCOST[18] = { 0, 15, 16, 16, 20, 25, 40, 60, 60, 15, 20, 22, 20, 25, 45, 50, 50, 10 };
const int BUILDING_UNLOCK_AGE[18] = { 0, 0, 1, 1, 2, 4, 4, 5, 5, 0, 1, 2, 3, 4, 4, 5, 5, 0 };
const int BUILDING_BIAS[18] = {0, 1, 0, 8, 8, 25, 30, 20, 30, 
5,		//Bool
8,		//Ohm
8,		//Mole
20,		//Monte Carlo
30,		//Larry Roborts
20,		//Robort Kahn
10,		//Musk
20,		//Hawkin
1 };//The probability of build the building

const int SOLDIER_ATTACK[8] = { 10, 18,	160,12,	300,25, 8, 500 };
const double SOLDIER_CRISIS_FACTOR[8] = {4, 1, 1, 1, 1, 1, 1, 2e-1};
const int SOLDIER_ATTACKRANGE[8] = { 16, 24,3,	10, 3,	40, 12, 20 };
const int SOLDIER_SPEED[8] = { 12, 8,	15,	4,	16, 12, 3,	8 };
const int _SOLDIER_TYPE[8] = { 1,	0,	0,	0,	1,	0,	1,	0 };
const int SOLDIER_MOVETYPE[8] = { 0,	0,	1,	2,	1,	0,	2,	0 };
const double SOLDIER_MOVETYPE_CRISIS_FACTOR[3] = { 1e1, 4e0, 2e0 };//push tower / charge / tank

const int BUILDING_DEFENCE[17] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 80, 120, 80, 40, 50, 120, 100, 400};
const int _BUILDING_TYPE[17] = { 3, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 2, 1, 2, 2 };//realbody(0) data(1) all(2)
const int BUILDING_ATTACK_RANGE[17] = { 0, 10, 5, 5, 15, 20, 15, 15, 10, 32, 30, 36, 50, 40, 35, 24, 20 };
const int BUILDING_LEVEL_FACTOR[6] = { 2, 3, 4, 5, 6, 7 };
const int BUILDING_HEAL[18] = { 10000, 150, 200, 180, 200, 150, 160, 250, 220, 200, 320, 250, 350, 220, 520, 1000, 360, 100 };

const double SOLDIER_ATTACK_FACTOR = 5e0;	//Adjust the power of soldier, to balance the power of buildings

const int dir[4][2] = { 0, 1, 1, 0, 0, -1, -1, 0 };
const int MAX_OPERATION_PER_TURN = 50;
const double MAX_CRISIS[2] = { 0, -5e6};
const double MIN_ATTACK[6] = { 1e5, 2e6, 4e7, 8e8, 16e9, 32e10};
const double PROGRAMMER_RATIO[6] = { 0.82, 0.82, 0.6, 0.5, 0.4, 0.4};
const double PROGRAMMER_MIN_PARTITION[6] = { 0.75, 0.75, 0.6, 0.5, 0.4, 0.4};
const int UPDATE_AGE_BIAS[6] = {50, 40, 40, 30, 30, 20};
const int DEFEND_BUILDING_TO_ROAD_DISTANCE = 3; 

const int FRENZY_LIMIT = 50000;
int frenzy_flag = 0;
const double FRENZY_FACTOR = 0.3;
const double ROAD_DEFENCE_FACTOR = 4e-1;	//When not on major road, the crisis factor should mult.

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
		long long tmp = ((rand()%10000)*(rand()%10000)) % sum + 1;
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
bool can_construct[MAP_SIZE][MAP_SIZE];

int buildingLimit() {
	return MAX_BD_NUM + MAX_BD_NUM_PLUS * state->age[ts19_flag];
}
bool canConstruct(Position p) {
	return can_construct[p.x][p.y];
}

int distance(Position a, Position b) {
	return abs(a.x - b.x) + abs(a.y - b.y);
}
Position inversePosition(Position p) {
	return Position(MAP_SIZE - 1 - p.x, MAP_SIZE - 1 - p.y);
}
Position Pos(Position p) {
	/*
		Returns the position after inversion
	*/
	return ts19_flag ? inversePosition(p) : p;
}
bool _construct(BuildingType a, Position b, Position c = Position(0, 0)) {
	if (operation_count <= 0) return false;
	if (BUILDING_BUILDINGCOST[a] > my_building_credits || BUILDING_RESOURCE[a] > my_resource)
		return false;
	if (int(state->building[ts19_flag].size() - 1 + my_build_request) >= buildingLimit()) 
		return false;
	debug("Build %s %2d [%s] at (%3d,%3d) while pro=(%3d,%3d)\n", (a<9)?"ATT":((a==17)?"PGM":"DEF"), a, BUILDING_NAME[a], b.x, b.y, c.x, c.y);
	--operation_count;
	++my_build_request;
	construct(a, b, c);
	return true;
}
bool _UpdateAge() {
	if (operation_count <= 0) return false;
	if (state->age[ts19_flag] == 5) return false;
	if (my_resource < UPDATE_COST + UPDATE_COST_PLUS * state->age[ts19_flag]) return false;
	my_resource -= UPDATE_COST + UPDATE_COST_PLUS * state->age[ts19_flag];
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

int road_number[MAP_SIZE][MAP_SIZE] = {0};
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

void forbidConstruct(Position p) {
	for (int j = 0; j < 4; ++j) {
		int px = p.x + dir[j][0], py = p.y + dir[j][1];
		if (positionIsValid(Position(px, py)))
			can_construct[px][py] = false;
	}
	can_construct[p.x][p.y] = false;
}
void canConstructUpdate() {
	for (int i = 0; i < MAP_SIZE; ++i)
		for (int j = 0; j < MAP_SIZE; ++j)
			can_construct[i][j] = false;
	for (int i = 0; i < 7 + BD_RANGE_FROM_BASE; ++i)
		for (int j = 0; j < 7 + BD_RANGE_FROM_BASE; ++j)
			if (i < 7 && j < 7);
			else
				can_construct[i][j] = true;
	for (auto i = state->building[ts19_flag].begin(); i != state->building[ts19_flag].end(); ++i)
		for (int j = Pos((*i).pos).x - BD_RANGE; j <= Pos((*i).pos).x + BD_RANGE; ++j)
			for (int k = Pos((*i).pos).y - (BD_RANGE - abs(j - Pos((*i).pos).x)); k <= Pos((*i).pos).y + (BD_RANGE - abs(j - Pos((*i).pos).x)); ++k)
				if (positionIsValid(Position(j, k)))
					can_construct[j][k] = true;
	for (int i = 0; i < MAP_SIZE; ++i)
		for (int j = 0; j < MAP_SIZE; ++j)
			if (ts19_map[i][j] != 0)
				can_construct[i][j] = false;
	for (auto i = state->building[!ts19_flag].begin(); i != state->building[!ts19_flag].end(); ++i)
		can_construct[Pos((*i).pos).x][Pos((*i).pos).y] = false;
	for (auto i = state->building[ts19_flag].begin(); i != state->building[ts19_flag].end(); ++i)
		forbidConstruct(Pos(i->pos));
}

int pos_cover_grid[MAP_SIZE][MAP_SIZE][60][8] = {0};
bool pos_cover_grid_vis[MAP_SIZE][MAP_SIZE][60][8] = {false};
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
	if (dis_enemy >= 60 && dis_enemy <= 140 || dis_mybase >= 60 && dis_mybase <= 140); else return 0;
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
			h.push(make_pair(-i->level, i));
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
bool _build_a_programmer() {
	for (int i = 0; i < 20; ++i)
		for (int j = 0; j < 20; ++j)
			if (canConstruct(Position(i, j)))
				if (_construct(Programmer, Pos(Position(i, j)))) {
					forbidConstruct(Position(i, j));
					my_resource -= BUILDING_RESOURCE[Programmer];
					my_building_credits -= BUILDING_BUILDINGCOST[Programmer];
					return true;
				}else
					return false;
	return false;
}
void _build_programmer() {
	int tot_programmer = 0;
	for (auto i = state->building[ts19_flag].begin(); i != state->building[ts19_flag].end(); ++i)
		if ((*i).building_type == Programmer)
			++tot_programmer;
	while (tot_programmer < max(PROGRAMMER_RATIO[state->age[ts19_flag]] * state->building[ts19_flag].size(),
		PROGRAMMER_MIN_PARTITION[state->age[ts19_flag]] * (MAX_BD_NUM + MAX_BD_NUM_PLUS * state->age[ts19_flag]))) {
		if (_build_a_programmer())
			++tot_programmer;
		else
			break;
	}
}

Position nearest_road[MAP_SIZE][MAP_SIZE][10];
bool nearest_road_vis[MAP_SIZE][MAP_SIZE][10] = {false};
Position nearestRoad(Position p, int roadnum, int LIM = 20) {
	if (nearest_road_vis[p.x][p.y][roadnum])
		return nearest_road[p.x][p.y][roadnum];
	nearest_road_vis[p.x][p.y][roadnum] = true;
	for (int range = 1; range <= LIM; ++range)
		for (int i = -range; i <= range; ++i)
			for (int j = -range + abs(i); j <= -abs(i) + range; ++j)
				if (positionIsValid(Position(p.x + i, p.y + j)))
					if (road_number[p.x+i][p.y+j] == roadnum)
						return nearest_road[p.x][p.y][roadnum] = Position(p.x+i, p.y+j);
	return nearest_road[p.x][p.y][roadnum] = Position(-1, -1);
}

void _defend() {
	priority_queue < heapComp > h;
	while (!h.empty()) h.pop();
	GenRandom gr;
	Timer timer;
	/*
	for (int i = 0; i < 2; ++i)
		for (int j = 1; j <= road_count; ++j) {
			h.push(heapComp(crisis_value[0][i][j], i, j));
			//debug("%lf %d %d\n", crisis_value[0][i][j], i, j);
		}
		*/
	for (; operation_count > 0; ) {
		int exit_flag = 0;
		for (int typ = 0; typ < 2 && operation_count > 0; ++typ)
			for (int r = 1; r <= road_count && operation_count > 0; ++r)
				if (crisis_value[0][typ][r] > MAX_CRISIS[typ]) {
					exit_flag++;
					gr.clear();
					for (int p = 9; p < 17; ++p)
						if ((_BUILDING_TYPE[p] == 2 || _BUILDING_TYPE[p] == typ) && BUILDING_UNLOCK_AGE[p] <= state->age[ts19_flag])
							gr.addItem(make_pair(p, BUILDING_BIAS[p]));
					int bdtype = gr._rand();
					gr.clear();
					//To save time, defensive building can only be built in the distance of DEFEND_BUILDING_TO_ROAD_DISTANCE far from road.
					for (int i = 0; i < MAP_SIZE; ++i)
						for (int j = 0; j < MAP_SIZE; ++j)
							if (canConstruct(Position(i, j)) && distance(nearestRoad(Position(i, j), r, DEFEND_BUILDING_TO_ROAD_DISTANCE), Position(i, j)) <= DEFEND_BUILDING_TO_ROAD_DISTANCE)
								gr.addItem(make_pair(i * MAP_SIZE + j, int((i + j)*(posCoverGrid(Position(i, j), BUILDING_ATTACK_RANGE[bdtype], r)) )));//Defensive building prefers far from base
					int tmppos = gr._rand();
					Position bdpos = Position(tmppos / MAP_SIZE, tmppos % MAP_SIZE);
					if (positionIsValid(bdpos) && _construct(BuildingType(bdtype), Pos(bdpos))) {
						forbidConstruct(bdpos);
						Building tmpbd = Building(BuildingType(bdtype), BUILDING_HEAL[bdtype], Pos(bdpos), ts19_flag, 0, 0);
						crisis_value[0][typ][r] -= buildingCrisisValue(tmpbd, typ, r);
						my_resource -= BUILDING_RESOURCE[bdtype];
						my_building_credits -= BUILDING_BUILDINGCOST[bdtype];
					}
					else {
						exit_flag--;
						continue;
					}
				}
		if (!exit_flag) break;
	}
	/*
	while (h.top().val > MAX_CRISIS && operation_count > 0) {
		heapComp t = h.top(); h.pop();
		gr.clear();
		for (int i = 9; i < 17; ++i)
			if ((_BUILDING_TYPE[i] == 2 || _BUILDING_TYPE[i] == t.typ) && BUILDING_UNLOCK_AGE[i] <= state->age[ts19_flag])
				gr.addItem(make_pair(i, BUILDING_BIAS[i]));
		int bdtype = gr._rand();
		gr.clear();
		//timer.time("Heap Processing");
			//To save time, defensive building can only be built in the distance of DEFEND_BUILDING_TO_ROAD_DISTANCE far from road.
		for (int i = 0; i < MAP_SIZE; ++i)
			for (int j = 0; j < MAP_SIZE; ++j)
				if (canConstruct(Position(i, j)) && distance(nearestRoad(Position(i, j), t.rnum, DEFEND_BUILDING_TO_ROAD_DISTANCE), Position(i, j)) <= DEFEND_BUILDING_TO_ROAD_DISTANCE)
					gr.addItem(make_pair(i * MAP_SIZE + j, int((i+j)*(log(posCoverGrid(Position(i, j), BUILDING_ATTACK_RANGE[bdtype], t.rnum))+1))));//Defensive building prefers far from base
		//timer.time("Heap Processing2");
		int tmppos = gr._rand();
		Position bdpos = Position(tmppos / MAP_SIZE, tmppos % MAP_SIZE);
		if (positionIsValid(bdpos) && _construct(BuildingType(bdtype), Pos(bdpos))) {
			forbidConstruct(bdpos);
			Building tmpbd = Building(BuildingType(bdtype), BUILDING_HEAL[bdtype], Pos(bdpos), ts19_flag, 0, 0);
			t.val -= buildingCrisisValue(tmpbd, t.typ, t.rnum);
			my_resource -= BUILDING_RESOURCE[bdtype];
			my_building_credits -= BUILDING_BUILDINGCOST[bdtype];
		}
		else {
			break;
		}
		h.push(t);
	}
*/
	//timer.time("DefendFinish");
}
void _attack() {
	priority_queue < heapComp > h;
	GenRandom gr;
	for (int i = 0; i < 2; ++i)
		for (int j = 1; j <= road_count; ++j)
			h.push(heapComp(crisis_value[1][i][j], i, j));
	while (h.top().val < MIN_ATTACK[state->age[ts19_flag]] && operation_count > 0) {
		heapComp t = h.top(); h.pop();

		//t.rnum = rand() % road_count + 1;

		gr.clear();
		for (int i = 1; i < 9; ++i)
			if (BUILDING_UNLOCK_AGE[i] <= state->age[ts19_flag]) 
				gr.addItem(make_pair(i, BUILDING_BIAS[i]));
		int bdtype = gr._rand();
		if (bdtype == -1) continue;
		gr.clear();
		for (int i = 0; i < MAP_SIZE; ++i)
			for (int j = 0; j < MAP_SIZE; ++j)
				if (canConstruct(Position(i, j)))
					if (nearestRoad(Position(i, j), t.rnum, BUILDING_ATTACK_RANGE[bdtype]).x != -1)
						gr.addItem(make_pair(i * MAP_SIZE + j, (40000 / (i + j))));//Productive building prefers close to base
		int tmppos = gr._rand();
		Position bdpos = Position(tmppos / MAP_SIZE, tmppos % MAP_SIZE);
		if (_construct(BuildingType(bdtype), Pos(bdpos), Pos(nearestRoad(bdpos, t.rnum, BUILDING_ATTACK_RANGE[bdtype])))) {
			forbidConstruct(bdpos);
			my_resource -= BUILDING_RESOURCE[bdtype];
			my_building_credits -= BUILDING_BUILDINGCOST[bdtype];
		}
		else
			break;
		h.push(t);
	}
}

void crisisValuePrint() {
	for (int i = 1; i <= road_count; ++i)
		debug("Crisis Value of Road #%d: %e\n", i, crisis_value[0][0][i] + crisis_value[0][1][i]);
	//for (int i = 1; i <= road_count; ++i)
	//	debug("ATT Road #%d: %.0lf\n", i, crisis_value[1][0][i] + crisis_value[1][1][i]);
	debug("\n");
}

void _frenzy_mode() {
	if (my_resource >= FRENZY_LIMIT && frenzy_flag == 0 && state->building[ts19_flag].size() - 1 >= buildingLimit())
		frenzy_flag = 1;
	if (my_resource < FRENZY_LIMIT / 5)
		frenzy_flag = 0;
	if (frenzy_flag == 1) {
		int tot_programmer = 0;
		for (auto i = state->building[ts19_flag].begin(); i != state->building[ts19_flag].end(); ++i)
			if (i->building_type == Programmer)
				++tot_programmer;
		int remain_programmer = int(tot_programmer * FRENZY_FACTOR);
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
		_upgradeBuilding();
		_maintain();
	}
}

//#############################################################################################
//Main
int srand_flag = 0;
void f_player() {
	Timer timer;
	my_resource = state->resource[ts19_flag].resource;
	my_building_credits = state->resource[ts19_flag].building_point;
	my_build_request = 0;
	debug("\nTurn=%4d\tAge=%2d\tBuildings=%4zd\tResources=%6d\n", state->turn ,state->age[ts19_flag], state->building[ts19_flag].size() - 1, state->resource[ts19_flag].resource);
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
	for (auto i = state->soldier[ts19_flag].begin(); i != state->soldier[ts19_flag].end(); ++i) 
		if (!vis[i->soldier_name] &&  soldierCrisisValue(*i, 0) + soldierCrisisValue(*i, 1) > 1) {
			debug("Sol %2d, %e\n", i->soldier_name, soldierCrisisValue(*i, 0) + soldierCrisisValue(*i, 1));
			vis[i->soldier_name] = 1;
		}
	memset(vis, 0, sizeof vis);
	for (auto i = state->building[ts19_flag].begin(); i != state->building[ts19_flag].end(); ++i) 
		if (!vis[i->building_type]) {
			double ans = 0;
			for (int j = 1; j <= road_count; ++j)
				ans += buildingCrisisValue(*i, 0, j) + buildingCrisisValue(*i, 1, j);
			if (ans > 1)
				debug("Bud %2d, %e\n", i->building_type, ans);
			vis[i->building_type] = 1;
		}


	//if (fatal_error) exit(1);
	_frenzy_mode();
	if (frenzy_flag == 0) {
		_update_age();
		_build_programmer();
		_defend();
		_upgradeBuilding();
		_maintain();
		_attack();
	}
}