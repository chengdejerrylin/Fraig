/****************************************************************************
  FileName     [ cirFraig.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir FRAIG functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2012-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include <iostream>
#include <iomanip>
#include "cirMgr.h"
#include "cirGate.h"
#include "sat.h"
#include "util.h"

#include "cirDef.h"

//#define FRAIG_MY_VER

using namespace std;


// TODO: Please keep "CirMgr::strash()" and "CirMgr::fraig()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/*******************************************/
/*   Public member functions about fraig   */
/*******************************************/
// _floatList may be changed.
// _unusedList and _undefList won't be changed
void CirMgr::strash(){
	strashMap map;
	map.reserve(_dfs.size() );
	strashMap::iterator pos;

	bool reDfs = false;
	size_t key;
	CirGate *gate;

	for(auto &i : _dfs){
		if(!getGate(i)->isAig() )continue;
		else{
			gate = getGate(i);
			key = ((AigGate*)gate)->getStrashKey(_M);

			pos = map.find(key);
			if(pos == map.end())map.insert(strashData(key, i) );
			else {
				cout << "Strashing: " << pos->second << " merging " << i << "...\n";
				removeGate(i, gate->replace((pos->second)*2) );
				reDfs = true;
			}
		}
	}

	if(reDfs)setDfs();

}

void CirMgr::fraig() {
	if(!_fec)return;

	if(_isFraig){
		for(auto &i : *_fec)delete i;
		delete _fec;
		_fec = 0;

		return;
	}

	/*****************************************************************
	                 Version4  (generate from version3)
	******************************************************************/
	#ifdef FRAIG_MY_VER

	vector<size_t>flagPI;
	FecPair* group;
	unsigned  var;
	bool reset;

	while(!_fec->empty() ){
		flagPI.clear();
		reset = false;

		flagPI.push_back(0);
		for(size_t i = 0; i < _dfs.size(); ++i)
			if(getGate(_dfs.at(i) )->isPI() && i != 0)flagPI.push_back(i);
		flagPI.push_back(_dfs.size() );

		for(size_t i = 1; i < flagPI.size(); ++i){
			for(size_t j = flagPI.at(i)-1; j > flagPI.at(i-1); --j){
				
				group = getGate(_dfs.at(j) )->getFecPair();
				if(!group)continue;

				var = group->getMinDfs();
				if(_dfs.at(j) == var/2)continue;

				//const0
				if(var/2 == 0){
					if(fraigConst(group->isInverse(0) == group->isInverse(_dfs.at(j)) ? 2*_dfs.at(j) : 2*_dfs.at(j) +1) )reset = true;
					else continue;
				}
				if(reset)break;

				//two Aig
				while(group){
					var = group->getMinDfs();
					if(_dfs.at(j) == var/2){ reset = true; break; }
					if(cirMgr->getGate(_dfs.at(j) )->getDfsPos() < cirMgr->getGate(var/2)->getDfsPos() )break;
					if(fraigAigs(2*_dfs.at(j), (group->isInverse(_dfs.at(j) ) == group->isInverse(var/2) ? (var>>1)<<1 : ((var>>1)<<1)+1) ) )
						reset = true;

					if(reset)break;
					group = getGate(_dfs.at(j) )->getFecPair();
				}

				if(reset)break;
			}
			if(reset)break;
		}

	}

	#endif//FRAIG_MY_VER
	/*****************************************************************
	                 Version3  (generate from version1)
	******************************************************************/
	#ifndef FRAIG_MY_VER

	FecPair* group;
	unsigned var;

	while(!_fec->empty() ){

		for(auto &i : _dfs){
			if(!getGate(i)->isAig() )continue;
			group = getGate(i)->getFecPair();
			if(!group)continue;

			var = group->getMinDfs();
			if(i == var/2)continue;

			//const0
			if(var/2 == 0){
				if(fraigConst(group->isInverse(0) == group->isInverse(i) ? 2*i : 2*i +1) )break;
				else continue;
			}

			//two Aig
			if(cirMgr->getGate(i)->getDfsPos() < cirMgr->getGate(var/2)->getDfsPos() )continue;
			if(fraigAigs(2*i, (group->isInverse(i) == group->isInverse(var/2) ? (var>>1)<<1 : ((var>>1)<<1)+1) ) )break;

		}
	}

	#endif//FRAIG_MY_VER
	
	delete _fec;
	_fec = 0;
	_isFraig = true;
}

/********************************************/
/*   Private member functions about fraig   */
/********************************************/
bool CirMgr::fraigAigs(unsigned var1, unsigned var2) {

	SatSolver sat;
	sat.initialize();
	gateSet pi;
	pi.reserve(_inputs.size() );

	++_flag;
	getGate(var1/2)->setSat(sat, pi, _flag);
	getGate(var2/2)->setSat(sat, pi, _flag);
	int target = sat.newVar();
	sat.addXorCNF(target, getGate(var1/2)->getSatVar(), var1 % 2, getGate(var2/2)->getSatVar(), var2 % 2);
	//sat.assumeRelease();
	sat.assumeProperty(target, true);
	bool satisfied = sat.assumpSolve();

	if(satisfied)simInFraig(sat, pi);
	else {
		if(getGate(var1/2)->getDfsPos() < getGate(var2/2)->getDfsPos() ){
			if(var2 % 2){ --var2; var1 = getInverseVar(var1); }
			
			cout << "Fraig: " << var1/2 <<" merging "; 
			CirGate::printNetFanin((var1 % 2 ? getInverseVar(var2) : var2) ); cout << "...\n";
			
			removeGate(var2/2, getGate(var2/2)->replace(var1) );
		
		}else {
			if(var1 % 2){ --var1; var2 = getInverseVar(var2); }
			
			cout << "Fraig: " << var2/2 <<" merging "; 
			CirGate::printNetFanin((var2 % 2 ? getInverseVar(var1) : var1)); cout << "...\n";
			
			removeGate(var1/2, getGate(var1/2)->replace(var2) );
		}

		setDfs();
		//strash();
	}

	return !satisfied;
}

bool CirMgr::fraigConst(unsigned var) {

	SatSolver sat;
	sat.initialize();
	gateSet pi;
	pi.reserve(_inputs.size() );

	getGate(var/2)->setSat(sat, pi, ++_flag);
	//sat.assumeRelease();
	sat.assumeProperty(getGate(var/2)->getSatVar(), var % 2 == 0 );
	bool satisfied = sat.assumpSolve();

	if(satisfied)simInFraig(sat, pi);
	else {
		cout << "Fraig: 0 merging "; CirGate::printNetFanin(var); cout << "...\n";
		removeGate(var/2, getGate(var/2)->replace((var % 2 ? 1 : 0) ) );
		setDfs();
		//strash();
	}

	return !satisfied;
}

void CirMgr::simInFraig(SatSolver& sat, gateSet& pi) {
	vector<size_t> test;
	test.resize(_inputs.size() );

	for(size_t i = 0; i < test.size(); ++i){
		test.at(i) = getRandomInput();
		if(pi.find(_inputs.at(i) ) != pi.end() ){
			test.at(i) <<= 1;
			if(sat.getValue(getGate(_inputs.at(i) )->getSatVar() ) )++test.at(i);
		}
	}

	simulate(test);

}