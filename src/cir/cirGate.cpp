/****************************************************************************
  FileName     [ cirGate.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define class CirAigGate member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdarg.h>
#include <cassert>

#include "cirGate.h"
#include "cirMgr.h"
#include "cirFecPair.h"
#include "util.h"

//define GATE_CPP_D
using namespace std;

// TODO: Keep "CirGate::reportGate()", "CirGate::reportFanin()" and
//       "CirGate::reportFanout()" for cir cmds. Feel free to define
//       your own variables and functions.

extern CirMgr *cirMgr;
const vector<unsigned> CirGate::nullVector;
const string CirGate::nullString;

/**************************************/
/*   class CirGate member functions   */
/**************************************/

bool CirGate::isFloating() const {
	vector<unsigned> fanin = getFanin();

	for(auto &i : fanin)if(cirMgr->getGate(i/2)->isUndef())return true;
	return false;
}

void CirGate::addDfsList(vector<unsigned> &dfs, size_t flag) const {
	if(_flag == flag)return;
	vector<unsigned> fanin = getFanin();
	
	for (auto &i : fanin){
		CirGate* gate = cirMgr->getGate(i/2);
		if(!gate->isUndef() )gate->addDfsList(dfs, flag);
	}

	dfs.push_back(_gateNo);
	_dfsPos = dfs.size();

	_flag = flag;
}

void CirGate::addWriteDfs(vector<unsigned>& aig, vector<unsigned> & input, size_t flag) const {
	if(_flag == flag)return;
	vector<unsigned> fanin = getFanin();

	for (auto &i : fanin){
		CirGate* gate = cirMgr->getGate(i/2);
		if(!gate->isUndef() && !gate->isConst() )gate->addWriteDfs(aig, input, flag);
	}
	if(isAig() )aig.push_back(_gateNo);
	if(isPI() )input.push_back(_gateNo);
	_flag = flag;
}

void CirGate::connectGate(CirGate* *gates) const {
	vector<unsigned> fanin = getFanin();
      for(auto& in : fanin){
         if(!gates[in/2])gates[in/2] = new UndefGate(in/2);
         if(in % 2)gates[in/2]->addFanout(2*_gateNo +1);
         else gates[in/2]->addFanout(2*_gateNo);
      }
}

vector<unsigned> CirGate::remove() {
	vector<unsigned> ans = getFanin();
	for( auto &i : ans)cirMgr->getGate(i/2)->removeFanout(_gateNo);

	return ans;
}

vector<unsigned> CirGate::replace(unsigned var) {
	for( auto &i : getFanout() ){

		#ifdef GATE_CPP_D
		cout << "Replace Gate " << i/2 << "'s Fanin from Gate " << _gateNo << " to var " << var << ".\n";
		#endif//GATE_CPP_D

		cirMgr->getGate(i/2)->replaceFanin(_gateNo, var);

		if((var %2) ^ (i %2) )cirMgr->getGate(var/2)->addFanout((i % 2 ? i-1: i+1) );
		else cirMgr->getGate(var/2)->addFanout(i);
	}

	return remove();
}

void CirGate::printGate() const {
	vector<unsigned> fanin  = getFanin();

	cout << setw(4) << left << getTypeStr();
	cout << _gateNo;
	for (unsigned i = 0; i < fanin.size(); ++i){
		cout << ' ';
		printNetFanin(fanin.at(i));
	}
	if(!getName().empty())cout << " (" << getName() << ')';

	cout << endl;

}

void CirGate::reportGate() const {
	string data = getTypeStr() + "(" + to_string(_gateNo) + ")";
	if(!getName().empty()) data +=  "\"" + getName() + "\"";
	data += ", line " + to_string(_lineNo); 

	cout << "================================================================================\n";
	cout << "= " << left << data << '\n';
	cout << "= FECs:" ; printFecPair(); cout << '\n';
	cout << "= Value: "; printValue()  ; cout << '\n';
	cout << "================================================================================\n";
}

void CirGate::printFecPair() const {
	FecPair *f = getFecPair();

	if(f != 0){
		f->getGates();
		if(f->isInverse(_gateNo)){
			for(auto &i : f->getGates() ){
				if(i == 2*_gateNo +1)continue;
				cout << ' ';
				printNetFanin(CirMgr::getInverseVar(i) );
			}
		}else {
			for(auto &i : f->getGates() ){
				if(i == 2*_gateNo)continue;
				cout << ' ';
				printNetFanin(i);
			}
		}
	}
}

void CirGate::reportFanin(int level, size_t flag, bool inverse, unsigned step) const {
   assert (level >= 0);

   vector<unsigned> fanin = getFanin();
   if(!reportFan(level, flag, inverse, step, !fanin.empty() ) && level >0){
   	for(auto &i : fanin)cirMgr->getGate(i/2)->reportFanin(level-1, flag, i%2, step+1);
   }
   
}

void CirGate::reportFanout(int level, size_t flag, bool inverse, unsigned step) const {
   assert (level >= 0);

   if(!reportFan(level, flag, inverse, step, !getFanout().empty() ) && level >0)
   	for(auto &i : getFanout())cirMgr->getGate(i/2)->reportFanout(level-1, flag, i%2, step+1);
}

bool CirGate::reportFan(int level, size_t flag, bool inverse, unsigned step, bool printStar) const {
	for(unsigned i = 0; i < step; ++i)cout << "  ";
   if(inverse)cout << '!';
   cout << getTypeStr() << ' ' << _gateNo;

   if(_flag != flag)_isPrintFan = false;
   _flag = flag;
   if(level != 0 && printStar && _isPrintFan)cout << " (*)";
   cout << endl;
   
   bool ans = _isPrintFan;
   if(level != 0 && printStar)_isPrintFan = true;
   return ans;
}

void CirGate::printNetFanin(unsigned var) {
	if(cirMgr->getGate(var/2) )
		if(cirMgr->getGate(var/2)->isUndef())cout << '*';

	if(var % 2)cout << '!';
	cout << var/2;
}

void CirGate::printValue() const {
	for(int shift = 63; shift >=0; --shift){
		if(_value & ((size_t)1 << shift) )cout << '1';
		else cout << '0';
		if(shift % 8 == 0 && shift != 0)cout << '_';
	}
}

size_t CirGate::getValue(unsigned var){
	CirGate* gate = cirMgr->getGate(var/2);

	size_t ans = gate->_value;
	if(var % 2)ans = ~ans;
	return ans;
}

/**************************************/
/*              AIG Class             */
/**************************************/

void AigGate::setSat(SatSolver& sat, gateSet& pi, size_t flag) const {
	if(isFlag(flag) )return;

	_satVar = sat.newVar();
	cirMgr->getGate(_fanin1/2)->setSat(sat, pi, flag);
	cirMgr->getGate(_fanin2/2)->setSat(sat, pi, flag);

	sat.addAigCNF(_satVar, cirMgr->getGate(_fanin1/2)->getSatVar(), _fanin1 % 2,
						   cirMgr->getGate(_fanin2/2)->getSatVar(), _fanin2 % 2);
}

int AigGate::optimize() const {
      if( _fanin1 == 0 || _fanin2 == 0)return 0;
      if( _fanin1 == 1)return _fanin2; if( _fanin2 == 1)return _fanin1;
      if( _fanin1 == _fanin2)return _fanin1;
      if( _fanin1/2 == _fanin2/2) return 0;

      return -1;
   };