#ifndef CIR_FECPAIR_H
#define CIR_FECPAIR_H

#include <vector>
#include <iostream>

#include "cirDef.h"
#include "cirGate.h"

using namespace std;

class FecPair {

public:
	FecPair(unsigned var, bool first = true);
	~FecPair();

	//access
	inline const vector<unsigned>& getGates() { setAll(); return _vec; }
	inline unsigned getMin() { setAll();  return (unsigned)_min; }
	inline bool isInverse(unsigned gateNo) { return _hash->find(gateNo*2) == _hash->end(); }
	inline bool isSolo() const { return _hash->size() <= 1; }
	inline bool find(unsigned gateNo) const { return _hash->find(gateNo*2) != _hash->end() || _hash->find(gateNo*2 +1) != _hash->end(); }
	inline unsigned getMinDfs() { setDfsMin(); return unsigned(_dfsMin); } 

	//set
	void insert(unsigned var);
	void generate(vector<FecPair* >*);
	void removeOutDfs();

	//print
	void print();

private:
	gateSet* _hash;
	vector<unsigned> _vec;
	int _min;
	int _dfsMin;
	bool _isVecSet;
	bool _first;
	bool _used;

	//adjustment
	void setAll();
	void checkInverse();
	void setMin();
	void setDfsMin();
	void makeVec();

	void erase(unsigned var);
	void generateFirst(vector<FecPair*> *, fecMap&);
	void generate(vector<FecPair*> *, fecMap&);
	
};

#endif//CIR_FECPAIR_H