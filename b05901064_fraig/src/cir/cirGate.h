/****************************************************************************
  FileName     [ cirGate.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic gate data structures ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_GATE_H
#define CIR_GATE_H

#include <string>
#include <vector>
#include <iostream>

#include "cirDef.h"
#include "sat.h"

using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.

class CirGate;
//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------
class CirGate{

public:
   CirGate(unsigned gateNo, unsigned lineNo):
   _flag(0), _gateNo(gateNo), _lineNo(lineNo), _dfsPos(0), _isPrintFan(false), _value(0) {}
   virtual ~CirGate()   {}

   // Basic access methods 
   virtual string getTypeStr() const = 0;
   virtual inline const string& getName() const { return nullString; }
   virtual inline const vector<unsigned> getFanin() const  { return nullVector; }
   virtual inline const vector<unsigned>& getFanout() const  { return nullVector; }
   virtual inline bool isUsed() const { return !( getFanout().empty() ); }
   virtual inline bool isAig() const { return false; }
   virtual inline bool isPI() const { return false; }
   virtual inline bool isUndef() const { return false; }
   virtual inline bool isConst() const { return false; }
   virtual inline int optimize() const { return -1; }
   virtual inline int getSatVar() const { return -1; }
   virtual inline FecPair* getFecPair() const { return 0; }
   unsigned inline getLineNo() const { return _lineNo; }
   unsigned inline getGateNo() const { return _gateNo; }
   unsigned inline getDfsPos() const { return _dfsPos; }
   size_t inline getValue() const { return _value; }
   size_t inline getFlag() const { return _flag; }
   bool isFloating() const;
   void addDfsList(vector<unsigned>&, size_t) const;
   void addWriteDfs(vector<unsigned>&, vector<unsigned>&, size_t) const;
   bool inline isFlag(size_t f) const { if(_flag == f)return true; _flag = f; return false;}


   // set function
   virtual void setName(string n) {}
   virtual void setFecPair(FecPair*) const {}
   virtual void setSat(SatSolver& sat, gateSet& pi, size_t falg) const {}
   virtual inline void simulate() {}
   void connectGate(CirGate**) const;
   vector<unsigned> remove();
   vector<unsigned> replace(unsigned var);

   // Printing functions
   virtual ostream& writeFile(ostream& os) const { return os; }
   void printGate() const;
   void reportGate() const;
   void reportFanin(int level, size_t flag, bool inverse = false, unsigned step = 0) const;
   void reportFanout(int level, size_t flag, bool inverse = false, unsigned step = 0) const;
   static void printNetFanin(unsigned);
   
private:
   mutable size_t _flag;
   const unsigned _gateNo;
   const unsigned _lineNo;
   mutable unsigned _dfsPos;
   mutable bool _isPrintFan;
   static const vector<unsigned> nullVector;
   static const string nullString;
 
   bool reportFan(int level, size_t flag, bool inverse, unsigned step, bool printStar) const;
   void printValue() const;
   void printFecPair() const;

protected:
   mutable size_t _value;

   virtual void addFanout(unsigned var) {}
   virtual void replaceFanin(unsigned gateNo, unsigned replace) {}
   virtual void removeFanout(unsigned gateNo) {}
   static size_t getValue(unsigned var);
};


class PIGate : public CirGate{
public:
   PIGate(unsigned gateNo, unsigned lineNo): CirGate(gateNo, lineNo), _satVar(-1) {}
   ~PIGate() {}
   virtual inline string getTypeStr() const override { return "PI"; }
   virtual inline bool isPI() const override{ return true; }
   virtual inline const string& getName() const override { return _name; }
   virtual inline const vector<unsigned>& getFanout() const override { return _fanout; }
   virtual inline int getSatVar() const  override{ return _satVar; }
   virtual ostream& writeFile(ostream& os) const override { return os << ( getGateNo()*2 ) << endl; }
   virtual void setName(string n) override { _name = n; }
   virtual void setSat(SatSolver& sat, gateSet& pi, size_t flag) const override {
      if(isFlag(flag) )return; _satVar = sat.newVar(); pi.insert(getGateNo() ); }
   void setInput (size_t i) { _value = i; }

private:
   vector<unsigned> _fanout;
   string _name;
   mutable int _satVar;

protected:
   virtual void addFanout (unsigned var) override { _fanout.push_back(var); }
   virtual void removeFanout( unsigned gateNo ) override {
      for( vector<unsigned>::iterator it = _fanout.begin(); it != _fanout.end(); ++it)
         if( *it == gateNo*2 || *it == gateNo*2 +1){
            _fanout.erase(it); break;
         }
   }
};


class POGate : public CirGate{
public:
   POGate(unsigned gateNo, unsigned lineNo, unsigned var): CirGate(gateNo, lineNo), _fanin(var) {}
   ~POGate() {}
   virtual inline string getTypeStr() const override { return "PO"; }
   virtual inline const string& getName() const override { return _name; }
   virtual inline const vector<unsigned> getFanin() const override { vector<unsigned> ans; ans.push_back(_fanin); return ans; }
   virtual ostream& writeFile(ostream& os) const override { return os << _fanin << endl; }
   virtual inline bool isUsed() const override { return true; }
   virtual void setName(string n) override { _name = n; }
   virtual inline void simulate() override { _value = getValue(_fanin); }

private:
   unsigned _fanin;
   string _name;

protected:
   virtual void replaceFanin(unsigned gateNo, unsigned replace) override { 
      if(_fanin == gateNo*2)_fanin = replace;
      else if(_fanin == gateNo*2 +1)_fanin = (replace % 2 ? replace-1 : replace +1);
      else assert(false);
   }
};


class AigGate : public CirGate{
public:
   AigGate(unsigned gateNo, unsigned lineNo, unsigned var1, unsigned var2): 
      CirGate(gateNo, lineNo), _fanin1(var1), _fanin2(var2), _satVar(-1), _fec(0) {}
   ~AigGate() {}
   virtual inline string getTypeStr() const override { return "AIG"; }
   virtual inline void simulate() override { _value = (getValue(_fanin1) & getValue(_fanin2) ); }
   virtual ostream& writeFile(ostream& os) const override {
      return os << ( getGateNo()*2 ) << ' ' << _fanin1 << ' ' << _fanin2 << endl; }
   virtual inline const vector<unsigned>& getFanout() const override { return _fanout; }
   virtual inline const vector<unsigned> getFanin() const override {
      vector<unsigned> v; v.push_back(_fanin1); v.push_back(_fanin2); return v; }
   virtual inline int getSatVar() const  override{ return _satVar; }
   virtual inline bool isAig() const override { return true; }
   virtual void setSat(SatSolver& sat, gateSet& pi, size_t flag) const override;
   virtual inline int optimize() const override;

   inline size_t getStrashKey(const unsigned M) const {
      return _fanin1 > _fanin2 ? (size_t)_fanin1 *(size_t)(_fanin1 +1)/2 + _fanin2 :
                                 (size_t)_fanin2 *(size_t)(_fanin2 +1)/2 + _fanin1 ;
      //return _fanin1 > _fanin2 ? (size_t)((_fanin1 -_fanin2)*(2*M+1) ) +(_fanin1 +_fanin2) : 
      //                           (size_t)((_fanin2 -_fanin1)*(2*M+1) ) +(_fanin1 +_fanin2) ;
   }
   virtual inline void setFecPair(FecPair* f) const override { _fec = f; }
   virtual inline FecPair* getFecPair() const override { return _fec; }

private:
   unsigned _fanin1, _fanin2;
   vector<unsigned> _fanout;
   mutable int _satVar;
   mutable FecPair* _fec;

protected:
   virtual void addFanout(unsigned var) override { _fanout.push_back(var); }
   virtual void replaceFanin(unsigned gateNo, unsigned replace) override { 
      if(_fanin1 == gateNo*2)_fanin1 = replace;
      else if(_fanin1 == gateNo*2 +1)_fanin1 = (replace % 2 ? replace-1 : replace +1);
      else if(_fanin2 == gateNo*2)_fanin2 = replace;
      else if(_fanin2 == gateNo*2 +1)_fanin2 = (replace % 2 ? replace-1 : replace +1);
      else assert(false);
   }
   virtual void removeFanout( unsigned gateNo ) override {
      for( vector<unsigned>::iterator it = _fanout.begin(); it != _fanout.end(); ++it)
         if( *it == gateNo*2 || *it == gateNo*2 +1){
            _fanout.erase(it); break;
         }
   }

};


class ConstGate : public CirGate{
public:
   ConstGate(): CirGate(0,0), _satVar(-1), _fec(0) {}
   ~ConstGate() {}
   virtual inline string getTypeStr() const override { return "CONST"; }
   virtual inline bool isConst() const override { return true; }
   virtual inline const vector<unsigned>& getFanout() const override { return _fanout; }
   virtual inline int getSatVar() const  override{ return _satVar; }
   virtual inline bool isUsed() const override { return true; }
   virtual void setSat(SatSolver& sat, gateSet& pi, size_t flag) const override { 
      if(isFlag(flag) )return; _satVar = sat.newVar(); sat.assumeProperty(_satVar, false); }
   virtual inline void setFecPair(FecPair* f) const override { _fec = f; }
   virtual inline FecPair* getFecPair() const override { return _fec; }

private:
   vector<unsigned> _fanout;
   mutable int _satVar;
   mutable FecPair* _fec;

protected:
   virtual void addFanout(unsigned var) override { _fanout.push_back(var); }
   virtual void removeFanout( unsigned gateNo ) override {
      for( vector<unsigned>::iterator it = _fanout.begin(); it != _fanout.end(); ++it)
         if( *it == gateNo*2 || *it == gateNo*2 +1){
            _fanout.erase(it); break;
         }
   }
};


class UndefGate : public CirGate{
public:
   UndefGate(unsigned gateNo): CirGate(gateNo, 0) {}
   ~UndefGate() {}
   virtual inline string getTypeStr() const override { return "UNDEF"; }
   virtual inline const vector<unsigned>& getFanout() const override { return _fanout; }
   virtual inline bool isUndef() const override { return true; }
   virtual void setSat(SatSolver& sat, gateSet& pi, size_t flag) const override { 
      if(isFlag(flag) )return; _satVar = sat.newVar(); sat.assumeProperty(_satVar, false); }
   virtual inline int getSatVar() const  override{ return _satVar; }
   virtual inline void setFecPair(FecPair* f) const override { _fec = f; }
   virtual inline FecPair* getFecPair() const override { return _fec; }
   

private:
   vector<unsigned> _fanout;
   mutable int _satVar;
   mutable FecPair* _fec;

protected:
   virtual void addFanout(unsigned var) override { _fanout.push_back(var); }
   virtual void removeFanout( unsigned gateNo ) override {
      for( vector<unsigned>::iterator it = _fanout.begin(); it != _fanout.end(); ++it)
         if( *it == gateNo*2 || *it == gateNo*2 +1){
            _fanout.erase(it); break;
         }
   }
};

#endif // CIR_GATE_H
