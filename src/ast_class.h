#include <iostream>
#include <memory>
#include <string>
#include <vector>


extern int var_count;
enum TYPE{
  _UnaryExp, _PrimaryExp, _UnaryOp, _Number, _Exp, 
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
  int type;
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
        std::string ans = std::to_string(number);
        return ans;
    }
};

class ExpAST : public BaseAST{
    public:
    ExpAST(){
        type = _Exp;
    }
    std::unique_ptr<BaseAST> unary_exp;
    void Dump(std::string& ret_str) const override {
    }
    std::string Calc(std::string& ret_str) override{
        std::string ans = unary_exp->Calc(ret_str);
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