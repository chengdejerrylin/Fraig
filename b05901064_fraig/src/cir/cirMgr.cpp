/****************************************************************************
  FileName     [ cirMgr.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir manager functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cstdio>
#include <ctype.h>
#include <cassert>
#include <cstring>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Implement memeber functions for class CirMgr

/*******************************/
/*   Global variable and enum  */
/*******************************/
CirMgr* cirMgr = 0;

enum CirParseError {
   EXTRA_SPACE,
   MISSING_SPACE,
   ILLEGAL_WSPACE,
   ILLEGAL_NUM,
   ILLEGAL_IDENTIFIER,
   ILLEGAL_SYMBOL_TYPE,
   ILLEGAL_SYMBOL_NAME,
   MISSING_NUM,
   MISSING_IDENTIFIER,
   MISSING_NEWLINE,
   MISSING_DEF,
   CANNOT_INVERTED,
   MAX_LIT_ID,
   REDEF_GATE,
   REDEF_SYMBOLIC_NAME,
   REDEF_CONST,
   NUM_TOO_SMALL,
   NUM_TOO_BIG,

   DUMMY_END
};

/**************************************/
/*   Static varaibles and functions   */
/**************************************/
static unsigned lineNo = 0;  // in printint, lineNo needs to ++
static unsigned colNo  = 0;  // in printing, colNo needs to ++
static char buf[1024];
static string errMsg;
static int errInt;
static CirGate *errGate;
static char tab = 9;
//static string white = " " + tab;
static char taba[3] = {' ', tab, '\0'};
static string white(taba);

static bool parseError(CirParseError err) {
   switch (err) {
      case EXTRA_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Extra space character is detected!!" << endl;
         break;
      case MISSING_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing space character!!" << endl;
         break;
      case ILLEGAL_WSPACE: // for non-space white space character
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal white space char(" << errInt
              << ") is detected!!" << endl;
         break;
      case ILLEGAL_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal "
              << errMsg << "!!" << endl;
         break;
      case ILLEGAL_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal identifier \""
              << errMsg << "\"!!" << endl;
         break;
      case ILLEGAL_SYMBOL_TYPE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal symbol type (" << errMsg << ")!!" << endl;
         break;
      case ILLEGAL_SYMBOL_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Symbolic name contains un-printable char(" << errInt
              << ")!!" << endl;
         break;
      case MISSING_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing " << errMsg << "!!" << endl;
         break;
      case MISSING_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing \""
              << errMsg << "\"!!" << endl;
         break;
      case MISSING_NEWLINE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": A new line is expected here!!" << endl;
         break;
      case MISSING_DEF:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing " << errMsg
              << " definition!!" << endl;
         break;
      case CANNOT_INVERTED:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": " << errMsg << " " << errInt << "(" << errInt/2
              << ") cannot be inverted!!" << endl;
         break;
      case MAX_LIT_ID:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Literal \"" << errInt << "\" exceeds maximum valid ID!!"
              << endl;
         break;
      case REDEF_GATE:
         cerr << "[ERROR] Line " << lineNo+1 << ": Literal \"" << errInt
              << "\" is redefined, previously defined as "
              << errGate->getTypeStr() << " in line " << errGate->getLineNo()
              << "!!" << endl;
         break;
      case REDEF_SYMBOLIC_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ": Symbolic name for \""
              << errMsg << errInt << "\" is redefined!!" << endl;
         break;
      case REDEF_CONST:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Cannot redefine const (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_SMALL:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too small (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_BIG:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too big (" << errInt << ")!!" << endl;
         break;
      default: break;
   }
   return false;
}

static bool checkSpace(const string& s) {
   if(s.size() <= colNo)return parseError(MISSING_SPACE);
   if(s.at(colNo) == 9){
      errInt = 9;
      return parseError(ILLEGAL_WSPACE);
   }
   if(s.at(colNo) != ' ')return parseError(MISSING_SPACE);
   ++colNo;
   return true;
}

static bool checkNotSpace(const string& s, CirParseError missing, string msg = errMsg) {
   if(s.size() <= colNo){
      errMsg = msg;
      return parseError(missing);
   }
   if(s.at(colNo) == 9){
      errInt = 9;
      return parseError(ILLEGAL_WSPACE);
   }
   if(s.at(colNo) == ' ')return parseError(EXTRA_SPACE);
   return true;
}

static bool checkNewLine(const string& s){
   if(colNo < s.size())return parseError(MISSING_NEWLINE);
   
   ++lineNo;
   colNo = 0;
   return true;
}

static size_t StrGetTokTab(const string& str, string& tok, size_t pos = 0){
   string tok1, tok2;
   size_t pos1 = myStrGetTok(str, tok1, pos);
   size_t pos2 = myStrGetTok(str, tok2, pos, tab);
   tok = (pos1 < pos2 ? tok1 : tok2);
   size_t ans = (pos1 < pos2 ? pos1 : pos2);
   return (ans == string::npos ? str.size() : ans);
}
/**************************************************************/
/*   class CirMgr member functions for circuit construction   */
/**************************************************************/
CirMgr::~CirMgr() {
   if(_fec){
      for(auto &i : *_fec)delete i;
      delete _fec;
   }

   if (_gates){
      unsigned O = _outputs.size();
      for(unsigned i = 0; i < _M +O +1; ++i)
         if(_gates[i])delete _gates[i];

      delete [] _gates;
   }
}

CirGate* CirMgr::getGate(unsigned gid) const{
   return (gid > _M + _outputs.size() ? 0 : _gates[gid]);
}

bool CirMgr::readCircuit(const string& fileName) {
   ifstream ifs(fileName.c_str());
   if(!ifs.is_open()){
      cerr << "Cannot open design \"" << fileName << "\"!!\n";
      return false;
   }

   unsigned I, O;

   if(!checkIdentifier(ifs, I, O)){ifs.close(); return false; }

   _gates = new CirGate*[_M +O +1];
   for(unsigned i = 0 ; i < _M+O+1; ++i)_gates[i] = 0;
   _gates[0] = new ConstGate();
   _inputs.resize(I);
   _outputs.resize(O);

   for(unsigned i = 0; i < I; ++i)
      if(!checkPIs(ifs, i) ){ ifs.close(); return false; }
   for(unsigned i = 0; i < O; ++i)
      if(!checkPOs(ifs, i) ){ ifs.close(); return false; }
   for(unsigned i = 0; i <_Na; ++i)
      if(!checkAIGs(ifs) ){ ifs.close(); return false; }
   if(!checkSymbol(ifs) ){ ifs.close(); return false;}

   for(unsigned i = 1; i < _M +O +1; ++i)
      if(_gates[i])_gates[i]->connectGate(_gates);

   setDfs();
   for(unsigned i = 0; i <= _M +O; ++i)
      if(_gates[i])
         if(!_gates[i]->isUsed())_notUsed.push_back(i);

   ifs.close();
   return true;
}

/**********************************************************/
/*   class CirMgr member functions for circuit printing   */
/**********************************************************/
/*********************
Circuit Statistics
==================
  PI          20
  PO          12
  AIG        130
------------------
  Total      162
*********************/
void CirMgr::printSummary() const {
   unsigned Ni = _inputs.size(), No = _outputs.size();
   cout << "\nCircuit Statistics\n";
   cout << "==================\n";
   cout << "  PI" << setw(12) << right << Ni << endl;
   cout << "  PO" << setw(12) << right << No << endl;
   cout << "  AIG" << setw(11) << right << _Na << endl;
   cout << "------------------\n";
   cout << "  Total" << setw(9) << right << Ni + No + _Na << endl;
}

void CirMgr::printNetlist() const{
   cout << endl;
   for (unsigned i = 0, n = _dfs.size(); i < n; ++i) {
      cout << "[" << i << "] ";
      getGate(_dfs[i])->printGate();
   }
}

void CirMgr::printPIs() const {
   cout << "PIs of the circuit:";
   for(auto& i: _inputs)cout << ' ' << i;
   cout << endl;
}

void CirMgr::printPOs() const {
   cout << "POs of the circuit:";
   for(auto& i: _outputs)cout << ' ' << i;
   cout << endl;
}

void CirMgr::printFloatGates() const {
   vector<unsigned> ans;
   unsigned O = _outputs.size();

   for(unsigned i = 0; i <=_M+O; ++i)
      if(_gates[i])
         if(_gates[i]->isFloating())ans.push_back(i);
   if(!ans.empty()){
      cout << "Gates with floating fanin(s):";
      for(auto& j : ans)cout << ' ' << j;
      cout << endl;
   }

   if(!_notUsed.empty()){
      cout << "Gates defined but not used  :";
      sort(_notUsed.begin(), _notUsed.end() );
      for(auto& j : _notUsed)cout << ' ' << j;
      cout << endl;
   }   
}

void CirMgr::printFECPairs() const {
   if(!_fec)return;

   sort((*_fec).begin(), (*_fec).end(), compare);
   for(size_t i = 0; i < _fec->size(); ++i){
      cout << '[' << i << ']';
      _fec->at(i)->print();
      cout << endl;
   }
}

void CirMgr::writeAag(ostream& os) const {
   vector<unsigned> aig;
   for(auto& i : _dfs)if(getGate(i)->isAig() )aig.push_back(i);

   os << "aag " << _M << ' ' << _inputs.size() << " 0 " << _outputs.size() << ' ' << aig.size() << endl;
   for(auto& i : _inputs)getGate(i)->writeFile(os);
   for(auto& i : _outputs)getGate(i)->writeFile(os);
   for(auto& i : aig)getGate(i)->writeFile(os);

   for(size_t i = 0; i < _inputs.size(); ++i)
      if(getGate(_inputs.at(i) )->getName() != "")os << 'i' << i << ' ' << getGate(_inputs.at(i) )->getName() << endl; 
   for(size_t i = 0; i < _outputs.size(); ++i)
      if(getGate(_outputs.at(i) )->getName() != "")os << 'o' << i << ' ' << getGate(_outputs.at(i) )->getName() << endl;
   
   os << "c\n";
   os << "AAG output by Chung-Yang (Ric) Huang\n";
   //os << "AAG output By Cheng-De Lin.\n";
}

void CirMgr::writeGate(ostream& os, CirGate *g) const {
   vector<unsigned> aig, PI;
   g->addWriteDfs(aig, PI, ++_flag);

   os << "aag " << _M << ' ' << PI.size() << " 0 1 "<< aig.size() << endl;
   for(auto& i : PI)getGate(i)->writeFile(os);
   os << g->getGateNo()*2 << endl;
   for(auto& i : aig)getGate(i)->writeFile(os);

   for(size_t i = 0; i < _inputs.size(); ++i)
      if(getGate(_inputs.at(i) )->getName() != "")os << 'i' << i << ' ' << getGate(_inputs.at(i) )->getName() << endl; 
   
   os << "c\n";
   //os << "AAG output by Chung-Yang (Ric) Huang\n";
   os << "AAG output By Cheng-De Lin.\n";
}

//*****************************************************************************
//*                  Private Function
//*****************************************************************************

void CirMgr::setDfs()  {
   _dfs.clear();
   ++_flag;

   for(auto& i : _outputs)
      getGate(i)->addDfsList(_dfs, _flag);

   //fec
   if(_fec){
      size_t i = 0;
      while(i < _fec->size() ){
         _fec->at(i)->removeOutDfs();

         if(_fec->at(i)->isSolo() ){
            delete _fec->at(i);
            _fec->at(i) = _fec->back();
            _fec->pop_back();
            //_fec->erase(_fec->begin() +i);
         }else ++i;
      }
   }
}

void CirMgr::removeGate(unsigned gateNo, vector<unsigned> fanin) {
   if(_gates[gateNo]->isAig() )--_Na;
   delete _gates[gateNo]; _gates[gateNo] = 0;

   int pre = -1;
   for( auto& i : fanin){
      if(pre == (int)i/2)continue;
      pre = (int)i/2;

      CirGate *gate = getGate(i/2);
      if(!gate->isUsed() ){
         if(gate->isUndef() )removeGate(i/2, gate->remove() );
         else {
            _notUsed.push_back(i/2);
         }
      }
   }
}

//*****************************************************************************
//*                  READING ERRORS
//*****************************************************************************

bool CirMgr::checkIdentifier(ifstream& ifs, unsigned& I, unsigned& O){
   ifs.getline(buf, 1024);
   string head(buf);
   if(head.back() == (char)13)head.pop_back();
   lineNo = colNo = 0;
   string temp;

   //aag
   if(!checkNotSpace(head, MISSING_IDENTIFIER, "aag"))return false;
   colNo = StrGetTokTab(head, errMsg, colNo);
   if(errMsg != "aag")return parseError(ILLEGAL_IDENTIFIER);
   if(!checkSpace(head))return false;

   //M
   errMsg = "#M";
   if(!checkPosNum(head, "number of variables"))return false;
   _M = errInt;
   if(!checkSpace(head))return false;

   //I
   errMsg = "#I";
   if(!checkPosNum(head, "number of variables"))return false;
   I = errInt;
   if(!checkSpace(head))return false;

   //L
   errMsg = "#L";
   if(!checkPosNum(head, "number of variables"))return false;
   if(errInt != 0)return parseError(ILLEGAL_NUM); 
   if(!checkSpace(head))return false;

   //O
   errMsg = "#O";
   if(!checkPosNum(head, "number of variables"))return false;
   O = errInt;
   if(!checkSpace(head))return false;

   //AIG
   errMsg = "#A";
   if(!checkPosNum(head, "number of variables"))return false; 
   _Na = errInt;
   if(_M < I + _Na){
      errInt = _M;
      errMsg = "number of variables";
      return parseError(NUM_TOO_SMALL);
   }
   
   return checkNewLine(head);
}

bool CirMgr::checkPIs(ifstream& ifs, unsigned idx){
   ifs.getline(buf, 1024);
   string input(buf);
   if(input.back() == (char)13)input.pop_back();
   errMsg = "PI";

   if(!checkId(input, "PI literal ID") )return false;   
   _gates[errInt] = new PIGate(errInt, lineNo+1);
   _inputs.at(idx) = errInt;

   return checkNewLine(input);
}

bool CirMgr::checkPOs(ifstream& ifs, unsigned idx){
   ifs.getline(buf, 1024);
   string input(buf);
   if(input.back() == (char)13)input.pop_back();
   errMsg = "PO";

   if(!checkFanIn(input,"PO literal ID"))return false;

   _gates[_M +idx +1] = new POGate(_M +idx +1, lineNo+1, errInt);
   _outputs.at(idx) = (_M +idx +1);
   
   return checkNewLine(input);
}

bool CirMgr::checkAIGs(ifstream& ifs){
   ifs.getline(buf, 1024);
   string input(buf);
   if(input.back() == (char)13)input.pop_back();
   errMsg = "AIG";

   if(!checkId(input, "AIG literal ID") )return false;
   unsigned id = (unsigned)errInt;
   if(!checkSpace(input))return false;   

   if(!checkFanIn(input,"AIG FanIn"))return false;
   unsigned in1 = errInt;
   if(!checkSpace(input))return false; 

   if(!checkFanIn(input,"AIG FanIn"))return false;
   unsigned in2 = errInt;

   _gates[id] = new AigGate(id, lineNo+1, in1, in2);
   return checkNewLine(input);
}

bool CirMgr::checkSymbol(ifstream& ifs){
   errMsg = "Symbol";
   while(ifs.getline(buf, 1024)){
      string input(buf);
      if(input.back() == (char)13)input.pop_back();
      unsigned colTemp;

      switch(buf[0]){
         case 'c':
            ++colNo;
            return checkNewLine(input);

         case 'i':
            ++colNo;
            if(!checkInputIdx(input, "PI index"))return false;
            if(colNo >= input.size()){
               errMsg = "symbolic name";
               return parseError(MISSING_IDENTIFIER);
            }
            if(!checkSpace(input))return false;

            //if(!checkNotSpace(input, MISSING_IDENTIFIER, "symbolic Name"))return false;
            colTemp = colNo;
            if(!checkName(input))return false;
            getGate(_inputs.at(errInt))->setName(input.substr(colTemp) );

            if(!checkNewLine(input))return false;
            break;

         case 'o':
            ++colNo;
            if(!checkOutputIdx(input, "PO index"))return false;
            if(colNo >= input.size()){
               errMsg = "symbolic name";
               return parseError(MISSING_IDENTIFIER);
            }
            if(!checkSpace(input))return false;

            //if(!checkNotSpace(input, MISSING_IDENTIFIER, "symbolic Name"))return false;
            colTemp = colNo;
            if(!checkName(input))return false;
            getGate(_outputs.at(errInt))->setName(input.substr(colTemp) );

            if(!checkNewLine(input))return false;
            break;

         default:
            if(!checkNotSpace(input, ILLEGAL_SYMBOL_TYPE, string(buf)) )return false;
            errMsg = buf[0];
            return parseError(ILLEGAL_SYMBOL_TYPE);
      }
   }
   return true;
}

bool CirMgr::checkPosNum(const string& input, string err = errMsg){
   string temp;
   unsigned colTemp;
   
   if(!checkNotSpace(input, MISSING_NUM, err))return false;
   colTemp = StrGetTokTab(input, temp, colNo);
   if(!myStr2Int(temp, errInt)){
      errMsg = temp;
      return parseError(ILLEGAL_SYMBOL_TYPE);
   }
   if(errInt < 0)return parseError(ILLEGAL_NUM);

   colNo = colTemp;
   return true;
}

bool CirMgr::checkInputIdx(const string& input,string err=errMsg){
   unsigned nextCol, temp = colNo;
   if(!checkPosNum(input, err))return false;
   nextCol = colNo;
   colNo = temp;

   if((unsigned)errInt > _inputs.size()){
      errMsg = "PI index";
      return parseError(NUM_TOO_BIG);
   }
   if(!getGate(_inputs.at(errInt) )->getName().empty()){
      errMsg = "i";
      return parseError(REDEF_SYMBOLIC_NAME);
   }
   
   colNo = nextCol;
   return true;

}

bool CirMgr::checkOutputIdx(const string& input,string err=errMsg){
   unsigned nextCol, temp = colNo;
   if(!checkPosNum(input, err))return false;
   nextCol = colNo;
   colNo = temp;

   if((unsigned)errInt > _outputs.size()){
      errMsg = "PO index";
      return parseError(NUM_TOO_BIG);
   }
   if(!getGate(_outputs.at(errInt) )->getName().empty()){
      errMsg = "o";
      return parseError(REDEF_SYMBOLIC_NAME);
   }
   
   colNo = nextCol;
   return true;

}

bool CirMgr::checkFanIn(const string& input, string err = errMsg){
   unsigned nextCol, temp = colNo;
   if(!checkPosNum(input, err))return false;
   nextCol = colNo;
   colNo = temp;

   if((unsigned)errInt/2 > _M)return parseError(MAX_LIT_ID);
   
   colNo = nextCol;
   return true;
}

bool CirMgr::checkId(const string& input, string err){
   unsigned nextCol, temp = colNo;
   if(!checkFanIn(input, err))return false;
   nextCol = colNo;
   colNo = temp;

   if(errInt <= 1){
      errGate = _gates[0];
      return parseError(REDEF_CONST);
   }
   if(errInt % 2)return parseError(CANNOT_INVERTED);
   if(_gates[errInt/2] !=0 ){
      errGate = _gates[errInt/2];
      return parseError(REDEF_GATE);
   }
   
   colNo = nextCol;
   errInt /=2;
   return true;

}

bool CirMgr::checkName(const string& str){
   if(colNo >= str.size()){
      errMsg = "symbolic name";
      return parseError(MISSING_IDENTIFIER);
   }
   for(; colNo < str.size(); ++colNo){
      //if(!checkNotSpace(str,MISSING_NEWLINE))return false;
      if (int(str[colNo]) <= 31 || int(str[colNo]) == 127){
         errInt = (int)str[colNo];
         return parseError(ILLEGAL_SYMBOL_NAME);
      }
   }

   return true;
}