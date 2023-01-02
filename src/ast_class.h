#include <iostream>
#include <memory>
#include <string>

// 所有 AST 的基类
class BaseAST {
 public:
  virtual ~BaseAST() = default;
  virtual void Dump(std::string& ret_str) const = 0;
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
    int number;
    void Dump(std::string& ret_str) const override {
        // std::cout << "StmtAST { ";
        ret_str += "    ret ";
        ret_str += std::to_string(number).c_str();
        std::cout << "   ret ";
        std::cout << number;
        // std::cout << " }";
    }
};