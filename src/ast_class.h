#include <iostream>
#include <memory>
#include <string>
#include <vector>


extern int var_count;
enum TYPE{
  _UnaryExp, _PrimaryExp, _UnaryOp, _Number, \
  _Exp, _MulExp, _AddExp, _RelExp, _EqExp, _LAndExp, _LOrExp, 
};

// 所有 AST 的基类
class BaseAST {
 public:
  virtual ~BaseAST() = default;
  virtual void Dump(std::string& ret_str) const = 0;
  virtual std::string Calc(std::string& ret_str){
    return "";
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
    std::unique_ptr<BaseAST> stmt;
    void Dump(std::string& ret_str) const override {
        // std::cout << "BlockAST { ";
        ret_str += " { \n";
        ret_str += "%entry:\n";
        std::cout << " {" << std::endl;
        std::cout << "%entry:" << std::endl;
        stmt->Dump(ret_str);
        ret_str += "\n}";
        std::cout << std::endl << "}";
    }
};

class StmtAST : public BaseAST{
    public:
    // int number;
    std::unique_ptr<BaseAST> Exp;
    void Dump(std::string& ret_str) const override {
        std::string ans;
        ans = Exp->Calc(ret_str);
        // std::cout << "StmtAST { ";
        ret_str += "    ret ";
        ret_str += ans;
        // ret_str += std::to_string(number).c_str();
        std::cout << "   ret ";
        std::cout << ans;
        // std::cout << number;
        // std::cout << " }";
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
};

class ExpAST : public BaseAST{
    public:
    ExpAST(){
        type = _Exp;
    }
    std::unique_ptr<BaseAST> lor_exp;
    void Dump(std::string& ret_str) const override {
    }
    std::string Calc(std::string& ret_str) override{
        std::cout << "EXP" << std::endl;
        std::string ans = lor_exp->Calc(ret_str);
        return ans;
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
    std::unique_ptr<BaseAST> number;
    void Dump(std::string& ret_str) const override{
    }
    std::string Calc(std::string& ret_str) override{
        std::cout << "PRIMARYEXP" << std::endl;
        std::string ans;
        ans = branch[0]->Calc(ret_str);
        return ans;
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
        std::string ans;
        std::string tmp;
        if(branch[0]->type == _PrimaryExp){
            PrimaryExpAST * cur_branch = (PrimaryExpAST *)branch[0];
            ans = cur_branch->Calc(ret_str);
        }
        else if(branch[0]->type == _UnaryOp){
            UnaryOpAST * cur_branch = (UnaryOpAST *)branch[0];
            if(cur_branch->op == '-'){
                tmp = branch[1]->Calc(ret_str);
                ans = "%" + std::to_string(var_count);
                var_count ++;

                // std::cout << "WRONG BRANCH!!!!" << std::endl;

                std::cout << "    ";
                std::cout << ans;
                std::cout << " = ";
                std::cout << "sub 0, ";
                std::cout << tmp;
                std::cout << "\n";

                ret_str += "    ";
                ret_str += ans;
                ret_str += " = ";
                ret_str += "sub 0, ";
                ret_str += tmp;
                ret_str += "\n";
            }
            else if(cur_branch->op == '+'){
                ans = branch[1]->Calc(ret_str);
            }
            else if(cur_branch->op == '!'){
                tmp = branch[1]->Calc(ret_str);
                ans = "%" + std::to_string(var_count);
                var_count ++;

                std::cout << "    ";
                std::cout << ans;
                std::cout << " = ";
                std::cout << "eq 0, ";
                std::cout << tmp;
                std::cout << "\n";
                
                ret_str += "    ";
                ret_str += ans;
                ret_str += " = ";
                ret_str += "eq 0, ";
                ret_str += tmp;
                ret_str += "\n";
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
            if(op == '*'){
                tmp1 = cur_branch->Calc(ret_str);
                tmp2 = branch[1]->Calc(ret_str);
                ans = "%" + std::to_string(var_count);
                var_count ++;
                
                ret_str += "    ";
                ret_str += ans;
                ret_str += " = ";
                ret_str += "mul ";
                ret_str += tmp1;
                ret_str += ", ";
                ret_str += tmp2;

                std::cout << "    ";
                std::cout << ans;
                std::cout << " = ";
                std::cout << "mul ";
                std::cout << tmp1;
                std::cout << ", ";
                std::cout << tmp2;
                std::cout << std::endl;

            }
            else if(op == '/'){
                tmp1 = cur_branch->Calc(ret_str);
                tmp2 = branch[1]->Calc(ret_str);
                ans = "%" + std::to_string(var_count);
                var_count ++;
                
                ret_str += "    ";
                ret_str += ans;
                ret_str += " = ";
                ret_str += "div ";
                ret_str += tmp1;
                ret_str += ", ";
                ret_str += tmp2;

                std::cout << "    ";
                std::cout << ans;
                std::cout << " = ";
                std::cout << "div ";
                std::cout << tmp1;
                std::cout << ", ";
                std::cout << tmp2;
                std::cout << std::endl;

            }
            else if(op == '%'){
                tmp1 = cur_branch->Calc(ret_str);
                tmp2 = branch[1]->Calc(ret_str);
                ans = "%" + std::to_string(var_count);
                var_count ++;
                
                ret_str += "    ";
                ret_str += ans;
                ret_str += " = ";
                ret_str += "mod ";
                ret_str += tmp1;
                ret_str += ", ";
                ret_str += tmp2;
                ret_str += "\n";

                std::cout << "    ";
                std::cout << ans;
                std::cout << " = ";
                std::cout << "mod ";
                std::cout << tmp1;
                std::cout << ", ";
                std::cout << tmp2;
                std::cout << std::endl;
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
        // std::cout << " " << bool(branch[0]->type == _AddExp) << std::endl;
        if(branch[0]->type == _MulExp){
            ans = branch[0]->Calc(ret_str);
        }
        else if(branch[0]->type == _AddExp){
            if(op == '+'){
                tmp1 = branch[0]->Calc(ret_str);
                tmp2 = branch[1]->Calc(ret_str);
                ans = "%" + std::to_string(var_count);
                var_count ++;
                
                ret_str += "    ";
                ret_str += ans;
                ret_str += " = ";
                ret_str += "add ";
                ret_str += tmp1;
                ret_str += ", ";
                ret_str += tmp2;
                ret_str += "\n";

                std::cout << "    ";
                std::cout << ans;
                std::cout << " = ";
                std::cout << "add ";
                std::cout << tmp1;
                std::cout << ", ";
                std::cout << tmp2;
                std::cout << std::endl;
            }
            else if(op == '-'){
                tmp1 = branch[0]->Calc(ret_str);
                tmp2 = branch[1]->Calc(ret_str);
                ans = "%" + std::to_string(var_count);
                var_count ++;
                
                // std::cout << "RIGHT BRANCH" << std::endl;
                
                ret_str += "    ";
                ret_str += ans;
                ret_str += " = ";
                ret_str += "sub ";
                ret_str += tmp1;
                ret_str += ", ";
                ret_str += tmp2;
                ret_str += "\n";

                std::cout << "    ";
                std::cout << ans;
                std::cout << " = ";
                std::cout << "sub ";
                std::cout << tmp1;
                std::cout << ", ";
                std::cout << tmp2;
                std::cout << std::endl;
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
            tmp1 = branch[0]->Calc(ret_str);
            tmp2 = branch[1]->Calc(ret_str);
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
            ret_str += tmp1;
            ret_str += ", ";
            ret_str += tmp2;
            ret_str += "\n";

            std::cout << tmp1;
            std::cout << ", ";
            std::cout << tmp2;
            std::cout << "\n";
            
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
            tmp1 = branch[0]->Calc(ret_str);
            tmp2 = branch[1]->Calc(ret_str);
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

            ret_str += tmp1;
            ret_str += ", ";
            ret_str += tmp2;
            ret_str += "\n";

            std::cout << tmp1;
            std::cout << ", ";
            std::cout << tmp2;
            std::cout << "\n";
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
            tmp1 = branch[0]->Calc(ret_str);
            tmp2 = branch[1]->Calc(ret_str);

            std::string neqa;
            std::string neqb;

            neqa = "%" + std::to_string(var_count);
            var_count ++;
            ret_str += "    ";
            ret_str += neqa;
            ret_str += " = ";
            ret_str += "ne  0, ";
            ret_str += tmp1;
            ret_str += "\n";

            std::cout << "    ";
            std::cout << neqa;
            std::cout << " = ";
            std::cout << "ne  0, ";
            std::cout << tmp1;
            std::cout << "\n";

            neqb = "%" + std::to_string(var_count);
            var_count ++;
            ret_str += "    ";
            ret_str += neqb;
            ret_str += " = ";
            ret_str += "ne  0, ";
            ret_str += tmp2;
            ret_str += "\n";

            std::cout << "    ";
            std::cout << neqb;
            std::cout << " = ";
            std::cout << "ne  0, ";
            std::cout << tmp2;
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
            tmp1 = branch[0]->Calc(ret_str);
            tmp2 = branch[1]->Calc(ret_str);

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
            ret_str += tmp1;
            ret_str += "\n";

            std::cout << "    ";
            std::cout << eqa;
            std::cout << " = ";
            std::cout << "eq  0, ";
            std::cout << tmp1;
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
            ret_str += tmp2;
            ret_str += "\n";

            std::cout << "    ";
            std::cout << eqb;
            std::cout << " = ";
            std::cout << "eq  0, ";
            std::cout << tmp2;
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
};



