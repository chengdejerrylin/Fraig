#include <algorithm>

#include "cirFecPair.h"
#include "cirMgr.h"

using namespace std;

extern CirMgr* cirMgr;


/***************************
//     Public Method
****************************/

FecPair::FecPair(unsigned var, bool first): 
	_hash(new gateSet() ), _min(var), _dfsMin(var), _isVecSet(false), _first(first), _used(true) 
{
	_hash->insert(var);
	cirMgr->getGate(var/2)->setFecPair(this);
}

FecPair::~FecPair() {
	if(_used)
		for(auto &i : *_hash)cirMgr->getGate(i/2)->setFecPair(0);

	delete _hash;
}

void FecPair::insert(unsigned var) {
	_hash->insert(var);
	cirMgr->getGate(var/2)->setFecPair(this);

	if((int)var < _min) _min = var;

	if(var/2 == 0)_dfsMin = var;
	else if (_dfsMin != -1)
		if(cirMgr->getGate(var/2)->getDfsPos() < cirMgr->getGate(_dfsMin/2)->getDfsPos() )_dfsMin = var;

	_isVecSet = false;
}

void FecPair::generate(vector<FecPair* >* group){
	fecMap map;
	size_t value;

	for(auto &i : *_hash){
		value = cirMgr->getGate(i/2)->getValue();
		if( i % 2) value = ~value;

		if(map.find(value) == map.end() )map.insert(fecData(value, new FecPair(i, false) ) );
		else map.find(value)->second->insert(i);
	}

	_first ? generateFirst(group, map) : generate(group, map);

	_used = false;
}

void FecPair::removeOutDfs() {
	_dfsMin = -1;
	_vec.clear();
	_isVecSet = false;

	for(auto &i : *_hash)
		if(i/2 == 0)_dfsMin = i;
		else if (!(cirMgr->getGate(i/2) ) )_vec.push_back(i);
		else if(cirMgr->getGate(i/2)->getFlag() == cirMgr->getFlag() ){
			if(_dfsMin == -1)_dfsMin = i;
			else if (_dfsMin /2 != 0)
				if(cirMgr->getGate(i/2)->getDfsPos() < cirMgr->getGate(_dfsMin/2)->getDfsPos() )_dfsMin = i;
		}else _vec.push_back(i);

	for(auto &i : _vec)erase(i);
}

void FecPair::print() {
	setAll();
	for(auto &i : _vec){
		cout << ' ';
		CirGate::printNetFanin(i);
	}
}

/***************************
//     Private Method
****************************/
void FecPair::setAll() {
	checkInverse();
	makeVec();
}

void FecPair::makeVec() {
	_vec.clear();
	_vec.resize(_hash->size() );
	gateSet::iterator pos = _hash->begin();

	for(size_t i = 0; i < _hash->size(); ++i){
		_vec.at(i) = *pos;
		++pos;
	}

	sort(_vec.begin(), _vec.end() );
	_isVecSet = true;
	setMin();
}

void FecPair::checkInverse() {
	setMin();

	if(_min % 2){
		gateSet *temp = new gateSet();
		temp->reserve(_hash->size() );
		
		_min = CirMgr::getInverseVar(_min);
		if(_dfsMin != -1)_dfsMin = CirMgr::getInverseVar(_dfsMin);
		for (auto &i : *_hash)temp->insert(CirMgr::getInverseVar(i) );

		delete _hash;
		_hash = temp;
	}
}

void FecPair::setMin() {
	if(_hash->empty() )return;
	if(_min != -1)return;
	if(_isVecSet){ _min = _vec.at(0); return; }

	_min = *(_hash->begin() );
	for(auto &i : *_hash)
			if((int) i < _min)_min = i;
}

void FecPair::setDfsMin() {
	if(_hash->empty() )return;
	if( _dfsMin != -1)return;

	_dfsMin = *(_hash->begin() );

	for(auto &i : *_hash){
		if(i/2 == 0){
			_dfsMin = i;
			return;
		}
		if( cirMgr->getGate(i/2)->getDfsPos() < cirMgr->getGate(_dfsMin/2)->getDfsPos() )_dfsMin = i;
	}
}

void FecPair::erase(unsigned var) {
	_hash->erase(var);
	if(cirMgr->getGate(var/2) )cirMgr->getGate(var/2)->setFecPair(0);

	//_isVecSet = false;
	if((int) var == _min) _min = -1;
	if((int) var == _dfsMin) _dfsMin = -1;
}

void FecPair::generateFirst(vector<FecPair*> * group, fecMap &map) {
	fecMap::iterator invPos;
	bool inverse;

	for(auto &i : map){
		if(i.second == 0)continue;

		inverse = i.first >> (sizeof(size_t)*8 -1);
		invPos = map.find(~i.first);

		//Exist positive
		if(inverse && invPos != map.end() )continue;
		
		//Merge inverse
		if(!inverse && invPos != map.end() ){
			for(auto &j : *(invPos->second->_hash) )i.second->insert(CirMgr::getInverseVar(j) );

			invPos->second->_used = false;
			delete invPos->second;
			invPos->second = 0;
		}

		if(i.second->isSolo() )delete i.second;
		else group->push_back(i.second);
	}
}

void FecPair::generate(vector<FecPair*> * group, fecMap &map) {
	for(auto &i : map)
		if(i.second->isSolo() )delete i.second;
		else group->push_back(i.second);
}