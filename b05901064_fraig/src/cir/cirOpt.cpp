/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir optimization functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Please keep "CirMgr::sweep()" and "CirMgr::optimize()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/**************************************************/
/*   Public member functions about optimization   */
/**************************************************/
// Remove unused gates
// DFS list should NOT be changed
// UNDEF, float and unused list may be changed
void CirMgr::sweep() {
	size_t pos = 0;

	while(pos != _notUsed.size()){
		unsigned gateNo = _notUsed.at(pos);

		if(getGate(gateNo)->isPI())++pos;
		else {
			cout << "Sweeping: " << getGate(gateNo)->getTypeStr() << '(' << getGate(gateNo)->getGateNo() << ") removed...\n";
			removeGate(gateNo, getGate(gateNo)->remove() );
			//_notUsed.erase(_notUsed.begin() + pos);
			_notUsed.at(pos) = _notUsed.back();
			_notUsed.pop_back();
		}
	}
}

// Recursively simplifying from POs;
// _dfsList needs to be reconstructed afterwards
// UNDEF gates may be delete if its fanout becomes empty...
void CirMgr::optimize() {
	bool dfs = false;
	int ans;
	CirGate* gate;

	for(auto& i : _dfs){
		gate = getGate(i);
		ans = gate->optimize();

		if(ans != -1){
			cout << "Simplifying: ";
			gate->printNetFanin((ans/2)*2 );
			cout << " merging ";
			gate->printNetFanin( ans % 2 ? gate->getGateNo()*2 +1 : gate->getGateNo()*2 );
			cout << "...\n";
			dfs = true;

			removeGate(gate->getGateNo(), gate->replace(ans) );
		}
	}
	if(dfs)setDfs();
}

/***************************************************/
/*   Private member functions about optimization   */
/***************************************************/