#pragma once
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <map>

extern int var_count;
enum SYM_TYPE{
    _CONST, _NUM, _STR, _UNK, 
};

struct Symbol{
  SYM_TYPE type;
  int sym_val;
  std::string sym_str;
};

typedef std::map<std::string, Symbol> SymTable;

extern SymTable sym_table;


enum TYPE{
  _UnaryExp, _PrimaryExp, _UnaryOp, _Number, \
  _Exp, _MulExp, _AddExp, _RelExp, _EqExp, _LAndExp, _LOrExp, \
  _MultiConstDef, _ConstDef, _MultiBlockItem, _BlockItem, _Decl, _Stmt, \
  _LVal, _ConstDecl, _VarDecl, _VarDef, _MultiVarDef, _Ident, 
};

// 所有 AST 的基类
class BaseAST {
 public:
  virtual ~BaseAST() = default;
  virtual void Dump(std::string& ret_str) const = 0;
  virtual std::string Calc(std::string& ret_str){
    return "";
  }
  virtual int Calc_val(){
    return 0;
  }
  std::vector<BaseAST *> branch;
  TYPE type;
};

// CompUnit 是 BaseAST
class CompUnitAST : public BaseAST {
 public:
  // 用智能指针管理对象
  std::unique_ptr<BaseAST> func_def;
  void Dump(std::string& ret_str) const override {
    std::cout << "COMPUNIT" << std::endl;
    // std::cout << "fun ";
    // std::cout << "CompUnitAST { ";
    ret_str = "";
    func_def->Dump(ret_str);
    // std::cout << " }";
  }
};

// FuncDef 也是 BaseAST
class FuncDefAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> func_type;
  std::string ident;
  std::unique_ptr<BaseAST> block;
  void Dump(std::string& ret_str) const override {
    std::cout << "FUNCDEF" << std::endl;
    ret_str += "fun ";
    ret_str += "@";
    ret_str += ident;
    ret_str += "(): ";
    std::cout << "fun ";
    // std::cout << "FuncDefAST { ";
    std::cout << "@" << ident << "(): ";
    func_type->Dump(ret_str);
    block->Dump(ret_str);
    // std::cout << " }";
  }
};

// FuncType 也是BaseAST
class FuncTypeAST : public BaseAST{
    public:
    std::string func_type_str; 
    void Dump(std::string& ret_str) const override {
        std::cout << "FUNCTYPE" << std::endl;
        // std::cout << "FuncTypeAST { ";
        if(func_type_str == "int"){
            ret_str += "i32";
            std::cout << "i32";
        }
        // std::cout << " }";
    }
};

// Block 也是 BaseAST
class BlockAST : public BaseAST{
    public:
    std::unique_ptr<BaseAST> multi_block_item;
    void Dump(std::string& ret_str) const override {
        std::cout << "BLOCK" << std::endl;
        // std::cout << "BlockAST { ";
        ret_str += " { \n";
        ret_str += "%entry:\n";
        std::cout << " {" << std::endl;
        std::cout << "%entry:" << std::endl;
        // if(!branch.empty())
        multi_block_item->Dump(ret_str);
        ret_str += "\n}";
        std::cout << std::endl << "}";
    }
};

class MultiBlockItemAST : public BaseAST{
    public:
    MultiBlockItemAST(){
        type = _MultiBlockItem;
    }
    std::unique_ptr<BaseAST> block_item;
    std::unique_ptr<BaseAST> multi_block_item;
    void Dump(std::string& ret_str) const override{
        std::cout << "MultiBlockItem" << std::endl;
        std::cout << branch.size() << std::endl;
        if(branch.size() == 0){
            return;
        }
        else if (branch[0]->type == _BlockItem){
            block_item->Dump(ret_str);
        }
        else if(branch[0]->type == _MultiBlockItem){
            std::cout << "MultiSize2" << std::endl;
            multi_block_item->Dump(ret_str);
            block_item->Dump(ret_str);
        }
    }
};

class BlockItemAST : public BaseAST{
    public:
    BlockItemAST(){
        type = _BlockItem;
    }
    std::unique_ptr<BaseAST> decl;
    std::unique_ptr<BaseAST> stmt;
    void Dump(std::string& ret_str) const override{
        std::cout << "BLOCKITEM" << std::endl;
        if(branch[0]->type == _Decl){
            std::cout << "DECL" << std::endl;
            decl->Dump(ret_str);
        }
        else if(branch[0]->type == _Stmt){
            stmt->Dump(ret_str);
        }
    }
};

class LValAST : public BaseAST{
    public:
    LValAST(){
        type = _LVal;
    }
    std::string ident;
    void Dump(std::string& ret_str) const override{

    }
    std::string Calc(std::string& ret_str) override{
        std::cout << "LVALCALC" << std::endl;
        Symbol tmp_ans = sym_table[ident];
        std::string ans;
        if(tmp_ans.type == _CONST){
            ans = std::to_string(tmp_ans.sym_val);
        }
        else if(tmp_ans.type == _NUM){
            std::cout << "NUM" << std::endl;
            ans = "@" + ident;
        }
        return ans;
    }
    int Calc_val() override{
        std::cout << "LVALCALC" << std::endl;
        Symbol tmp_ans = sym_table[ident];
        int ans = 0;
        if(tmp_ans.type != _STR){
            ans = tmp_ans.sym_val;
        }
        return ans;
    }
};

class StmtAST : public BaseAST{
    public:
    StmtAST(){
        type = _Stmt;
    }
    // int number;
    std::unique_ptr<BaseAST> LVal;
    std::unique_ptr<BaseAST> Exp;
    void Dump(std::string& ret_str) const override {
        std::cout << "STMT" << std::endl;
        if (branch[0]->type == _Exp){
            std::cout << "STMTEXP" << std::endl;
            std::string ans;
            ans = Exp->Calc(ret_str);
            std::cout << "AAA" << std::endl;
            if(ans[0] == '@'){ // 具名变量
                std::string tmp;
                tmp = "%" + std::to_string(var_count);
                var_count ++;

                ret_str += "    ";
                ret_str += tmp;
                ret_str += " = ";
                ret_str += "load ";
                ret_str += ans;
                ret_str += "\n";

                std::cout << "    ";
                std::cout << tmp;
                std::cout << " = ";
                std::cout << "load ";
                std::cout << ans;
                std::cout << "\n";

                ret_str += "    ret ";
                ret_str += tmp;
                std::cout << "   ret ";
                std::cout << tmp;
            }
            else { // 立即数或%0
                ret_str += "    ret ";
                ret_str += ans;
                std::cout << "   ret ";
                std::cout << ans;
            }
        }
        else if (branch[0]->type == _LVal){
            LValAST * cur_branch = (LValAST *)branch[0];
            std::string tmp1;
            std::string ans1;
            std::string ans2;
            std::string var_name = cur_branch->ident;
            int value;
            value = cur_branch->Calc_val();

            // 计算结果已经由Exp->Calc(ret_str)得出
            // 可以是数或%0
            tmp1 = Exp->Calc(ret_str);
            ans2 = "@" + var_name;

            ans1 = tmp1;

            if(tmp1[0] == '@'){
                ans1 = "%" + std::to_string(var_count);
                var_count ++;

                ret_str += "    ";
                ret_str += ans1;
                ret_str += " = ";
                ret_str += "load ";
                ret_str += tmp1;
                ret_str += "\n";

                std::cout << "    ";
                std::cout << ans1;
                std::cout << " = ";
                std::cout << "load ";
                std::cout << tmp1;
                std::cout << "\n";
            }

            Symbol symb;
            symb.type = _NUM;
            symb.sym_val = value;
            sym_table[var_name] = symb;
            
            ret_str += "    ";
            ret_str += "store ";
            ret_str += ans1;
            ret_str += ", ";
            ret_str += ans2;
            ret_str += "\n";

            std::cout << "    ";
            std::cout << "store ";
            std::cout << ans1;
            std::cout << ", ";
            std::cout << ans2;
            std::cout << "\n";
        }
    }
};

class NumberAST : public BaseAST{
    public:
    NumberAST(){
        type = _Number;
    }
    int number;
    void Dump(std::string& ret_str) const override{
    }
    std::string Calc(std::string& ret_str) override{
        std::cout << "NUMBER " << number << std::endl;
        std::string ans = std::to_string(number);
        return ans;
    }
    int Calc_val() override{
        return number;
    }
};

class ExpAST : public BaseAST{
    public:
    ExpAST(){
        type = _Exp;
    }
    std::unique_ptr<BaseAST> lor_exp;
    int val;
    void Dump(std::string& ret_str) const override {
    }
    std::string Calc(std::string& ret_str) override{
        std::cout << "EXP" << std::endl;
        std::string ans = lor_exp->Calc(ret_str);
        return ans;
    }
    int Calc_val() override{
        return lor_exp->Calc_val();
    }
};

class UnaryOpAST : public BaseAST{
    public:
    UnaryOpAST(){
        type = _UnaryOp;
    }
    char op;
    void Dump(std::string& ret_str) const override{
    }
};

class PrimaryExpAST : public BaseAST{
    public:
    PrimaryExpAST(){
        type = _PrimaryExp;
    }
    std::unique_ptr<BaseAST> Exp;
    std::unique_ptr<BaseAST> LVal;
    std::unique_ptr<BaseAST> number;
    void Dump(std::string& ret_str) const override{
    }
    std::string Calc(std::string& ret_str) override{
        std::cout << "PRIMARYEXP" << std::endl;
        std::string ans;
        ans = branch[0]->Calc(ret_str);
        return ans;
    }
    int Calc_val() override{
        return branch[0]->Calc_val();
    }
};

class UnaryExpAST : public BaseAST{
    public:
    UnaryExpAST(){
        type = _UnaryExp;
    }
    std::unique_ptr<BaseAST> primary_exp;
    std::unique_ptr<BaseAST> unary_op;
    std::unique_ptr<BaseAST> unary_exp;
    void Dump(std::string& ret_str) const override{
    }
    std::string Calc(std::string& ret_str) override{
        std::cout << "UNARYEXP" << std::endl;
        std::cout << int(branch[0]->type == _PrimaryExp) << std::endl;
        std::string ans;
        std::string tmp;
        std::string ans1;
        if(branch[0]->type == _PrimaryExp){
            PrimaryExpAST * cur_branch = (PrimaryExpAST *)branch[0];
            ans = cur_branch->Calc(ret_str);
        }
        else if(branch[0]->type == _UnaryOp){
            UnaryOpAST * cur_branch = (UnaryOpAST *)branch[0];
            if(cur_branch->op == '+'){
                ans = branch[1]->Calc(ret_str);
                return ans;
            }

            tmp = branch[1]->Calc(ret_str);
            ans1 = tmp;
            
            if(tmp[0] == '@'){
                ans1 = "%" + std::to_string(var_count);
                var_count ++;

                ret_str += "    ";
                ret_str += ans1;
                ret_str += " = ";
                ret_str += "load ";
                ret_str += tmp;
                ret_str += "\n";

                std::cout << "    ";
                std::cout << ans1;
                std::cout << " = ";
                std::cout << "load ";
                std::cout << tmp;
                std::cout << "\n";
            }

            ans = "%" + std::to_string(var_count);
            var_count ++;

            ret_str += "    ";
            ret_str += ans;
            ret_str += " = ";
            
            std::cout << "    ";
            std::cout << ans;
            std::cout << " = ";

            if(cur_branch->op == '-'){
                ret_str += "sub 0, ";
                std::cout << "sub 0, ";
            }
            else if(cur_branch->op == '!'){
                ret_str += "eq 0, ";
                std::cout << "eq 0, ";
            }
            
            ret_str += ans1;
            ret_str += "\n";

            std::cout << ans1;
            std::cout << "\n";
        }
        return ans;
    }
    int Calc_val() override{
        int ans = 0;
        if(branch[0]->type == _PrimaryExp){
            ans = branch[0]->Calc_val();
        }
        else if(branch[0]->type == _UnaryOp){
            UnaryOpAST * cur_branch = (UnaryOpAST *)branch[0];
            if(cur_branch->op == '-'){
                int tmp_ans = branch[1]->Calc_val();
                ans = 0 - tmp_ans;
            }
            else if(cur_branch->op == '+'){
                ans = branch[1]->Calc_val();
            }
            else if(cur_branch->op == '!'){
                int tmp_ans = branch[1]->Calc_val();
                ans = !tmp_ans;
            }
        }
        return ans;
    }
};

class MulExpAST : public BaseAST{
    public:
    MulExpAST(){
        type = _MulExp;
    }
    std::unique_ptr<BaseAST> unary_exp;
    std::unique_ptr<BaseAST> mul_exp;
    char op;
    void Dump(std::string& ret_str) const override{

    }
    std::string Calc(std::string& ret_str) override{
        std::cout << "MULEXP" << std::endl;
        std::string ans;
        if(branch[0]->type == _UnaryExp){
            UnaryExpAST* cur_branch = (UnaryExpAST *)branch[0];
            ans = cur_branch->Calc(ret_str);
        }
        else if(branch[0]->type == _MulExp){
            MulExpAST * cur_branch = (MulExpAST *) branch[0];
            std::string tmp1, tmp2;
            std::string ans1, ans2;

            tmp1 = cur_branch->Calc(ret_str);
            tmp2 = branch[1]->Calc(ret_str);

            ans1 = tmp1;
            ans2 = tmp2;
            
            if(tmp1[0] == '@'){
                ans1 = "%" + std::to_string(var_count);
                var_count ++;

                ret_str += "    ";
                ret_str += ans1;
                ret_str += " = ";
                ret_str += "load ";
                ret_str += tmp1;
                ret_str += "\n";

                std::cout << "    ";
                std::cout << ans1;
                std::cout << " = ";
                std::cout << "load ";
                std::cout << tmp1;
                std::cout << "\n";
            }

            if(tmp2[0] == '@'){
                ans2 = "%" + std::to_string(var_count);
                var_count ++;

                ret_str += "    ";
                ret_str += ans2;
                ret_str += " = ";
                ret_str += "load ";
                ret_str += tmp2;
                ret_str += "\n";

                std::cout << "    ";
                std::cout << ans1;
                std::cout << " = ";
                std::cout << "load ";
                std::cout << tmp2;
                std::cout << "\n";
            }

            ans = "%" + std::to_string(var_count);
            var_count ++;
            
            ret_str += "    ";
            ret_str += ans;
            ret_str += " = ";

            std::cout << "    ";
            std::cout << ans;
            std::cout << " = ";

            if(op == '*'){
                ret_str += "mul ";
                std::cout << "mul ";
            }
            else if(op == '/'){
                ret_str += "div ";
                std::cout << "div ";
            }
            else if(op == '%'){
                ret_str += "mod ";
                std::cout << "mod ";
            }

            ret_str += ans1;
            ret_str += ", ";
            ret_str += ans2;
            ret_str += "\n";

            std::cout << ans1;
            std::cout << ", ";
            std::cout << ans2;
            std::cout << std::endl;
        }
        return ans;
    }
    int Calc_val() override{
        int ans = 0;
        if(branch[0]->type == _UnaryExp){
            UnaryExpAST* cur_branch = (UnaryExpAST *)branch[0];
            ans = cur_branch->Calc_val();
        }
        else if(branch[0]->type == _MulExp){
            MulExpAST * cur_branch = (MulExpAST *) branch[0];
            if(op == '*'){
                ans = cur_branch->Calc_val() * branch[1]->Calc_val();
            }
            else if(op == '/'){
                ans = cur_branch->Calc_val() / branch[1]->Calc_val();
            }
            else if(op == '%'){
                ans = cur_branch->Calc_val() % branch[1]->Calc_val();
            }
        }
        return ans;
    }
};

class AddExpAST : public BaseAST{
    public:
    AddExpAST(){
        type = _AddExp;
    }
    std::unique_ptr<BaseAST> mul_exp;
    std::unique_ptr<BaseAST> add_exp;
    char op;
    void Dump(std::string& ret_str) const override{
    }
    std::string Calc(std::string& ret_str) override{
        std::cout << "ADDEXP" << bool(op == '-');
        std::string ans;
        std::string tmp1, tmp2;
        std::string ans1, ans2;
        // std::cout << " " << bool(branch[0]->type == _AddExp) << std::endl;
        if(branch[0]->type == _MulExp){
            ans = branch[0]->Calc(ret_str);
        }
        else if(branch[0]->type == _AddExp){
            tmp1 = branch[0]->Calc(ret_str);
            tmp2 = branch[1]->Calc(ret_str);

            ans1 = tmp1;
            ans2 = tmp2;
            
            if(tmp1[0] == '@'){
                ans1 = "%" + std::to_string(var_count);
                var_count ++;

                ret_str += "    ";
                ret_str += ans1;
                ret_str += " = ";
                ret_str += "load ";
                ret_str += tmp1;
                ret_str += "\n";

                std::cout << "    ";
                std::cout << ans1;
                std::cout << " = ";
                std::cout << "load ";
                std::cout << tmp1;
                std::cout << "\n";
            }

            if(tmp2[0] == '@'){
                ans2 = "%" + std::to_string(var_count);
                var_count ++;

                ret_str += "    ";
                ret_str += ans2;
                ret_str += " = ";
                ret_str += "load ";
                ret_str += tmp2;
                ret_str += "\n";

                std::cout << "    ";
                std::cout << ans2;
                std::cout << " = ";
                std::cout << "load ";
                std::cout << tmp2;
                std::cout << "\n";
            }

            ans = "%" + std::to_string(var_count);
            var_count ++;

            ret_str += "    ";
            ret_str += ans;
            ret_str += " = ";

            std::cout << "    ";
            std::cout << ans;
            std::cout << " = ";

            if(op == '+'){
                ret_str += "add ";
                std::cout << "add ";
            }
            else if(op == '-'){
                ret_str += "sub ";
                std::cout << "sub ";
            }

            ret_str += ans1;
            ret_str += ", ";
            ret_str += ans2;
            ret_str += "\n";

            std::cout << ans1;
            std::cout << ", ";
            std::cout << ans2;
            std::cout << std::endl;
        }
        return ans;
    }
    int Calc_val() override{
        int ans = 0;
        if(branch[0]->type == _MulExp){
            MulExpAST* cur_branch = (MulExpAST *)branch[0];
            ans = cur_branch->Calc_val();
        }
        else if(branch[0]->type == _AddExp){
            AddExpAST * cur_branch = (AddExpAST *) branch[0];
            if(op == '+'){
                ans = cur_branch->Calc_val() + branch[1]->Calc_val();
            }
            else if(op == '-'){
                ans = cur_branch->Calc_val() - branch[1]->Calc_val();
            }
        }
        return ans;
    }
};

class RelExpAST : public BaseAST{
    public:
    RelExpAST(){
        type = _RelExp;
    }
    std::unique_ptr<BaseAST> add_exp;
    std::unique_ptr<BaseAST> rel_exp;
    std::string op;
    void Dump(std::string& ret_str) const override{

    }
    std::string Calc(std::string& ret_str) override{
        std::cout << "RELEXP" << std::endl;
        std::string ans;
        std::string tmp1;
        std::string tmp2;
        if(branch[0]->type == _AddExp){
            ans = branch[0]->Calc(ret_str);
        }
        else if(branch[0]->type == _RelExp){
            //std::cout << "CalcTest " << branch[0]->Calc_val() << std::endl;
            std::string ans1;
            std::string ans2;

            tmp1 = branch[0]->Calc(ret_str);
            tmp2 = branch[1]->Calc(ret_str);

            ans1 = tmp1;
            ans2 = tmp2;
            
            if(tmp1[0] == '@'){
                ans1 = "%" + std::to_string(var_count);
                var_count ++;

                ret_str += "    ";
                ret_str += ans1;
                ret_str += " = ";
                ret_str += "load ";
                ret_str += tmp1;
                ret_str += "\n";

                std::cout << "    ";
                std::cout << ans1;
                std::cout << " = ";
                std::cout << "load ";
                std::cout << tmp1;
                std::cout << "\n";
            }

            if(tmp2[0] == '@'){
                ans2 = "%" + std::to_string(var_count);
                var_count ++;

                ret_str += "    ";
                ret_str += ans2;
                ret_str += " = ";
                ret_str += "load ";
                ret_str += tmp2;
                ret_str += "\n";

                std::cout << "    ";
                std::cout << ans2;
                std::cout << " = ";
                std::cout << "load ";
                std::cout << tmp2;
                std::cout << "\n";
            }

            ans = "%" + std::to_string(var_count);
            var_count ++;
            ret_str += "    ";
            ret_str += ans;
            ret_str += " = ";

            std::cout << "    ";
            std::cout << ans;
            std::cout << " = ";

            if(op == "<"){
                ret_str += "lt ";
                std::cout << "lt ";
            }
            else if(op == ">"){
                ret_str += "gt  ";
                std::cout << "gt  ";
            }
            else if(op == "<="){
                ret_str += "le  ";
                std::cout << "le  ";
            }
            else if(op == ">="){
                ret_str += "ge  ";
                std::cout << "ge  ";
            }
            ret_str += ans1;
            ret_str += ", ";
            ret_str += ans2;
            ret_str += "\n";

            std::cout << ans1;
            std::cout << ", ";
            std::cout << ans2;
            std::cout << "\n";
            
        }
        return ans;
    }
    int Calc_val() override{
        int ans = 0;
        if(branch[0]->type == _AddExp){
            AddExpAST* cur_branch = (AddExpAST *)branch[0];
            ans = cur_branch->Calc_val();
        }
        else if(branch[0]->type == _RelExp){
            RelExpAST * cur_branch = (RelExpAST *) branch[0];
            if(op == ">="){
                ans = cur_branch->Calc_val() >= branch[1]->Calc_val();
            }
            else if(op == "<="){
                ans = cur_branch->Calc_val() <= branch[1]->Calc_val();
            }
            else if(op == ">"){
                ans = cur_branch->Calc_val() > branch[1]->Calc_val();
            }
            else if(op == "<"){
                ans = cur_branch->Calc_val() < branch[1]->Calc_val();
            }
        }
        return ans;
    }
};

class EqExpAST : public BaseAST{
    public:
    EqExpAST(){
        type = _EqExp;
    }
    std::unique_ptr<BaseAST> rel_exp;
    std::unique_ptr<BaseAST> eq_exp;
    std::string op;
    void Dump(std::string& ret_str) const override{

    }
    std::string Calc(std::string& ret_str) override{
        std::cout << "EQEXP" << std::endl;
        std::string ans;
        if(branch[0]->type == _RelExp){
            ans = branch[0]->Calc(ret_str);
        }
        else if(branch[0]->type == _EqExp){
            std::string tmp1;
            std::string tmp2;
            std::string ans1;
            std::string ans2;

            tmp1 = branch[0]->Calc(ret_str);
            tmp2 = branch[1]->Calc(ret_str);

            ans1 = tmp1;
            ans2 = tmp2;
            
            if(tmp1[0] == '@'){
                ans1 = "%" + std::to_string(var_count);
                var_count ++;

                ret_str += "    ";
                ret_str += ans1;
                ret_str += " = ";
                ret_str += "load ";
                ret_str += tmp1;
                ret_str += "\n";

                std::cout << "    ";
                std::cout << ans1;
                std::cout << " = ";
                std::cout << "load ";
                std::cout << tmp1;
                std::cout << "\n";
            }

            if(tmp2[0] == '@'){
                ans2 = "%" + std::to_string(var_count);
                var_count ++;

                ret_str += "    ";
                ret_str += ans2;
                ret_str += " = ";
                ret_str += "load ";
                ret_str += tmp2;
                ret_str += "\n";

                std::cout << "    ";
                std::cout << ans2;
                std::cout << " = ";
                std::cout << "load ";
                std::cout << tmp2;
                std::cout << "\n";
            }

            ans = "%" + std::to_string(var_count);
            var_count ++;
            ret_str += "    ";
            ret_str += ans;
            ret_str += " = ";

            std::cout << "    ";
            std::cout << ans;
            std::cout << " = ";

            if(op == "=="){
                ret_str += "eq  ";
                std::cout << "eq  ";
            }
            else if(op == "!="){
                ret_str += "ne  ";
                std::cout << "ne  ";
            }

            ret_str += ans1;
            ret_str += ", ";
            ret_str += ans2;
            ret_str += "\n";

            std::cout << ans1;
            std::cout << ", ";
            std::cout << ans2;
            std::cout << "\n";
        }
        return ans;
    }
    int Calc_val() override{
        int ans = 0;
        if(branch[0]->type == _RelExp){
            RelExpAST* cur_branch = (RelExpAST *)branch[0];
            ans = cur_branch->Calc_val();
        }
        else if(branch[0]->type == _EqExp){
            EqExpAST * cur_branch = (EqExpAST *) branch[0];
            if(op == "=="){
                ans = cur_branch->Calc_val() == branch[1]->Calc_val();
            }
            else if(op == "!="){
                ans = cur_branch->Calc_val() != branch[1]->Calc_val();
            }
        }
        return ans;
    }
};

// (a != 0) && (b != 0)
class LAndExpAST : public BaseAST{
    public:
    LAndExpAST(){
        type = _LAndExp;
    }
    std::unique_ptr<BaseAST> eq_exp;
    std::unique_ptr<BaseAST> land_exp;
    std::string op;
    void Dump(std::string& ret_str) const override{

    }
    std::string Calc(std::string& ret_str) override{
        std::cout << "LANDEXP" << std::endl;
        std::string ans;
        if(branch[0]->type == _EqExp){
            ans = branch[0]->Calc(ret_str);
        }
        else if(branch[0]->type == _LAndExp){
            std::string tmp1;
            std::string tmp2;
            std::string ans1;
            std::string ans2;

            tmp1 = branch[0]->Calc(ret_str);
            tmp2 = branch[1]->Calc(ret_str);

            ans1 = tmp1;
            ans2 = tmp2;
            
            if(tmp1[0] == '@'){
                ans1 = "%" + std::to_string(var_count);
                var_count ++;

                ret_str += "    ";
                ret_str += ans1;
                ret_str += " = ";
                ret_str += "load ";
                ret_str += tmp1;
                ret_str += "\n";

                std::cout << "    ";
                std::cout << ans1;
                std::cout << " = ";
                std::cout << "load ";
                std::cout << tmp1;
                std::cout << "\n";
            }

            if(tmp2[0] == '@'){
                ans2 = "%" + std::to_string(var_count);
                var_count ++;

                ret_str += "    ";
                ret_str += ans2;
                ret_str += " = ";
                ret_str += "load ";
                ret_str += tmp2;
                ret_str += "\n";

                std::cout << "    ";
                std::cout << ans2;
                std::cout << " = ";
                std::cout << "load ";
                std::cout << tmp2;
                std::cout << "\n";
            }

            std::string neqa;
            std::string neqb;

            neqa = "%" + std::to_string(var_count);
            var_count ++;
            ret_str += "    ";
            ret_str += neqa;
            ret_str += " = ";
            ret_str += "ne  0, ";
            ret_str += ans1;
            ret_str += "\n";

            std::cout << "    ";
            std::cout << neqa;
            std::cout << " = ";
            std::cout << "ne  0, ";
            std::cout << ans1;
            std::cout << "\n";

            neqb = "%" + std::to_string(var_count);
            var_count ++;
            ret_str += "    ";
            ret_str += neqb;
            ret_str += " = ";
            ret_str += "ne  0, ";
            ret_str += ans2;
            ret_str += "\n";

            std::cout << "    ";
            std::cout << neqb;
            std::cout << " = ";
            std::cout << "ne  0, ";
            std::cout << ans2;
            std::cout << "\n";

            ans = "%" + std::to_string(var_count);
            var_count ++;
            ret_str += "    ";
            ret_str += ans;
            ret_str += " = ";
            ret_str += "and  ";
            ret_str += neqa;
            ret_str += ", ";
            ret_str += neqb;
            ret_str += "\n";

            std::cout << "    ";
            std::cout << ans;
            std::cout << " = ";
            std::cout << "and  ";
            std::cout << neqa;
            std::cout << ", ";
            std::cout << neqb;
            std::cout << "\n";

        }
        return ans;
    }
    int Calc_val() override{
        int ans = 0;
        if(branch[0]->type == _EqExp){
            EqExpAST* cur_branch = (EqExpAST *)branch[0];
            ans = cur_branch->Calc_val();
        }
        else if(branch[0]->type == _LAndExp){
            LAndExpAST * cur_branch = (LAndExpAST *) branch[0];
            ans = cur_branch->Calc_val() && branch[1]->Calc_val();
        }
        return ans;
    }
};

// !(a==0)|!(b==0)
class LOrExpAST : public BaseAST{
    public:
    LOrExpAST(){
        type = _LOrExp;
    }
    std::unique_ptr<BaseAST> land_exp;
    std::unique_ptr<BaseAST> lor_exp;
    std::string op;
    void Dump(std::string& ret_str) const override{

    }
    std::string Calc(std::string& ret_str) override{
        std::string ans;
        std::cout << "LOREXP" << std::endl;
        if(branch[0]->type == _LAndExp){
            ans = branch[0]->Calc(ret_str);
        }
        else if(branch[0]->type == _LOrExp){
            std::string tmp1;
            std::string tmp2;
            std::string ans1;
            std::string ans2;

            tmp1 = branch[0]->Calc(ret_str);
            tmp2 = branch[1]->Calc(ret_str);

            ans1 = tmp1;
            ans2 = tmp2;
            
            if(tmp1[0] == '@'){
                ans1 = "%" + std::to_string(var_count);
                var_count ++;

                ret_str += "    ";
                ret_str += ans1;
                ret_str += " = ";
                ret_str += "load ";
                ret_str += tmp1;
                ret_str += "\n";

                std::cout << "    ";
                std::cout << ans1;
                std::cout << " = ";
                std::cout << "load ";
                std::cout << tmp1;
                std::cout << "\n";
            }

            if(tmp2[0] == '@'){
                ans2 = "%" + std::to_string(var_count);
                var_count ++;

                ret_str += "    ";
                ret_str += ans2;
                ret_str += " = ";
                ret_str += "load ";
                ret_str += tmp2;
                ret_str += "\n";

                std::cout << "    ";
                std::cout << ans2;
                std::cout << " = ";
                std::cout << "load ";
                std::cout << tmp2;
                std::cout << "\n";
            }

            std::string eqa;
            std::string eqb;
            std::string na;
            std::string nb;

            eqa = "%" + std::to_string(var_count);
            var_count ++;
            ret_str += "    ";
            ret_str += eqa;
            ret_str += " = ";
            ret_str += "eq  0, ";
            ret_str += ans1;
            ret_str += "\n";

            std::cout << "    ";
            std::cout << eqa;
            std::cout << " = ";
            std::cout << "eq  0, ";
            std::cout << ans1;
            std::cout << "\n";

            na = "%" + std::to_string(var_count);
            var_count ++;
            ret_str += "    ";
            ret_str += na;
            ret_str += " = ";
            ret_str += "eq  0, ";
            ret_str += eqa;
            ret_str += "\n";

            std::cout << "    ";
            std::cout << na;
            std::cout << " = ";
            std::cout << "eq  0, ";
            std::cout << eqa;
            std::cout << "\n";

            eqb = "%" + std::to_string(var_count);
            var_count ++;
            ret_str += "    ";
            ret_str += eqb;
            ret_str += " = ";
            ret_str += "eq  0, ";
            ret_str += ans2;
            ret_str += "\n";

            std::cout << "    ";
            std::cout << eqb;
            std::cout << " = ";
            std::cout << "eq  0, ";
            std::cout << ans2;
            std::cout << "\n";

            nb = "%" + std::to_string(var_count);
            var_count ++;
            ret_str += "    ";
            ret_str += nb;
            ret_str += " = ";
            ret_str += "eq  0, ";
            ret_str += eqb;
            ret_str += "\n";

            std::cout << "    ";
            std::cout << nb;
            std::cout << " = ";
            std::cout << "eq  0, ";
            std::cout << eqb;
            std::cout << "\n";

            ans = "%" + std::to_string(var_count);
            var_count ++;
            ret_str += "    ";
            ret_str += ans;
            ret_str += " = ";
            ret_str += "or  ";
            ret_str += na;
            ret_str += ", ";
            ret_str += nb;
            ret_str += "\n";

            std::cout << "    ";
            std::cout << ans;
            std::cout << " = ";
            std::cout << "or  ";
            std::cout << na;
            std::cout << ", ";
            std::cout << nb;
            std::cout << "\n";

        }
        return ans;
    }
    int Calc_val() override{
        int ans = 0;
        if(branch[0]->type == _LAndExp){
            LAndExpAST* cur_branch = (LAndExpAST *)branch[0];
            ans = cur_branch->Calc_val();
        }
        else if(branch[0]->type == _LOrExp){
            LOrExpAST * cur_branch = (LOrExpAST *) branch[0];
            ans = cur_branch->Calc_val() || branch[1]->Calc_val();
        }
        return ans;
    }
};

class DeclAST : public BaseAST{
    public:
    DeclAST(){
        type = _Decl;
    }
    std::unique_ptr<BaseAST> const_decl;
    std::unique_ptr<BaseAST> var_decl;
    void Dump(std::string& ret_str) const override{
        if(branch[0]->type == _VarDecl){
            var_decl->Dump(ret_str);
        }
        else if(branch[0]->type == _ConstDecl){
            const_decl->Dump(ret_str);
        }
    }
};

class ConstDeclAST : public BaseAST{
    public:
    ConstDeclAST(){
        type = _ConstDecl;
    }
    std::unique_ptr<BaseAST> btype;
    std::unique_ptr<BaseAST> multi_const_def;
    void Dump(std::string& ret_str) const override{
        btype->Dump(ret_str);
        multi_const_def->Dump(ret_str);
    }
};

class VarDeclAST : public BaseAST {
    public:
    VarDeclAST(){
        type = _VarDecl;
    }
    std::unique_ptr<BaseAST> btype;
    std::unique_ptr<BaseAST> multi_var_def;
    void Dump(std::string& ret_str) const override {
        btype->Dump(ret_str);
        multi_var_def->Dump(ret_str);
    }
};

class BTypeAST : public BaseAST{
    public:
    std::string btype_name;
    void Dump(std::string& ret_str) const override{
        return;
    }
};

class MultiConstDefAST : public BaseAST{
    public:
    MultiConstDefAST(){
        type = _MultiConstDef;
    }
    std::unique_ptr<BaseAST> const_def;
    std::unique_ptr<BaseAST> multi_const_def;
    void Dump(std::string& ret_str) const override{
        std::cout << "MULTICONSTDEF" << std::endl;
        std::cout << branch.size() << std::endl;
        if(branch[0]->type == _ConstDef){
            const_def->Dump(ret_str);
        }
        else if(branch[0]->type == _MultiConstDef){
            multi_const_def->Dump(ret_str);
            const_def->Dump(ret_str);
        }
    }
};

class MultiVarDefAST : public BaseAST{
    public:
    MultiVarDefAST(){
        type = _MultiVarDef;
    }
    std::unique_ptr<BaseAST> var_def;
    std::unique_ptr<BaseAST> multi_var_def;
    void Dump(std::string& ret_str) const override{
        std::cout << "MULTIVARDEF" << std::endl;
        // std::cout << branch.size() << std::endl;
        if(branch[0]->type == _VarDef){
            var_def->Dump(ret_str);
        }
        else if(branch[0]->type == _MultiVarDef){
            multi_var_def->Dump(ret_str);
            var_def->Dump(ret_str);
        }
    }
};

class ConstDefAST : public BaseAST{
    public:
    ConstDefAST(){
        type = _ConstDef;
    }
    std::string ident;
    std::unique_ptr<BaseAST> const_init_val;
    void Dump(std::string& ret_str) const override{
        std::cout << "CONSTDEF" << std::endl;
        int ans = const_init_val->Calc_val();
        Symbol symb;
        symb.type = _CONST;
        symb.sym_val = ans;
        sym_table[ident] = symb;
    }
};

class IdentAST : public BaseAST {
    public:
    IdentAST(){
        type = _Ident;
    }
    std::string ident;
    void Dump(std::string& ret_str) const override {

    }
};

class VarDefAST : public BaseAST{
    public:
    VarDefAST(){
        type = _VarDef;
    }
    std::string ident;
    std::unique_ptr<BaseAST> init_val;
    void Dump(std::string& ret_str) const override{
        std::cout << "VARDEF" << std::endl;
        if(branch.size() == 1){
            Symbol symb;
            symb.type = _UNK;
            symb.sym_val = 0;
            sym_table[ident] = symb;

            std::string ans;
            ans = "@" + ident;

            ret_str += "    ";
            ret_str += ans;
            ret_str += " = alloc i32";
            ret_str += "\n";

            std::cout << "    ";
            std::cout << ans;
            std::cout << " = alloc i32";
            std::cout << "\n";
        }
        else if(branch.size() == 2){
            std::string ans;
            std::string val;
            int value;
            value = init_val->Calc_val();

            Symbol symb;
            symb.type = _NUM;
            symb.sym_val = value;
            sym_table[ident] = symb;
            // init_val->Dump(ret_str);

            ans = "@" + ident;
            val = init_val->Calc(ret_str); // 可能是立即数，也可能是%0

            ret_str += "    ";
            ret_str += ans;
            ret_str += " = alloc i32";
            ret_str += "\n";
            ret_str += "    store ";
            ret_str += val;
            ret_str += ", ";
            ret_str += ans;
            ret_str += "\n";

            std::cout << "    ";
            std::cout << ans;
            std::cout << " = alloc i32";
            std::cout << "\n";
            std::cout << "  store ";
            std::cout << val;
            std::cout << ", ";
            std::cout << ans;
            std::cout << "\n";
        }
    }
};

class ConstInitValAST : public BaseAST{
    public:
    std::unique_ptr<BaseAST> const_exp;
    void Dump(std::string& ret_str) const override{

    }
    int Calc_val() override{
        return const_exp->Calc_val();
    }
};

class InitValAST : public BaseAST{
    public:
    std::unique_ptr<BaseAST> Exp;
    void Dump(std::string& ret_str) const override{

    }
    std::string Calc(std::string& ret_str) override{
        std::string ans;
        ans = Exp->Calc(ret_str);
        return ans;
    }
    int Calc_val() override{
        return Exp->Calc_val();
    }
};


class ConstExpAST : public BaseAST{
    public:
    std::unique_ptr<BaseAST> Exp;
    void Dump(std::string& ret_str) const override{

    }
    int Calc_val() override{
        return Exp->Calc_val();
    }
};


