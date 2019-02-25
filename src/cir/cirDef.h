/****************************************************************************
  FileName     [ cirDef.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic data or var for cir package ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2012-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_DEF_H
#define CIR_DEF_H

#include <unordered_map>
#include <unordered_set>

using namespace std;

// TODO: define your own typedef or enum
class CirGate;
class CirMgr;
class SatSolver;
class FecPair;

typedef pair<size_t, unsigned> strashData;
typedef unordered_map<size_t, unsigned> strashMap;
typedef pair<size_t, FecPair* > fecData;
typedef unordered_map<size_t, FecPair* > fecMap;

typedef unordered_set<unsigned> gateSet;
#endif // CIR_DEF_H
