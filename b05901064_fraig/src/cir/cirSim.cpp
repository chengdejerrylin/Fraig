/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir simulation functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cassert>
#include <cstring>
#include <climits>
#include <cmath>

#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Keep "CirMgr::randimSim()" and "CirMgr::fileSim()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/************************************************/
/*   Public member functions about Simulation   */
/************************************************/
void CirMgr::randomSim() {
	size_t pattern = 0;
	vector<size_t> data;
	data.resize(_inputs.size() );

	if( _inputs.size() <= log(sizeof(size_t) *8.0 *100.0 / 1.5)/log(2) ){

		//static
		double goal = 1.5*pow(2, _inputs.size() );
		while( pattern <= goal ){
			for(auto &i : data)i = getRandomInput();
			simulate(data);
			pattern += (sizeof(size_t)*8 );
		}
	}else {

		//dynamic
		unsigned time = 0;
		const unsigned goal = 18;
		const float rate = 0.0001;
		unsigned last = 0;

		while (time < goal) {
			for(auto &i : data)i = getRandomInput();
			simulate(data);
			pattern += (sizeof(size_t)*8 );

			if(_fec->empty() )break;
			if(_fec->size() < float(_Na)*0.001)break;

			if(last != 0){
				if(abs (_fec->size() - last ) < rate * float(last) )++time;
				else time = 0;
			}

			last = _fec->size();
		}

	}

	cout << pattern << " patterns simulated.\n";
}

void CirMgr::fileSim(ifstream& patternFile) {
	string temp, token;
	bool error = false;
	size_t pattern = 0, print = 0, colNo;

	
	vector<size_t> input;
	input.resize(_inputs.size() );

	while(getline(patternFile, temp) ){
		if(temp.empty() )continue;

		colNo = 0;
		while(colNo != string::npos){
			colNo = myStrGetTok(temp, token, colNo);
			if(token.size() != _inputs.size() ){
				cerr << "Error: Pattern(" << token << ") length(" << token.size()
	    			 << ") does not match the number of inputs(" << _inputs.size() << ") in a circuit!!\n";
	    		error = true;
	    		break;
			}

			for(size_t i = 0; i < token.size(); ++i)
				if(token.at(i) == '1')input.at(i) += (size_t(1) << (pattern % (sizeof(size_t)*8) ) );
				else if(token.at(i) != '0'){
					cerr << "Error: Pattern(" << token << ") contains a non-0/1 character(\'" << token.at(i) << "\').\n";
					error = true;
					break;
				}
			if(error)break;

			if((++pattern) % (sizeof(size_t)*8) == 0){
				simulate(input);
				print += sizeof(size_t)*8;
				for(auto &i : input)i = 0;
			}
		}

		if(error)break;
		
	}

	if(!error && pattern % (sizeof(size_t)*8) ){
			simulate(input, pattern % (sizeof(size_t)*8) );
			print += pattern % (sizeof(size_t)*8);
	}

	cout << print << " patterns simulated.\n";
}

/*************************************************/
/*   Private member functions about Simulation   */
/*************************************************/
void CirMgr::simulate(vector<size_t>& input, unsigned length) {

	//simulate
	for(size_t i = 0; i < _inputs.size(); ++i)
		((PIGate*)getGate(_inputs.at(i) ) )->setInput(input.at(i) );	
	for(auto& i : _dfs)getGate(i)->simulate();

	//log
	if(_simLog){
		for(unsigned i = 0; i < length; ++i){
			for(auto &j : input)
				if(j & ( ((size_t)1) << i) ) (*_simLog) << '1';
				else (*_simLog) << '0';

			(*_simLog) << ' '; 

			for(auto &j : _outputs)
				if(getGate(j)->getValue() & ( ((size_t)1) << i) ) (*_simLog) << '1';
				else (*_simLog) << '0';

			(*_simLog) << '\n';
		}
	}

	//FEC
	if(!_fec){
		_fec = new vector<FecPair*>;
		FecPair* group = new FecPair(0);

		for(auto &i : _dfs)
			if(getGate(i)->isAig() )group->insert(i*2);
		group->generate(_fec);

		delete group;

	}else{
		vector<FecPair* >* temp = new vector<FecPair*>;

		for(auto &i : *_fec){
			i->generate(temp);
			delete i;
		}

		delete _fec;
		_fec = temp;
	}
}

size_t CirMgr::getRandomInput() {
	size_t ans = 0;
	unsigned size = 0;

	while(size != sizeof(size_t) ){
		ans <<= 1;
		ans += rnGen(2);
		ans <<= (sizeof(int)*8 -1 );
		ans += rnGen(INT_MAX);

		size += sizeof(int);
	}

	return ans;
}