/****************************************************************************
  FileName     [ cirMgr.h ]
  PackageName  [ cir ]
  Synopsis     [ Define circuit manager ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_MGR_H
#define CIR_MGR_H

#include <vector>
#include <string>
#include <fstream>
#include <iostream>

using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.

#include "cirGate.h"
#include "cirFecPair.h"
#include "sat.h"

extern CirMgr *cirMgr;

class CirMgr
{
public:
   CirMgr(): _gates(0), _fec(0), _M(0), _Na(0), _flag(0), _isFraig(false), _simLog(0) {}
   ~CirMgr(); 

   // Access functions
   // return '0' if "gid" corresponds to an undefined gate.
   CirGate* getGate(unsigned gid) const;
   size_t inline getFlag() const { return _flag; }

   // Member functions about circuit construction
   bool readCircuit(const string&);

   // Member functions about circuit optimization
   void sweep();
   void optimize();

   // Member functions about simulation
   void randomSim();
   void fileSim(ifstream&);
   void setSimLog(ofstream *logFile) { _simLog = logFile; }

   // Member functions about fraig
   void strash();
   void printFEC() const;
   void fraig();

   // Member functions about circuit reporting
   void printSummary() const;
   void printNetlist() const;
   void printPIs() const;
   void printPOs() const;
   void printFloatGates() const;
   void printFECPairs() const;
   void inline reportFanin(int level, CirGate* gate) const { gate->reportFanin(level, ++_flag); }
   void inline reportFanout(int level, CirGate* gate) const { gate->reportFanout(level, ++_flag); }
   void writeAag(ostream&) const;
   void writeGate(ostream&, CirGate*) const;
   static inline unsigned getInverseVar(unsigned var) { return (var % 2) ? --var : ++var; }

private:
   CirGate* *_gates;
   vector<unsigned> _inputs;
   vector<unsigned> _outputs;
   mutable vector<unsigned> _notUsed;

   vector<unsigned> _dfs;

   vector<FecPair* >* _fec;

   unsigned _M, _Na;
   mutable size_t _flag;
   bool _isFraig;

   ofstream           *_simLog;

   //member function
   void setDfs();
   void removeGate(unsigned gateNo, vector<unsigned>);

   //simulation
   void simulate(vector<size_t>& input, unsigned length = sizeof(size_t)*8 );
   static size_t getRandomInput();
   static bool compare (FecPair* &l, FecPair* &r) { return l->getMin() < r->getMin(); }

   //fraig
   bool fraigConst(unsigned var);
   bool fraigAigs(unsigned var1, unsigned var2);
   void simInFraig(SatSolver&, gateSet&);

   //READING ERRORS
   bool checkIdentifier(ifstream&, unsigned&, unsigned&);
   bool checkPIs(ifstream&, unsigned);
   bool checkPOs(ifstream&, unsigned);
   bool checkAIGs(ifstream&);
   bool checkSymbol(ifstream&);
   bool checkPosNum(const string&, string);
   bool checkInputIdx(const string& ,string);
   bool checkOutputIdx(const string& ,string);
   bool checkFanIn(const string& input, string err);
   bool checkId(const string& input, string err);
   bool checkName(const string&);

};

#endif // CIR_MGR_H
