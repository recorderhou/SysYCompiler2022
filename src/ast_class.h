#pragma once
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <map>

extern int var_count;

// 参数param，全局变量，以及初始化/赋值时使用参数和全局变量的
enum SYM_TYPE{
    _CONST, _NUM, _ARRAY, _ARRAY_UNCALC, _GLOBAL_ARRAY, _CONST_ARRAY, _CONST_GLOBAL_ARRAY, \
    _UNK, _PARAM, _NUM_GLOBAL, _CONST_GLOBAL, _NUM_UNCALC, _CONST_UNCALC, 
};

enum PARAM_TYPE {
    _PARAM_ARRAY, _PARAM_NUM, 
};

struct RetVal {
    bool calcable;
    int value;
    std::vector<int> array_value;
    std::string sym_str; 
};


struct Symbol{
  SYM_TYPE type;
  int sym_val;
  std::vector<int> array_val; // 数组的值
  std::string sym_str;
};

typedef std::map<std::string, Symbol> SymTable;
typedef std::map<std::string, std::string> FuncTable;
typedef std::map<std::string, Symbol> GlobalVarTable;
typedef std::map<std::string, PARAM_TYPE> CurParamType;
typedef std::map<std::string, CurParamType> FuncParamType; 

struct SymbolTable{
    SymTable sym_table;
    std::string func_name;
    int table_index;
    bool returned; // 是否已经出现了
    bool blocking; 
    // blocking = true的block, 它内部的return不会影响父节点和兄弟节点其他代码的生成
    // 这样的block，在它之前parent是否return对它是有影响的
    // blocking = false的block, 它内部的return会影响父节点和兄弟节点
    // 并且一直影响到最近的blocking的祖先节点的家族
    std::vector<SymbolTable *> child;
    SymbolTable *parent;
};

typedef std::vector<SymbolTable> SymTableList; // 每个函数一个SymtableList
typedef std::map<std::string, SymTableList> ProgramSymTableList; // 程序中可能有多个函数

extern SymTable sym_table;
extern FuncTable func_table;
extern GlobalVarTable global_var_table;
extern SymTableList sym_table_list;
extern ProgramSymTableList program_sym_table;
extern FuncParamType program_param_list;
extern CurParamType cur_param_list;

extern int block_count;
extern SymbolTable* cur_table;
extern SymTableList* cur_sym_table_list;
extern int end_count;
extern int if_count;
extern int else_count;
extern int tmp_result_count;
extern int while_count;
extern int ptr_count;
extern int cur_while;
extern int cur_while_end;
extern int prev_cur_while;
extern bool break_continue;
extern std::string cur_func_name;


enum TYPE{
  _UnaryExp, _PrimaryExp, _UnaryOp, _Number, \
  _Exp, _MulExp, _AddExp, _RelExp, _EqExp, _LAndExp, _LOrExp, \
  _MultiConstDef, _ConstDef, _MultiBlockItem, _BlockItem, _Decl, _Stmt, \
  _LVal, _ConstDecl, _VarDecl, _VarDef, _MultiVarDef, _Ident, _Return, _Block, \
  _OpenStmt, _MatchedStmt, _Continue, _Break, _CompUnit, _FuncDef, \
  _MultiFuncFParam, _FuncFParam, _MultiFuncRParam, _FuncRParam, \
  _MultiConstArrayInitVal, _MultiArrayInitVal, _ConstExp, _InitVal, _ConstInitVal\
};

// 所有 AST 的基类
class BaseAST {
 public:
  virtual ~BaseAST() = default;
  virtual void Dump(std::string& ret_str) const = 0;
  virtual std::string Calc(std::string& ret_str){
    return "";
  }
  virtual RetVal Calc_val(){
    RetVal ans;
    return ans;
  }
  virtual std::vector<std::string> Param(std::string& ret_str){
    std::vector<std::string> aaa;
    return aaa;
  }
  virtual std::vector<std::string> ArrayInit(std::string& ret_str){
    std::vector<std::string> aaa;
    return aaa;
  }
  std::vector<BaseAST *> branch;
  TYPE type;
};

class ProgramUnitAST : public BaseAST {
    public:
    std::unique_ptr<BaseAST> comp_unit;
    void Dump(std::string& ret_str) const override {
        ret_str = "";
        cur_func_name.clear();
        ret_str += "decl @getint(): i32\n";
        ret_str += "decl @getch(): i32\n";
        ret_str += "decl @getarray(*i32): i32\n";
        ret_str += "decl @putint(i32)\n";
        ret_str += "decl @putch(i32)\n";
        ret_str += "decl @putarray(i32, *i32)\n";
        ret_str += "decl @starttime()\n";
        ret_str += "decl @stoptime()\n";
        
        std::cout << "decl @getint(): i32\n";
        std::cout << "decl @getch(): i32\n";
        std::cout << "decl @getarray(*i32): i32\n";
        std::cout << "decl @putint(i32)\n";
        std::cout << "decl @putch(i32)\n";
        std::cout << "decl @putarray(i32, *i32)\n";
        std::cout << "decl @starttime()\n";
        std::cout << "decl @stoptime()\n";

        func_table["getint"] = "int";
        func_table["getch"] = "int";
        func_table["getarray"] = "int";
        func_table["putint"] = "void";
        func_table["putch"] = "void";
        func_table["putarray"] = "void";
        func_table["starttime"] = "void";
        func_table["stoptime"] = "void";
        
        comp_unit->Dump(ret_str);
    }
};

// CompUnit 是 BaseAST
class CompUnitAST : public BaseAST {
 public:
  // 用智能指针管理对象
  CompUnitAST(){
    type = _CompUnit;
  }
  std::unique_ptr<BaseAST> inner_comp_unit;
  std::unique_ptr<BaseAST> func_def;
  std::unique_ptr<BaseAST> decl;
  void Dump(std::string& ret_str) const override {
    std::cout << "COMPUNIT" << std::endl;
    // std::cout << "fun ";
    // std::cout << "CompUnitAST { "
    if(branch[0]->type == _CompUnit){
        std::cout << "COMPUNIT in COMPUNIT" << std::endl;
        std::cout << "COMPUNIT's NEXT" << int(branch[1]->type == _FuncDef) << std::endl;
        inner_comp_unit->Dump(ret_str);
        if(branch[1]->type == _FuncDef){
            func_def->Dump(ret_str);
        }
        else if(branch[1]->type == _Decl){
            decl->Dump(ret_str);
        }
    }
    else if(branch[0]->type == _FuncDef){
        std::cout << "COMPUNIT in FUNCDEF" << std::endl;
        func_def->Dump(ret_str);
    }
    else if(branch[0]->type == _Decl){
        std::cout << "COMPUNIT in DECL" << std::endl;
        decl->Dump(ret_str);
        std::cout << "COMPUNIT DECL FINISH" << std::endl;
    }
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
            ret_str += ": i32";
            std::cout << ": i32";
        }
        else if(func_type_str == "void") {
            return;
        }
        // std::cout << " }";
    }
};

class BTypeAST : public BaseAST{
    public:
    std::string btype_name;
    void Dump(std::string& ret_str) const override{
        return;
    }
};

class MultiFuncFParamAST : public BaseAST {
    public:
    MultiFuncFParamAST(){
        type = _MultiFuncFParam;
    }
    std::unique_ptr<BaseAST> multi_funcf_param;
    std::unique_ptr<BaseAST> funcf_param;
    void Dump(std::string& ret_str) const override {
        if(branch.size() == 0){
            return;
        }
        else if(branch[0]->type == _MultiFuncFParam){
            multi_funcf_param->Dump(ret_str);
            ret_str += ", ";
            std::cout << ", ";
            funcf_param->Dump(ret_str);
        }
        else if(branch[0]->type == _FuncFParam){
            funcf_param->Dump(ret_str);
        }
    }
    std::vector<std::string> Param(std::string& ret_str) override{
        std::vector<std::string> ans;
        std::vector<std::string> multi_ans;
        std::vector<std::string> single_ans;
        if(branch.size() == 0){
            ans.clear();
        }
        else if(branch[0]->type == _MultiFuncFParam){
            multi_ans = multi_funcf_param->Param(ret_str);
            single_ans = funcf_param->Param(ret_str);
            int multi_len = multi_ans.size();
            for(int i = 0; i < multi_len; ++ i){
                ans.push_back(multi_ans[i]);
            }
            ans.push_back(single_ans[0]);
        }
        else if(branch[0]->type == _FuncFParam){
            std::cout << "SINGLE PARAM STORE TO VEC" << std::endl;
            single_ans = funcf_param->Param(ret_str);
            ans.push_back(single_ans[0]);
        }
        return ans;
    }
};

class FuncFParamAST : public BaseAST {
    public:
    FuncFParamAST(){
        type = _FuncFParam;
    }
    std::unique_ptr<BaseAST> btype;
    std::string ident;
    void Dump(std::string& ret_str) const override {
        ret_str += "@" + ident;
        std::cout << "@" + ident;
        BTypeAST * btype_ptr = (BTypeAST *)branch[0];
        if(branch.size() == 2){
            if(btype_ptr->btype_name == "int"){
                ret_str += " : i32";
                std::cout << " : i32";
                cur_param_list[ident] = _PARAM_NUM;
            }
        }
        else if(branch.size() == 3){ // 数组
            if(btype_ptr->btype_name == "int"){
                ret_str += " : *i32";
                std::cout << " : *i32";
                cur_param_list[ident] = _PARAM_ARRAY;
            }
        }
    }
    std::vector<std::string> Param(std::string& ret_str) override {
        std::vector<std::string> ans;
        ans.push_back(ident);
        return ans;
    }
};

// FuncDef 也是 BaseAST
class FuncDefAST : public BaseAST {
 public:
 FuncDefAST(){
    type = _FuncDef;
 }
  std::unique_ptr<BaseAST> func_type;
  std::string ident;
  std::unique_ptr<BaseAST> block;
  std::unique_ptr<BaseAST> multi_funcf_params;
  void Dump(std::string& ret_str) const override {
    std::cout << "FUNCDEF" << std::endl;
    // 保存函数类型函数名到全局函数符号表
    CurParamType param_type_list;
    // param_type_list.clear();
    program_param_list[ident] = param_type_list;
    cur_param_list = program_param_list[ident];
    cur_param_list.clear(); // 清空参数类型表
    BTypeAST* func_type_ptr = (BTypeAST *)branch[0];
    func_table[ident] = func_type_ptr->btype_name;

    // 导出参数列表
    MultiFuncFParamAST* func_params = (MultiFuncFParamAST *)branch[2];

    ret_str += "fun ";
    ret_str += "@";
    ret_str += ident;
    ret_str += "(";
    // std::cout << "FuncDefAST { ";
    std::cout << "fun ";
    
    std::cout << "@" << ident << "(";

    multi_funcf_params->Dump(ret_str);

    ret_str += ") ";
    std::cout << ") ";
    // func_type->Dump(ret_str);

    if(func_table[ident] == "int"){
        ret_str += ": i32";
        std::cout << ": i32";
    }
    else if(func_table[ident] == "void"){

    }

    ret_str += " { \n";
    std::cout << " {" << std::endl;

    ret_str += "%entry:\n";
    std::cout << "%entry:" << std::endl;

    SymTableList sym_table_list;
    sym_table_list.clear();
    program_sym_table[ident] = sym_table_list;
    cur_sym_table_list = &sym_table_list;
    cur_func_name = ident;
    std::cout << "IDENT IS" << cur_func_name << std::endl; 
    // 进入函数之前的sym_table中的内容和函数内部symtable的内容不互通
    cur_table = NULL;

    // 进入函数之前，先加一个符号表，表示函数的参数
    block_count ++;
    SymbolTable symbol_table;
    symbol_table.table_index = block_count;
    symbol_table.func_name = cur_func_name;
    symbol_table.parent = cur_table;
    symbol_table.returned = false;
    symbol_table.blocking = false;
    //将符号表改为当前符号表（之前的符号表一定是这个block的parent）
    cur_table = &symbol_table;
    cur_sym_table_list->push_back(symbol_table);

    std::cout << "BUILD A SYMTABLE FOR FUNCTION" << std::endl;
    std::vector<std::string> param_list = func_params->Param(ret_str);
    int param_len = param_list.size();
    std::cout << "PARAM SIZE " << param_len << std::endl;
    for(int i = 0; i < param_len; ++ i){

        std::string param_register = "%" + param_list[i] + "_" + cur_func_name + "_" + std::to_string(cur_table->table_index);
        std::string param_name = "@" + param_list[i];
        ret_str += "    ";
        ret_str += param_register;
        ret_str += " = alloc ";
        if(cur_param_list[param_list[i]] == _PARAM_ARRAY){
            ret_str += "*";
        }
        ret_str += "i32\n";
        ret_str += "    store ";
        ret_str += param_name;
        ret_str += ", ";
        ret_str += param_register;
        ret_str += "\n";

        std::cout << "  ";
        std::cout << param_register;
        std::cout << " = alloc ";
        if(cur_param_list[param_list[i]] == _PARAM_ARRAY){
            std::cout <<  "*";
        }
        std::cout << "i32\n";
        std::cout << "  store ";
        std::cout << param_name;
        std::cout << ", ";
        std::cout << param_register;
        std::cout << "\n";

        Symbol tmp_sym;
        tmp_sym.type = _PARAM;
        cur_table->sym_table[param_list[i]] = tmp_sym;
        
    }

    block->Dump(ret_str);

    if(cur_table->parent == NULL && !cur_table->returned){
        ret_str += "    ret \n";
        std::cout << "  ret \n";
    }

    cur_table = cur_table->parent;
    cur_sym_table_list->pop_back();
    // 退出函数，清空当前函数名
    // 除了函数，就是全局变量，如果cur_func_name为空，那么就意味着是全局变量定义
    cur_func_name.clear();

    ret_str += "}\n";
    std::cout << "}\n";
    // std::cout << " }";
  }
};

// Block 也是 BaseAST
class BlockAST : public BaseAST{
    public:
    BlockAST(){
        type = _Block;
    }
    std::unique_ptr<BaseAST> multi_block_item;
    void Dump(std::string& ret_str) const override {
        std::cout << int(NULL) << std::endl;
        // 进入一个新block，初始化这个block的符号表
        block_count ++;
        SymbolTable symbol_table;
        symbol_table.table_index = block_count;
        symbol_table.func_name = cur_func_name;
        symbol_table.parent = cur_table;
        symbol_table.returned = false;
        symbol_table.blocking = false;
        //将符号表改为当前符号表（之前的符号表一定是这个block的parent）
        cur_table = &symbol_table;
        cur_sym_table_list->push_back(symbol_table);

        std::cout << "BLOCK" << std::endl;
        std::cout << "BLOCKCOUNT = " << std::to_string(block_count) << std::endl;
        // std::cout << "BlockAST { ";
        // if(!branch.empty())

        // 如果之前已经出现一个non blocking的块出现return，那么一定是在这个块之前运行，因此这个块不用输出
        // block之外的return，一定已经在进入block之前整理好
        // 因此不需要在内部语句中涉及到其他block，内部语句只需要判断当前block的状态是否改变
        SymbolTable *find_return_table = cur_table;
        bool return_flag = false;
        while(find_return_table){
            if(find_return_table->returned){
                return_flag = true;
                break;
            }
            find_return_table = find_return_table->parent;
        }
        if(return_flag){
            cur_table->returned = true;
        }
        
        if(!cur_table->returned){
            multi_block_item->Dump(ret_str);
        }
        
        std::cout << "BLOCKFINISH" << std::endl;
        // 这个block结束，内部的block肯定耶已经结束
        // 此时栈顶一定是当前block的符号表
        // 退出是，一定回到parent block
        // 如果当前block已经出现ret指令，那么一直影响到第一个blocking的祖先节点
        if(cur_table->returned){
            SymbolTable *ancestor_table = cur_table;
            // 是否blocking->影响自己与parent节点的returned传递，并不影响自身
            // 如果自身blocking，那么while循环退出，不再传递return给parent节点
            while(!ancestor_table->blocking){
                if(ancestor_table->parent){
                    ancestor_table = ancestor_table->parent;
                    ancestor_table->returned = true;
                }
                else{
                    break;
                }
            }
        }
        cur_table = cur_table->parent;
        cur_sym_table_list->pop_back();
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
        // 所有语句都是从BlockItem解析得出，所以只需要在遇到blockitem时判断该条语句是否可以输出即可
        if(cur_table->returned){
            return;
        }
        if(branch[0]->type == _Decl){
            std::cout << "BLOCKITEM - DECL" << std::endl;
            decl->Dump(ret_str);
        }
        else if(branch[0]->type == _Stmt){
            std::cout << "BLOCKITEM - STMT" << std::endl;
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
    std::unique_ptr<BaseAST> Exp;
    void Dump(std::string& ret_str) const override{

    }
    std::string Calc(std::string& ret_str) override{
        std::cout << "LVALCALC "  << ident << std::endl;
        int depth = cur_table->table_index;
        std::string inside_func = cur_table->func_name;
        SymbolTable *used_table = cur_table;
        Symbol tmp_ans;
        std::string ans;

        // 看看ident到底是哪儿定义的
        while(used_table && used_table->sym_table.find(ident) == used_table->sym_table.end()){
            used_table = used_table->parent;
        }
        // 说明局部没有，看看全局
        if(used_table == NULL){
            // std::cout << "LVAL-SYMTABLE WRONG" << std::endl;
            if(global_var_table.find(ident) == global_var_table.end()){
                std::cout << "LVAL-SYMTABLE WRONG" << std::endl;
            }
            tmp_ans = global_var_table[ident];
            if(tmp_ans.type == _CONST_GLOBAL){
                ans = std::to_string(tmp_ans.sym_val);
            }
            else if(tmp_ans.type == _NUM_GLOBAL){
                ans = "@" + ident + "_" + "global";
            }
            else if(tmp_ans.type == _CONST_GLOBAL_ARRAY){
                ans = "%" + ident + "_" + "global_ptr" + "_" + std::to_string(ptr_count);
                ptr_count ++;
                std::string arr_name = "@" + ident + "_" + "global";
                std::string index;
                std::string index_ans;
                if(branch.size() == 1){ // 数组参数
                    index = "0";
                    index_ans = index;
                }
                else if(branch.size() == 2){ // int
                    index = Exp->Calc(ret_str);
                    index_ans = index;
                }

                if(index[0] == '@' || (index[0] == '%' && (index[1] > '9' || index[1] < '0'))){
                    index_ans = "%" + std::to_string(var_count);
                    var_count ++;

                    ret_str += "    ";
                    ret_str += index_ans;
                    ret_str += " = ";
                    ret_str += "load ";
                    ret_str += index;
                    ret_str += "\n";

                    std::cout << "    ";
                    std::cout << index_ans;
                    std::cout << " = ";
                    std::cout << "load ";
                    std::cout << index;
                    std::cout << "\n";
                }

                ret_str += "    ";
                ret_str += ans;
                ret_str += " = getelemptr ";
                ret_str += arr_name;
                ret_str += ", ";
                ret_str += index_ans;
                ret_str += "\n";

                std::cout << "    ";
                std::cout << ans;
                std::cout << " = getelemptr ";
                std::cout << arr_name;
                std::cout << ", ";
                std::cout << index_ans;
                std::cout << "\n";
            }
            else if(tmp_ans.type == _GLOBAL_ARRAY){
                ans = "%" + ident + "_" + "global_ptr" + "_" + std::to_string(ptr_count);
                ptr_count ++;
                std::string arr_name = "@" + ident + "_" + "global";
                std::string index;
                std::string index_ans;
                if(branch.size() == 1){ // 数组参数
                    index = "0";
                    index_ans = index;
                }
                else if(branch.size() == 2){ // int
                    index = Exp->Calc(ret_str);
                    index_ans = index;
                }

                if(index[0] == '@' || (index[0] == '%' && (index[1] > '9' || index[1] < '0'))){
                    index_ans = "%" + std::to_string(var_count);
                    var_count ++;

                    ret_str += "    ";
                    ret_str += index_ans;
                    ret_str += " = ";
                    ret_str += "load ";
                    ret_str += index;
                    ret_str += "\n";

                    std::cout << "    ";
                    std::cout << index_ans;
                    std::cout << " = ";
                    std::cout << "load ";
                    std::cout << index;
                    std::cout << "\n";
                }

                ret_str += "    ";
                ret_str += ans;
                ret_str += " = getelemptr ";
                ret_str += arr_name;
                ret_str += ", ";
                ret_str += index_ans;
                ret_str += "\n";

                std::cout << "    ";
                std::cout << ans;
                std::cout << " = getelemptr ";
                std::cout << arr_name;
                std::cout << ", ";
                std::cout << index_ans;
                std::cout << "\n";
            }
        }
        
        else{ // 说明在局部找到了
            depth = used_table->table_index;
            inside_func = used_table->func_name;
            
            tmp_ans = used_table->sym_table[ident];
            std::cout << "LVALTYPE " << tmp_ans.type << std::endl; 
            if(tmp_ans.type == _CONST){
                ans = std::to_string(tmp_ans.sym_val);
            }
            else if(tmp_ans.type == _NUM || tmp_ans.type == _UNK || tmp_ans.type == _NUM_UNCALC){
                std::cout << "NUM OR UNK" << std::endl;
                ans = "@" + ident + "_" + inside_func + "_" + std::to_string(depth);
            }
            else if(tmp_ans.type == _CONST_UNCALC){
                std::cout << "IMPOSSIBLE CONST UNCALC" << std::endl;
            }
            else if(tmp_ans.type == _PARAM){
                ans = "%" + ident + "_" + inside_func + "_" + std::to_string(depth);
            }
            else if(tmp_ans.type == _CONST_ARRAY){
                ans = "%" + ident +  + "_" + inside_func + "_" + std::to_string(depth) + "_" + "ptr" + "_" + std::to_string(ptr_count);
                ptr_count ++;
                std::string arr_name = "@" + ident + "_" + inside_func + "_" + std::to_string(depth);
                std::string index;
                std::string index_ans;
                if(branch.size() == 1){ // 数组参数
                    index = "0";
                    index_ans = index;
                }
                else if(branch.size() == 2){ // int
                    index = Exp->Calc(ret_str);
                    index_ans = index;
                }

                if(index[0] == '@' || (index[0] == '%' && (index[1] > '9' || index[1] < '0'))){
                    index_ans = "%" + std::to_string(var_count);
                    var_count ++;

                    ret_str += "    ";
                    ret_str += index_ans;
                    ret_str += " = ";
                    ret_str += "load ";
                    ret_str += index;
                    ret_str += "\n";

                    std::cout << "    ";
                    std::cout << index_ans;
                    std::cout << " = ";
                    std::cout << "load ";
                    std::cout << index;
                    std::cout << "\n";
                }

                ret_str += "    ";
                ret_str += ans;
                ret_str += " = getelemptr ";
                ret_str += arr_name;
                ret_str += ", ";
                ret_str += index_ans;
                ret_str += "\n";

                std::cout << "    ";
                std::cout << ans;
                std::cout << " = getelemptr ";
                std::cout << arr_name;
                std::cout << ", ";
                std::cout << index_ans;
                std::cout << "\n";
            }
            else if(tmp_ans.type == _ARRAY){
                ans = "%" + ident +  + "_" + inside_func + "_" + std::to_string(depth) + "_" + "ptr" + "_" + std::to_string(ptr_count);
                ptr_count ++;
                std::string arr_name = "@" + ident + "_" + inside_func + "_" + std::to_string(depth);
                std::string index;
                std::string index_ans;
                if(branch.size() == 1){ // 数组参数
                    index = "0";
                    index_ans = index;
                }
                else if(branch.size() == 2){ // int
                    index = Exp->Calc(ret_str);
                    index_ans = index;
                }

                if(index[0] == '@' || (index[0] == '%' && (index[1] > '9' || index[1] < '0'))){
                    index_ans = "%" + std::to_string(var_count);
                    var_count ++;

                    ret_str += "    ";
                    ret_str += index_ans;
                    ret_str += " = ";
                    ret_str += "load ";
                    ret_str += index;
                    ret_str += "\n";

                    std::cout << "    ";
                    std::cout << index_ans;
                    std::cout << " = ";
                    std::cout << "load ";
                    std::cout << index;
                    std::cout << "\n";
                }

                ret_str += "    ";
                ret_str += ans;
                ret_str += " = getelemptr ";
                ret_str += arr_name;
                ret_str += ", ";
                ret_str += index_ans;
                ret_str += "\n";

                std::cout << "    ";
                std::cout << ans;
                std::cout << " = getelemptr ";
                std::cout << arr_name;
                std::cout << ", ";
                std::cout << index_ans;
                std::cout << "\n";
            }
        }
        return ans;
    }
    RetVal Calc_val() override{
        std::cout << "LVALCALC" << std::endl;

        SymbolTable *used_table = cur_table;
        Symbol tmp_ans;
        RetVal ans;
        // 看看ident到底是哪儿定义的
        while(used_table && used_table->sym_table.find(ident) == used_table->sym_table.end()){
            used_table = used_table->parent;
        }
        if(used_table == NULL){
            tmp_ans = global_var_table[ident];
        }
        else{
            tmp_ans = used_table->sym_table[ident];
        }
        // Symbol tmp_ans = cur_table->sym_table[ident];
        std::cout << "LVALTYPE in calc_val " << tmp_ans.type << std::endl; 
        if(tmp_ans.type == _NUM || tmp_ans.type == _CONST || tmp_ans.type == _CONST_GLOBAL){
            ans.value = tmp_ans.sym_val;
            ans.calcable = true;
        }
        else if(tmp_ans.type == _ARRAY || tmp_ans.type == _CONST_ARRAY || tmp_ans.type == _CONST_GLOBAL_ARRAY){
            RetVal index_exp = Exp->Calc_val();
            if(index_exp.calcable){
                ans.calcable = true;
                ans.value = tmp_ans.array_val[index_exp.value];
            }
            else{
                ans.calcable = false;
            }
        }
        else { // PARAM, GLOBAL, NUM_UNCALC, UNK
            ans.calcable = false;
        }
        return ans;
    }
};

class StmtAST : public BaseAST {
    public:
    StmtAST() {
        type = _Stmt;
    }
    std::unique_ptr<BaseAST> matched_stmt; 
    std::unique_ptr<BaseAST> open_stmt;
    void Dump(std::string& ret_str) const override {
        std::cout << "STMT" << std::endl;
        if(branch[0]->type == _MatchedStmt){
            std::cout << "STMT->MATCHED STMT" << std::endl;
            matched_stmt->Dump(ret_str);
        }
        else if(branch[0]->type == _OpenStmt){
            std::cout << "STMT->OPEN STMT" << std::endl;
            open_stmt->Dump(ret_str);
        }
    }
};

class OpenStmtAST : public BaseAST {
    public:
    OpenStmtAST() {
        type = _OpenStmt;
    }
    std::unique_ptr<BaseAST> Exp;
    std::unique_ptr<BaseAST> stmt;
    std::unique_ptr<BaseAST> matched_stmt;
    std::unique_ptr<BaseAST> open_stmt;
    void Dump(std::string& ret_str) const override{
        // if exp then stmt
        if(branch.size() == 2){
            /* if(exp) */
            std::string tmp1 = Exp->Calc(ret_str);
            std::string cond = tmp1;
            std::string end = "%end_" + std::to_string(end_count);
            std::string then = "%then_" + std::to_string(if_count);

            end_count ++;
            if_count ++;
            if(tmp1[0] == '@' || (tmp1[0] == '%' && (tmp1[1] > '9' || tmp1[1] < '0'))){
                cond = "%" + std::to_string(var_count);
                var_count ++;

                ret_str += "    ";
                ret_str += cond;
                ret_str += " = ";
                ret_str += "load ";
                ret_str += tmp1;
                ret_str += "\n";

                std::cout << "    ";
                std::cout << cond;
                std::cout << " = ";
                std::cout << "load ";
                std::cout << tmp1;
                std::cout << "\n";
            }

            ret_str += "    br    ";
            ret_str += cond;
            ret_str += ", ";
            ret_str += then;
            ret_str += ", ";
            ret_str += end;
            ret_str += "\n";

            std::cout << "  br    ";
            std::cout << cond;
            std::cout << ", ";
            std::cout << then;
            std::cout << ", ";
            std::cout << end;
            std::cout << std::endl;

            /* { */
            // 初始化if分支的符号表
            block_count ++;
            SymbolTable symbol_table_if_then;
            symbol_table_if_then.table_index = block_count;
            symbol_table_if_then.func_name = cur_func_name;
            symbol_table_if_then.parent = cur_table;
            symbol_table_if_then.returned = false;
            symbol_table_if_then.blocking = true; //if的{}是blocking的，在if内部return并不影响在main和else中return
            //将符号表改为当前符号表（之前的符号表一定是这个block的parent）
            cur_table = &symbol_table_if_then;
            cur_sym_table_list->push_back(symbol_table_if_then);

            ret_str += "\n";
            ret_str += then;
            ret_str += ":\n";
            std::cout << std::endl;
            std::cout << then;
            std::cout << ":\n";
            stmt->Dump(ret_str);

            if(!cur_table->returned){
                ret_str += "    jump    ";
                ret_str += end;
                std::cout << "    jump    ";
                std::cout << end;
            }

            cur_table = cur_table->parent;
            cur_sym_table_list->pop_back();

            /* } */

            ret_str += "\n";
            ret_str += end;
            ret_str += ":\n";
            std::cout << std::endl;
            std::cout << end;
            std::cout << ":\n";
        }
        else if(branch.size() == 3){ // if exp matched_stmt else open_stmt
            /* if(exp) */
            std::string tmp1 = Exp->Calc(ret_str);
            std::string cond = tmp1;
            std::string end = "%end_" + std::to_string(end_count);
            std::string then = "%then_" + std::to_string(if_count);
            std::string else_str = "%else_" + std::to_string(else_count);
            end_count ++;
            if_count ++;
            else_count ++;
            if(tmp1[0] == '@' || (tmp1[0] == '%' && (tmp1[1] > '9' || tmp1[1] < '0'))){
                cond = "%" + std::to_string(var_count);
                var_count ++;

                ret_str += "    ";
                ret_str += cond;
                ret_str += " = ";
                ret_str += "load ";
                ret_str += tmp1;
                ret_str += "\n";

                std::cout << "    ";
                std::cout << cond;
                std::cout << " = ";
                std::cout << "load ";
                std::cout << tmp1;
                std::cout << "\n";
            }
            
            ret_str += "    br    ";
            ret_str += cond;
            ret_str += ", ";
            ret_str += then;
            ret_str += ", ";
            ret_str += else_str;
            ret_str += "\n";

            std::cout << "  br    ";
            std::cout << cond;
            std::cout << ", ";
            std::cout << then;
            std::cout << ", ";
            std::cout << else_str;
            std::cout << std::endl;

            /* { */
            // 初始化if分支的符号表
            block_count ++;
            SymbolTable symbol_table_if;
            symbol_table_if.table_index = block_count;
            symbol_table_if.func_name = cur_func_name;
            symbol_table_if.parent = cur_table;
            symbol_table_if.returned = false;
            symbol_table_if.blocking = true; //if的{}是blocking的，在if内部return并不影响在main和else中return
            //将符号表改为当前符号表（之前的符号表一定是这个block的parent）
            cur_table = &symbol_table_if;
            cur_sym_table_list->push_back(symbol_table_if);

            ret_str += "\n";
            ret_str += then;
            ret_str += ":\n";
            std::cout << std::endl;
            std::cout << then;
            std::cout << ":\n";
            matched_stmt->Dump(ret_str);

            if(!cur_table->returned){
                ret_str += "    jump    ";
                ret_str += end;
                std::cout << "    jump    ";
                std::cout << end;
            }

            cur_table = cur_table->parent;
            cur_sym_table_list->pop_back();
            /* } */
            
            /* else {*/
            // 初始化else分支的符号表
            block_count ++;
            SymbolTable symbol_table_else;
            symbol_table_else.table_index = block_count;
            symbol_table_else.func_name = cur_func_name;
            symbol_table_else.parent = cur_table;
            symbol_table_else.returned = false;
            symbol_table_else.blocking = true; //if的{}是blocking的，在if内部return并不影响在main和else中return
            //将符号表改为当前符号表（之前的符号表一定是这个block的parent）
            cur_table = &symbol_table_else;
            cur_sym_table_list->push_back(symbol_table_else);

            ret_str += "\n";
            ret_str += else_str;
            ret_str += ":\n";
            std::cout << std::endl;
            std::cout << else_str;
            std::cout << ":\n";
            open_stmt->Dump(ret_str); 
            // 虽然open_stmt内部的return一定不会传导到当前block（因为stmt可能的block均为blocking的block）
            // 为了整齐，依然这么写了

            if(!cur_table->returned){
                std::cout << "WRONG RETURNED SPREAD" << std::endl;
                ret_str += "    jump    ";
                ret_str += end;
                std::cout << "    jump    ";
                std::cout << end;
            }

            cur_table = cur_table->parent;
            cur_sym_table_list->pop_back();
            /* } */

            ret_str += "\n";
            ret_str += end;
            ret_str += ":\n";
            std::cout << std::endl;
            std::cout << end;
            std::cout << ":\n";
        }
    }

};

// matched_stmt可能解析出ret语句，所以所有可能解析出matched_stmt的语句都需要在内部判断是否还要继续生成下一个句子
class MatchedStmtAST : public BaseAST{
    public:
    MatchedStmtAST(){
        type = _MatchedStmt;
    }
    // int number;
    std::unique_ptr<BaseAST> LVal;
    std::unique_ptr<BaseAST> Exp;
    std::unique_ptr<BaseAST> block;
    std::unique_ptr<BaseAST> matched_stmt_1;
    std::unique_ptr<BaseAST> matched_stmt_2;
    std::unique_ptr<BaseAST> stmt;
    void Dump(std::string& ret_str) const override {
        std::cout << "MATCHED STMT" << std::endl;
        if(branch.size() == 0){
            return;
        }
        else if (branch[0]->type == _Return){
            std::cout << "STMTRETEXP" << std::endl;
            std::string ans;
            ans = Exp->Calc(ret_str);
            std::cout << ans << std::endl;
            std::cout << "AAA" << branch.size() << std::endl;
            if(branch.size() == 1){
                ret_str += "    ret ";
                std::cout << "   ret ";
            }
            else if(branch.size() == 2){
                if(ans[0] == '@' || (ans[0] == '%' && (ans[1] > '9' || ans[1] < '0'))){ // 具名变量
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
                    ret_str += "\n";
                    std::cout << "   ret ";
                    std::cout << tmp;
                    std::cout << std:: endl;
                }
                else { // 立即数或%0
                    ret_str += "    ret ";
                    ret_str += ans;
                    ret_str += "\n";
                    std::cout << "   ret ";
                    std::cout << ans;
                    std::cout << std::endl;
                }
            }
            // 能进入这个ret语句，之前一定returned = false
            // 已经输出一个ret语句，这个block内其他语句不能再生成
            cur_table->returned = true;
        }
        else if (branch[0]->type == _LVal){ //赋值 有可能是赋值给参数同名的变量
            std::cout << "STMTLVAL" << std::endl;
            LValAST * cur_branch = (LValAST *)branch[0];
            std::string tmp1;
            std::string ans1;
            std::string ans2;
            std::string var_name = cur_branch->ident;
            int value;
            RetVal calc_value;

            calc_value = cur_branch->Calc_val();
            if(calc_value.calcable){
                value = calc_value.value;
            }

            // int depth = cur_table->table_index;
            // cur table 会不会变？
            SymbolTable* used_table = cur_table;

            // 看看var_name到底是哪儿定义的
            while(used_table && used_table->sym_table.find(var_name) == used_table->sym_table.end()){
                used_table = used_table->parent;
            }

            // int depth = used_table->table_index;

            // 计算结果已经由Exp->Calc(ret_str)得出
            // 可以是数或%0
            // 变量名已经由LVal->Calc(ret_str)整理好
            ans2 = cur_branch->Calc(ret_str);
            std::cout << "ans2 " << ans2 << std::endl; 
            std::cout << int(cur_branch->type == _LVal) << std::endl;
            tmp1 = Exp->Calc(ret_str);

            ans1 = tmp1;

            if(tmp1[0] == '@' || (tmp1[0] == '%' && (tmp1[1] > '9' || tmp1[1] < '0'))){
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

            // 只可能有UNK, NUM, GLOBAL, PARAM
            // 先取出来看看
            Symbol symb;
            if(used_table == NULL){
                symb = global_var_table[var_name];
            }
            else {
               symb = used_table->sym_table[var_name];
            }
            std::cout << "LVAL = EXP CALCABLE? " << calc_value.calcable << std::endl;
            std::cout << "LVALTYPE " << symb.type << std::endl;
            if(calc_value.calcable){
                if(symb.type == _UNK || symb.type == _NUM){
                    symb.type = _NUM;
                }
                symb.sym_val = value;
                if(used_table == NULL){
                    global_var_table[var_name] = symb; // 如果是全局变量，存储到全局symtable中
                }
                else{
                    used_table->sym_table[var_name] = symb; // 存到对应的symtable中
                }
            }
            else{
                if(symb.type == _UNK || symb.type == _NUM){
                    symb.type = _NUM_UNCALC;
                }
            }
            
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
        else if(branch[0]->type == _Exp){
            std::cout << "STMTEXP" << std::endl;
            std::string ans;
            if(branch.size() == 1){
                ans = Exp->Calc(ret_str);
            }
            else if(branch.size() == 2){ // while exp stmt
                std::cout << "WHILE BRANCH" << std::endl;
                std::string while_entry = "%while_entry_" + std::to_string(while_count);
                std::string while_body = "%while_body_" + std::to_string(while_count);
                std::string while_to_entry = "%while_to_entry_" + std::to_string(while_count);
                std::string while_end = "%while_end_" + std::to_string(while_count);
                std::string tmp1;
                std::string cond;
                prev_cur_while = cur_while;
                cur_while = while_count;
                cur_while_end = end_count;
                while_count ++;
                // end_count ++;

                ret_str += "    jump    ";
                ret_str += while_entry;
                ret_str += "\n";

                std::cout << "  jump    ";
                std::cout << while_entry;
                std::cout << "\n";

                /* while (exp) */
                ret_str += while_entry;
                ret_str += ":\n";

                std::cout << while_entry;
                std::cout << ":\n";

                tmp1 = Exp->Calc(ret_str);
                cond = tmp1;

                if(tmp1[0] == '@' || (tmp1[0] == '%' && (tmp1[1] > '9' || tmp1[1] < '0'))){
                    cond = "%" + std::to_string(var_count);
                    var_count ++;

                    ret_str += "    ";
                    ret_str += cond;
                    ret_str += " = ";
                    ret_str += "load ";
                    ret_str += tmp1;
                    ret_str += "\n";

                    std::cout << "    ";
                    std::cout << cond;
                    std::cout << " = ";
                    std::cout << "load ";
                    std::cout << tmp1;
                    std::cout << "\n";
                }

                ret_str += "    br    ";
                ret_str += cond;
                ret_str += ", ";
                ret_str += while_body;
                ret_str += ", ";
                ret_str += while_end;
                ret_str += "\n";

                std::cout << "  br    ";
                std::cout << cond;
                std::cout << ", ";
                std::cout << while_body;
                std::cout << ", ";
                std::cout << while_end;
                std::cout << std::endl;

                /* { */
                // 初始化while分支的符号表
                block_count ++;
                SymbolTable symbol_table_while;
                symbol_table_while.table_index = block_count;
                symbol_table_while.func_name = cur_func_name;
                symbol_table_while.parent = cur_table;
                symbol_table_while.returned = false;
                symbol_table_while.blocking = true; 
                // while的{}不是blocking的
                // 在while内部return，也是在parent节点中return
                //将符号表改为当前符号表（之前的符号表一定是这个block的parent）
                cur_table = &symbol_table_while;
                cur_sym_table_list->push_back(symbol_table_while);

                ret_str += while_body;
                ret_str += ":\n";

                std::cout << while_body;
                std::cout << ":\n";

                bool last_break_continue = break_continue;
                break_continue = false;
                stmt->Dump(ret_str); // 

                std::cout << "WHILE STMT RETURN" << std::endl;

                if(!cur_table->returned){
                    ret_str += "    jump    ";
                    ret_str += while_entry;
                    std::cout << "    jump    ";
                    std::cout << while_entry;
                }

                break_continue = last_break_continue;

                // while {}不是blocking的，因此需要把信息传递给parent节点和祖先节点
                // 如果当前block已经出现ret指令，那么一直影响到第一个blocking的祖先节点
                /*if(cur_table->returned){
                    SymbolTable *ancestor_table = cur_table;
                    // 是否blocking->影响自己与parent节点的returned传递，并不影响自身
                    // 如果自身blocking，那么while循环退出，不再传递return给parent节点
                    while(!ancestor_table->blocking){
                        if(ancestor_table->parent){
                            ancestor_table = ancestor_table->parent;
                            ancestor_table->returned = true;
                        }
                        else{
                            break;
                        }
                    }
                }*/
                cur_table = cur_table->parent;
                cur_sym_table_list->pop_back();
                /* } */

                ret_str += "\n";
                ret_str += while_end;
                ret_str += ":\n";
                std::cout << std::endl;
                std::cout << while_end;
                std::cout << ":\n";

                cur_while = prev_cur_while;

            }
            else if(branch.size() == 3){ // if exp then matched_stmt_1 else matched_stmt_2
                // 每个if都可以有大括号，因此可以假定每个if是单独的一个block
                // 本身有{}没关系，本身的{}由BlockAST初始化，blocking = false
                // 退出该block时这个block会把return状况传递给我们手动设置的这个if block
                // if(exp)本身在这个block外

                /* if (exp) */

                std::string tmp1 = Exp->Calc(ret_str);
                std::string cond = tmp1;
                std::string end = "%end_" + std::to_string(end_count);
                std::string then = "%then_" + std::to_string(if_count);
                std::string else_str = "%else_" + std::to_string(else_count);
                end_count ++;
                if_count ++;
                else_count ++;

                if(tmp1[0] == '@' || (tmp1[0] == '%' && (tmp1[1] > '9' || tmp1[1] < '0'))){
                    cond = "%" + std::to_string(var_count);
                    var_count ++;

                    ret_str += "    ";
                    ret_str += cond;
                    ret_str += " = ";
                    ret_str += "load ";
                    ret_str += tmp1;
                    ret_str += "\n";

                    std::cout << "    ";
                    std::cout << cond;
                    std::cout << " = ";
                    std::cout << "load ";
                    std::cout << tmp1;
                    std::cout << "\n";
                }
                
                ret_str += "    br    ";
                ret_str += cond;
                ret_str += ", ";
                ret_str += then;
                ret_str += ", ";
                ret_str += else_str;
                ret_str += "\n";

                std::cout << "  br    ";
                std::cout << cond;
                std::cout << ", ";
                std::cout << then;
                std::cout << ", ";
                std::cout << else_str;
                std::cout << std::endl;

                /* { */
                // 初始化if分支的符号表
                block_count ++;
                SymbolTable symbol_table_if;
                symbol_table_if.table_index = block_count;
                symbol_table_if.func_name = cur_func_name;
                symbol_table_if.parent = cur_table;
                symbol_table_if.returned = false;
                symbol_table_if.blocking = true; //if的{}是blocking的，在if内部return并不影响在main和else中return
                //将符号表改为当前符号表（之前的符号表一定是这个block的parent）
                cur_table = &symbol_table_if;
                cur_sym_table_list->push_back(symbol_table_if);

                ret_str += "\n";
                ret_str += then;
                ret_str += ":\n";
                std::cout << std::endl;
                std::cout << then;
                std::cout << ":\n";
                // 可能是ret
                matched_stmt_1->Dump(ret_str);

                if(!cur_table->returned){
                    ret_str += "    jump    ";
                    ret_str += end;
                    std::cout << "    jump    ";
                    std::cout << end;
                }

                // if 的block本身是blocking的，因此不需要更新parent及祖先节点的returned值
                cur_table = cur_table->parent;
                cur_sym_table_list->pop_back();

                /* } */

                /* else { */
                // 初始化else分支的符号表
                block_count ++;
                SymbolTable symbol_table_else;
                symbol_table_else.table_index = block_count;
                symbol_table_else.func_name = cur_func_name;
                symbol_table_else.parent = cur_table;
                symbol_table_else.returned = false;
                symbol_table_else.blocking = true; //if的{}是blocking的，在if内部return并不影响在main和else中return
                //将符号表改为当前符号表（之前的符号表一定是这个block的parent）
                cur_table = &symbol_table_else;
                cur_sym_table_list->push_back(symbol_table_else);

                ret_str += "\n";
                ret_str += else_str;
                ret_str += ":\n";
                std::cout << std::endl;
                std::cout << else_str;
                std::cout << ":\n";
                matched_stmt_2->Dump(ret_str); // 可能有return

                if(!cur_table->returned){
                    ret_str += "    jump    ";
                    ret_str += end;
                    std::cout << "    jump    ";
                    std::cout << end;
                }

                // else的block本身是blocking的，因此不需要更新parent及祖先节点的returned值
                cur_table = cur_table->parent;
                cur_sym_table_list->pop_back();

                /* } */

                ret_str += "\n";
                ret_str += end;
                ret_str += ":\n";
                std::cout << std::endl;
                std::cout << end;
                std::cout << ":\n";
            }
        }
        else if(branch[0]->type == _Block){
            std::cout << "STMTBLOCK" << std::endl;
            block->Dump(ret_str);
        }
        else if(branch[0]->type == _Break){
            break_continue = true;
            std::string break_end = "%while_end_" + std::to_string(cur_while);
            std::string break_follow = "%follow_break_" + std::to_string(cur_while);
            ret_str += "    jump    ";
            ret_str += break_end;
            ret_str += "\n";

            std::cout << "    jump    ";
            std::cout <<  break_end;
            std::cout << "\n";

            cur_table->returned = true;

            /*
            ret_str += break_follow;
            ret_str += ":\n";

            std::cout << break_follow;
            std::cout << ":\n";
            */
        }
        else if(branch[0]->type == _Continue){
            break_continue = true;
            std::string continue_head = "%while_entry_" + std::to_string(cur_while);
            std::string continue_follow = "%follow_continue_" + std::to_string(cur_while);
            ret_str += "    jump    ";
            ret_str += continue_head;
            ret_str += "\n";

            std::cout <<  "    jump    ";
            std::cout <<  continue_head;
            std::cout <<  "\n";

            cur_table->returned = true;

            /*
            ret_str += continue_follow;
            ret_str += ":\n";

            std::cout << continue_follow;
            std::cout << ":\n";
            */
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
    RetVal Calc_val() override{
        RetVal ans;
        ans.calcable = true;
        ans.value = number;
        return ans;
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
    RetVal Calc_val() override{
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
    RetVal Calc_val() override{
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
    std::string ident;
    std::unique_ptr<BaseAST> multi_funcr_param;
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
            
            if(tmp[0] == '@' || (tmp[0] == '%' && (tmp[1] > '9' || tmp[1] < '0'))){
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
        else if(branch[0]->type == _Ident){ // 函数调用
            std::cout << "CHECK FUNC TYPE" << func_table[ident] << std::endl;
            CurParamType prev_param_list = cur_param_list;
            cur_param_list = program_param_list[ident];
            std::vector<std::string> cur_param = multi_funcr_param->Param(ret_str);
            int length = cur_param.size();
            if(func_table[ident] == "int"){
                ans = "%" + std::to_string(var_count);
                var_count ++;

                ret_str += "    ";
                ret_str += ans;
                ret_str += " = ";
                
                std::cout << "    ";
                std::cout << ans;
                std::cout << " = ";
            }
            else if(func_table[ident] == "void"){
                ret_str += "    ";
                std::cout << "  ";
            }

            int param_size = cur_param.size();
            ret_str += "call @";
            ret_str += ident;
            ret_str += "(";

            std::cout << "call @";
            std::cout << ident;
            std::cout << "(";

            if(param_size){
                ret_str += cur_param[0];
                std::cout << cur_param[0];
            }
            for(int i = 1; i < param_size; ++ i){
                ret_str += ", ";
                ret_str += cur_param[i];

                std::cout << ", ";
                std::cout << cur_param[i];
            }
            ret_str += ")\n";
            std::cout << ")\n";
            cur_param_list = prev_param_list;
        }

        return ans;
    }
    RetVal Calc_val() override{
        RetVal ans;
        if(branch[0]->type == _PrimaryExp){
            ans = branch[0]->Calc_val();
        }
        else if(branch[0]->type == _UnaryOp){
            UnaryOpAST * cur_branch = (UnaryOpAST *)branch[0];
            if(cur_branch->op == '-'){
                RetVal tmp_ans = branch[1]->Calc_val();
                if(tmp_ans.calcable){
                    ans.calcable = true;
                    ans.value = 0 - tmp_ans.value;
                }
                // ans = 0 - tmp_ans;
            }
            else if(cur_branch->op == '+'){
                ans = branch[1]->Calc_val();
            }
            else if(cur_branch->op == '!'){
                RetVal tmp_ans = branch[1]->Calc_val();
                if(tmp_ans.calcable){
                    ans.calcable = true;
                    ans.value = !tmp_ans.value;
                }
                // ans = !tmp_ans;
            }
        }
        else if(branch[0]->type == _Ident){
            // assert(0);
            std::cout << "Calc_val Error in function calling" << std::endl;
        }
        return ans;
    }
};

class MultiFuncRParamAST : public BaseAST {
    public:
    MultiFuncRParamAST(){
        type = _MultiFuncRParam;
    }
    std::unique_ptr<BaseAST> multi_funcr_param;
    std::unique_ptr<BaseAST> Exp;
    void Dump(std::string& ret_str) const override {
        multi_funcr_param->Dump(ret_str);
        Exp->Calc(ret_str);
    }
    std::vector<std::string> Param(std::string& ret_str) override {
        std::vector<std::string> ans;
        ans.clear();
        std::string tmp_ans;
        if(branch.size() == 0){

        }
        else if(branch[0]->type == _Exp){
            // std::cout << "FUNCRPARAM EXP " << std::endl;
            tmp_ans = branch[0]->Calc(ret_str);
            std::string ans1 = tmp_ans;
            std::cout << "FUNCRPARAM EXP " << tmp_ans << std::endl;
            if(tmp_ans[0] == '@' || (tmp_ans[0] == '%' && (tmp_ans[1] > '9' || tmp_ans[1] < '0'))){
                ans1 = "%" + std::to_string(var_count);
                var_count ++;

                ret_str += "    ";
                ret_str += ans1;
                ret_str += " = ";
                ret_str += "load ";
                ret_str += tmp_ans;
                ret_str += "\n";

                std::cout << "    ";
                std::cout << ans1;
                std::cout << " = ";
                std::cout << "load ";
                std::cout << tmp_ans;
                std::cout << "\n";
            }
            ans.push_back(ans1);
        }
        else if(branch[0]->type == _MultiFuncRParam){
            std::vector<std::string> tmp;
            tmp = multi_funcr_param->Param(ret_str);
            tmp_ans = branch[1]->Calc(ret_str);
            std::string ans1 = tmp_ans;
            if(tmp_ans[0] == '@' || (tmp_ans[0] == '%' && (tmp_ans[1] > '9' || tmp_ans[1] < '0'))){
                ans1 = "%" + std::to_string(var_count);
                var_count ++;

                ret_str += "    ";
                ret_str += ans1;
                ret_str += " = ";
                ret_str += "load ";
                ret_str += tmp_ans;
                ret_str += "\n";

                std::cout << "    ";
                std::cout << ans1;
                std::cout << " = ";
                std::cout << "load ";
                std::cout << tmp_ans;
                std::cout << "\n";
            }
            int tmp_length = tmp.size();
            for(int i = 0; i < tmp_length; ++ i){
                ans.push_back(tmp[i]);
            }
            ans.push_back(ans1);
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

            std::cout << "FIRST: " << tmp1 << std::endl;
            std::cout << (tmp1[0] == '%' && (tmp1[1] > '9' || tmp1[1] < '0')) << std::endl;
            
            if(tmp1[0] == '@' || (tmp1[0] == '%' && (tmp1[1] > '9' || tmp1[1] < '0'))){
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

            if(tmp2[0] == '@' || (tmp2[0] == '%' && (tmp2[1] > '9' || tmp2[1] < '0'))){
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
    RetVal Calc_val() override{
        RetVal ans;
        if(branch[0]->type == _UnaryExp){
            UnaryExpAST* cur_branch = (UnaryExpAST *)branch[0];
            ans = cur_branch->Calc_val();
        }
        else if(branch[0]->type == _MulExp){
            MulExpAST * cur_branch = (MulExpAST *) branch[0];
            if(op == '*'){
                RetVal tmp1, tmp2;
                tmp1 = cur_branch->Calc_val();
                tmp2 =  branch[1]->Calc_val();
                ans.calcable = tmp1.calcable && tmp2.calcable;
                if(ans.calcable){
                    ans.value = tmp1.value * tmp2.value;
                }
            }
            else if(op == '/'){
                RetVal tmp1, tmp2;
                tmp1 = cur_branch->Calc_val();
                tmp2 =  branch[1]->Calc_val();
                ans.calcable = tmp1.calcable && tmp2.calcable;
                if(ans.calcable){
                    ans.value = tmp1.value / tmp2.value;
                }
            }
            else if(op == '%'){
                RetVal tmp1, tmp2;
                tmp1 = cur_branch->Calc_val();
                tmp2 =  branch[1]->Calc_val();
                ans.calcable = tmp1.calcable && tmp2.calcable;
                if(ans.calcable){
                    ans.value = tmp1.value % tmp2.value;
                }
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
            
            if(tmp1[0] == '@' || (tmp1[0] == '%' && (tmp1[1] > '9' || tmp1[1] < '0'))){
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

            if(tmp2[0] == '@' || (tmp2[0] == '%' && (tmp2[1] > '9' || tmp2[1] < '0'))){
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
    RetVal Calc_val() override{
        RetVal ans;
        if(branch[0]->type == _MulExp){
            MulExpAST* cur_branch = (MulExpAST *)branch[0];
            ans = cur_branch->Calc_val();
        }
        else if(branch[0]->type == _AddExp){
            AddExpAST * cur_branch = (AddExpAST *) branch[0];
            if(op == '+'){
                RetVal tmp1, tmp2;
                tmp1 = cur_branch->Calc_val();
                tmp2 =  branch[1]->Calc_val();
                ans.calcable = tmp1.calcable && tmp2.calcable;
                if(ans.calcable){
                    ans.value = tmp1.value + tmp2.value;
                }
                // ans = cur_branch->Calc_val() + branch[1]->Calc_val();
            }
            else if(op == '-'){
                RetVal tmp1, tmp2;
                tmp1 = cur_branch->Calc_val();
                tmp2 =  branch[1]->Calc_val();
                ans.calcable = tmp1.calcable && tmp2.calcable;
                if(ans.calcable){
                    ans.value = tmp1.value - tmp2.value;
                }
                // ans = cur_branch->Calc_val() - branch[1]->Calc_val();
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
            
            if(tmp1[0] == '@' || (tmp1[0] == '%' && (tmp1[1] > '9' || tmp1[1] < '0'))){
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

            if(tmp2[0] == '@' || (tmp2[0] == '%' && (tmp2[1] > '9' || tmp2[1] < '0'))){
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
    RetVal Calc_val() override{
        RetVal ans;
        if(branch[0]->type == _AddExp){
            AddExpAST* cur_branch = (AddExpAST *)branch[0];
            ans = cur_branch->Calc_val();
        }
        else if(branch[0]->type == _RelExp){
            RelExpAST * cur_branch = (RelExpAST *) branch[0];
            if(op == ">="){
                RetVal tmp1, tmp2;
                tmp1 = cur_branch->Calc_val();
                tmp2 =  branch[1]->Calc_val();
                ans.calcable = tmp1.calcable && tmp2.calcable;
                if(ans.calcable){
                    ans.value = tmp1.value >= tmp2.value;
                }
                // ans = cur_branch->Calc_val() >= branch[1]->Calc_val();
            }
            else if(op == "<="){
                RetVal tmp1, tmp2;
                tmp1 = cur_branch->Calc_val();
                tmp2 =  branch[1]->Calc_val();
                ans.calcable = tmp1.calcable && tmp2.calcable;
                if(ans.calcable){
                    ans.value = tmp1.value <= tmp2.value;
                }
                // ans = cur_branch->Calc_val() <= branch[1]->Calc_val();
            }
            else if(op == ">"){
                RetVal tmp1, tmp2;
                tmp1 = cur_branch->Calc_val();
                tmp2 =  branch[1]->Calc_val();
                ans.calcable = tmp1.calcable && tmp2.calcable;
                if(ans.calcable){
                    ans.value = tmp1.value > tmp2.value;
                }
                // ans = cur_branch->Calc_val() > branch[1]->Calc_val();
            }
            else if(op == "<"){
                RetVal tmp1, tmp2;
                tmp1 = cur_branch->Calc_val();
                tmp2 =  branch[1]->Calc_val();
                ans.calcable = tmp1.calcable && tmp2.calcable;
                if(ans.calcable){
                    ans.value = tmp1.value < tmp2.value;
                }
                // ans = cur_branch->Calc_val() < branch[1]->Calc_val();
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
            
            if(tmp1[0] == '@' || (tmp1[0] == '%' && (tmp1[1] > '9' || tmp1[1] < '0'))){
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

            if(tmp2[0] == '@' || (tmp2[0] == '%' && (tmp2[1] > '9' || tmp2[1] < '0'))){
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
    RetVal Calc_val() override{
        RetVal ans;
        if(branch[0]->type == _RelExp){
            RelExpAST* cur_branch = (RelExpAST *)branch[0];
            ans = cur_branch->Calc_val();
        }
        else if(branch[0]->type == _EqExp){
            EqExpAST * cur_branch = (EqExpAST *) branch[0];
            if(op == "=="){
                RetVal tmp1, tmp2;
                tmp1 = cur_branch->Calc_val();
                tmp2 =  branch[1]->Calc_val();
                ans.calcable = tmp1.calcable && tmp2.calcable;
                if(ans.calcable){
                    ans.value = tmp1.value == tmp2.value;
                }
                // ans = cur_branch->Calc_val() == branch[1]->Calc_val();
            }
            else if(op == "!="){
                RetVal tmp1, tmp2;
                tmp1 = cur_branch->Calc_val();
                tmp2 =  branch[1]->Calc_val();
                ans.calcable = tmp1.calcable && tmp2.calcable;
                if(ans.calcable){
                    ans.value = tmp1.value != tmp2.value;
                }
                // ans = cur_branch->Calc_val() != branch[1]->Calc_val();
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

            // calc a
            tmp1 = branch[0]->Calc(ret_str);
            ans1 = tmp1;

            if(tmp1[0] == '@' || (tmp1[0] == '%' && (tmp1[1] > '9' || tmp1[1] < '0'))){
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

            // calc a != 0
            std::string neqa;

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

            // 6.2 check (a != 0)
            // 这个语句一定在matched_stmt内部，因此不会出现需要考虑ret的情况
            std::string tmp_ans = "@temp_result_" + std::to_string(tmp_result_count);
            tmp_result_count ++;
            
            ret_str += "    ";
            ret_str += tmp_ans;
            ret_str += " = ";
            ret_str += "alloc i32\n";

            std::cout << "    ";
            std::cout << tmp_ans;
            std::cout << " = ";
            std::cout << "alloc i32\n";
            
            ret_str += "    store 0, ";
            ret_str += tmp_ans;
            ret_str += "\n";

            std::cout << "    store 0, ";
            std::cout << tmp_ans;
            std::cout << "\n";

            std::string tmp_lhs = "%" + std::to_string(var_count);
            var_count ++;
            std::string then = "%then_" + std::to_string(if_count);
            std::string end = "%end_" + std::to_string(end_count);
            if_count ++;
            end_count ++;

            ret_str += "    ";
            ret_str += tmp_lhs;
            ret_str += " = ";
            ret_str += "ne  0, ";
            ret_str += neqa;
            ret_str += "\n";

            std::cout << "    ";
            std::cout << tmp_lhs;
            std::cout << " = ";
            std::cout << "ne  0, ";
            std::cout << neqa;
            std::cout << "\n";

            ret_str += "    br  ";
            ret_str += tmp_lhs;
            ret_str += ", ";
            ret_str += then;
            ret_str += ", ";
            ret_str += end;
            ret_str += "\n";

            std::cout << "  br  ";
            std::cout << tmp_lhs;
            std::cout << ", ";
            std::cout << then;
            std::cout << ", ";
            std::cout << end;
            std::cout << "\n";

            ret_str += then;
            ret_str += ":\n";

            std::cout << then;
            std::cout << ":\n";

            tmp2 = branch[1]->Calc(ret_str);
            ans2 = tmp2;
            
            if(tmp2[0] == '@' || (tmp2[0] == '%' && (tmp2[1] > '9' || tmp2[1] < '0'))){
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

            // calc (b != 0)
            std::string neqb;

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

            std::string tmp_rhs = "%" + std::to_string(var_count);
            var_count ++;

            ret_str += "    ";
            ret_str += tmp_rhs;
            ret_str += " = ";
            ret_str += "ne  0, ";
            ret_str += neqb;
            ret_str += "\n";

            std::cout << "    ";
            std::cout << tmp_rhs;
            std::cout << " = ";
            std::cout << "ne  0, ";
            std::cout << neqb;
            std::cout << "\n";

            ret_str += "    store ";
            ret_str += tmp_rhs;
            ret_str += ", ";
            ret_str += tmp_ans;
            ret_str += "\n";

            std::cout << "    store ";
            std::cout << tmp_rhs;
            std::cout << ", ";
            std::cout << tmp_ans;
            std::cout << "\n";

            ret_str += "    jump ";
            ret_str += end;
            ret_str += "\n";

            std::cout << "    jump ";
            std::cout <<  end;
            std::cout << "\n";

            ret_str += end;
            ret_str += ":\n";

            std::cout << end;
            std::cout << ":\n";

            ans = "%" + std::to_string(var_count);
            var_count ++;
            
            ret_str += "    ";
            ret_str += ans;
            ret_str += " = ";
            ret_str += "load ";
            ret_str += tmp_ans;
            ret_str += "\n";

            std::cout << "    ";
            std::cout << ans;
            std::cout << " = ";
            std::cout << "load ";
            std::cout << tmp_ans;
            std::cout << "\n";

            /*
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
            */
        }
        return ans;
    }
    RetVal Calc_val() override{
        RetVal ans;
        if(branch[0]->type == _EqExp){
            EqExpAST* cur_branch = (EqExpAST *)branch[0];
            ans = cur_branch->Calc_val();
        }
        else if(branch[0]->type == _LAndExp){
            LAndExpAST * cur_branch = (LAndExpAST *) branch[0];
            RetVal tmp1, tmp2;
            tmp1 = cur_branch->Calc_val();
            ans.calcable = tmp1.calcable;
            ans.value = 0;
            if(ans.calcable && tmp1.value){
                tmp2 = branch[1]->Calc_val();
                ans.calcable = ans.calcable && tmp2.calcable;
                if(tmp2.calcable)
                    ans.value = (tmp2.value != 0);
            }
            /*int tmp1 = cur_branch->Calc_val();
            if(tmp1 != 0){
                int tmp2 = branch[1]->Calc_val();
                ans = (tmp2 != 0);
            }*/
            // ans = cur_branch->Calc_val() && branch[1]->Calc_val();
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

            // calc a
            tmp1 = branch[0]->Calc(ret_str);
            ans1 = tmp1;

            if(tmp1[0] == '@' || (tmp1[0] == '%' && (tmp1[1] > '9' || tmp1[1] < '0'))){
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

            // calc !(a == 0)
            std::string eqa;
            std::string na;

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

            // 6.2 check !(a == 0) first
            std::string tmp_ans = "@temp_result_" + std::to_string(tmp_result_count);
            tmp_result_count ++;
            
            ret_str += "    ";
            ret_str += tmp_ans;
            ret_str += " = ";
            ret_str += "alloc i32\n";

            std::cout << "    ";
            std::cout << tmp_ans;
            std::cout << " = ";
            std::cout << "alloc i32\n";
            
            ret_str += "    store 1, ";
            ret_str += tmp_ans;
            ret_str += "\n";

            std::cout << "    store 1, ";
            std::cout << tmp_ans;
            std::cout << "\n";

            std::string tmp_lhs = "%" + std::to_string(var_count);
            var_count ++;
            std::string then = "%then_" + std::to_string(if_count);
            std::string end = "%end_" + std::to_string(end_count);
            if_count ++;
            end_count ++;

            ret_str += "    ";
            ret_str += tmp_lhs;
            ret_str += " = ";
            ret_str += "eq  0, ";
            ret_str += na;
            ret_str += "\n";

            std::cout << "    ";
            std::cout << tmp_lhs;
            std::cout << " = ";
            std::cout << "eq  0, ";
            std::cout << na;
            std::cout << "\n";

            ret_str += "    br  ";
            ret_str += tmp_lhs;
            ret_str += ", ";
            ret_str += then;
            ret_str += ", ";
            ret_str += end;
            ret_str += "\n";

            std::cout << "  br  ";
            std::cout << tmp_lhs;
            std::cout << ", ";
            std::cout << then;
            std::cout << ", ";
            std::cout << end;
            std::cout << "\n";

            ret_str += then;
            ret_str += ":\n";

            std::cout << then;
            std::cout << ":\n";

            // calc b
            tmp2 = branch[1]->Calc(ret_str);
            ans2 = tmp2;
            
            if(tmp2[0] == '@' || (tmp2[0] == '%' && (tmp2[1] > '9' || tmp2[1] < '0'))){
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

            // calc !(b == 0)
            std::string eqb;
            std::string nb;

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

            std::string tmp_rhs = "%" + std::to_string(var_count);
            var_count ++;

            ret_str += "    ";
            ret_str += tmp_rhs;
            ret_str += " = ";
            ret_str += "ne  0, ";
            ret_str += nb;
            ret_str += "\n";

            std::cout << "    ";
            std::cout << tmp_rhs;
            std::cout << " = ";
            std::cout << "ne  0, ";
            std::cout << nb;
            std::cout << "\n";

            ret_str += "    store ";
            ret_str += tmp_rhs;
            ret_str += ", ";
            ret_str += tmp_ans;
            ret_str += "\n";

            std::cout << "    store ";
            std::cout << tmp_rhs;
            std::cout << ", ";
            std::cout << tmp_ans;
            std::cout << "\n";

            ret_str += "    jump ";
            ret_str += end;
            ret_str += "\n";

            std::cout << "    jump ";
            std::cout <<  end;
            std::cout << "\n";

            ret_str += end;
            ret_str += ":\n";

            std::cout << end;
            std::cout << ":\n";

            ans = "%" + std::to_string(var_count);
            var_count ++;
            
            ret_str += "    ";
            ret_str += ans;
            ret_str += " = ";
            ret_str += "load ";
            ret_str += tmp_ans;
            ret_str += "\n";

            std::cout << "    ";
            std::cout << ans;
            std::cout << " = ";
            std::cout << "load ";
            std::cout << tmp_ans;
            std::cout << "\n";

            /*
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
            */
        }
        return ans;
    }
    RetVal Calc_val() override{
        RetVal ans;
        if(branch[0]->type == _LAndExp){
            LAndExpAST* cur_branch = (LAndExpAST *)branch[0];
            ans = cur_branch->Calc_val();
        }
        else if(branch[0]->type == _LOrExp){
            LOrExpAST * cur_branch = (LOrExpAST *) branch[0];
            RetVal tmp1, tmp2;
            tmp1 = cur_branch->Calc_val();
            ans.calcable = tmp1.calcable;
            ans.value = 1;
            if(ans.calcable && (tmp1.value == 0)){
                tmp2 = branch[1]->Calc_val();
                ans.calcable = ans.calcable && tmp2.calcable;
                if(tmp2.calcable)
                    ans.value = (tmp2.value != 0);
            }
            /*
            ans = 1;
            int tmp1 = cur_branch->Calc_val();
            if(tmp1 == 0){
                int tmp2 = branch[1]->Calc_val();
                ans = (tmp2 != 0);
            }
            */
            // ans = cur_branch->Calc_val() || branch[1]->Calc_val();
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
        std::cout << "DECL" << std::endl;
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
        std::cout << "CONSTDECL" << std::endl;
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
        std::cout << "VARDECL" << std::endl;
        btype->Dump(ret_str);
        multi_var_def->Dump(ret_str);
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
            std::cout << "SINGLEVARDEF" << std::endl;
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
    std::unique_ptr<BaseAST> const_exp;
    void Dump(std::string& ret_str) const override{
        std::cout << "CONSTDEF" << std::endl;
        if(branch.size() == 2){ // 数
            std::string const_ans;
            RetVal ans = const_init_val->Calc_val();
            Symbol symb;
            if(ans.calcable){
                symb.type = _CONST;
                symb.sym_val = ans.value;
            }
            else{
                symb.type = _CONST_UNCALC;
            }

            // 如果当前函数名为空，那么这是一个全局定义
            if(cur_func_name.empty()){
                symb.type = _CONST_GLOBAL;
                global_var_table[ident] = symb;
            }
            else{
                cur_table->sym_table[ident] = symb;
            }
        }
        else if(branch.size() == 3){ // 数组
            std::string const_ans;
            std::vector<int> array_init_vec;
            int length = 0;
            bool zero_init = false;
            RetVal len = const_exp->Calc_val();
            RetVal ans = const_init_val->Calc_val();
            Symbol symb;
            if(ans.calcable){
                symb.type = _CONST_ARRAY;
                symb.array_val = ans.array_value;
                array_init_vec = ans.array_value;
                length = len.value;
            }
            else {
                std::cout << "CONST ARRAY INIT WRONG - CONSTDEF" << std::endl;
            }
            if(ans.array_value.size() == 0){
                zero_init = true;
            }
            
            // 补0
            int extra_zero = 0;
            if(array_init_vec.size() < length){
                extra_zero = length - array_init_vec.size();
            }
            for(int i = 0; i < extra_zero; ++ i){
                array_init_vec.push_back(0);
            }
            symb.array_val = array_init_vec;

            ret_str += "    ";
            std::cout << "  ";

            // 如果当前函数名为空，那么这是一个全局定义
            if(cur_func_name.empty()){
                symb.type = _CONST_GLOBAL_ARRAY;
                global_var_table[ident] = symb;
                const_ans = "@" + ident + "_" + "global";

                ret_str += "global ";
                std::cout << "global ";
            }
            else{
                cur_table->sym_table[ident] = symb;
                int depth = cur_table->table_index;
                std::string inside_func = cur_table->func_name;
                const_ans = "@" + ident + "_" + inside_func + "_" + std::to_string(depth);
            }

            ret_str += const_ans;
            ret_str += " = alloc [i32, ";
            ret_str += std::to_string(length);
            ret_str += "]";

            std::cout << const_ans;
            std::cout << " = alloc [i32, ";
            std::cout << std::to_string(length);
            std::cout << "]";

            if(symb.type == _CONST_GLOBAL_ARRAY){
                if(zero_init){
                    ret_str += ", zeroinit\n";
                    std::cout << ", zeroinit\n";
                }
                else{
                    ret_str += ", {";
                    std::cout << ", {";

                    if(array_init_vec.size()){
                        ret_str += std::to_string(array_init_vec[0]);
                        std::cout << std::to_string(array_init_vec[0]);
                    }
                    for(int i = 1; i < array_init_vec.size(); ++ i){
                        ret_str += ", ";
                        std::cout << ", ";
                        ret_str += std::to_string(array_init_vec[i]);
                        std::cout << std::to_string(array_init_vec[i]);
                    }
                    ret_str += "}\n";
                    std::cout << "}\n";
                    }
            }
            else if(symb.type == _CONST_ARRAY){
                ret_str += "\n";
                std::cout << "\n";
                for(int i = 0; i < length; ++ i){
                    std::string init_index = "%" + std::to_string(var_count);
                    var_count ++;
                    ret_str += "    ";
                    ret_str += init_index;
                    ret_str += " = ";
                    ret_str += "getelemptr ";
                    ret_str += const_ans;
                    ret_str += ", ";
                    ret_str += std::to_string(i);
                    ret_str += "\n    store ";
                    ret_str += std::to_string(array_init_vec[i]);
                    ret_str += ", ";
                    ret_str += init_index;
                    ret_str += "\n";

                    std::cout << "    ";
                    std::cout << init_index;
                    std::cout << " = ";
                    std::cout << "getelemptr ";
                    std::cout << const_ans;
                    std::cout << ", ";
                    std::cout << std::to_string(i);
                    std::cout << "\n    store ";
                    std::cout << std::to_string(array_init_vec[i]);
                    std::cout << ", ";
                    std::cout << init_index;
                    std::cout << "\n";
                }
            }

        }
    }
};

// 没有实际作用，只是用于判断分支
class IdentAST : public BaseAST {
    public:
    IdentAST(){
        type = _Ident;
    }
    std::string ident;
    void Dump(std::string& ret_str) const override {

    }
};

// 没有实际作用，只是用于判断分支
class ReturnAST : public BaseAST {
    public:
    ReturnAST(){
        type = _Return;
    }
    std::string ret;
    void Dump(std::string& ret_str) const override {

    }
};

// 没有实际作用，只是用于判断分支
class BreakAST : public BaseAST {
    public:
    BreakAST() {
        type = _Break;
    }
    std::string break_str;
    void Dump(std::string& ret_str) const override {

    }
};

// 没有实际作用，只是用于判断分支
class ContinueAST : public BaseAST {
    public:
    ContinueAST() {
        type = _Continue;
    }
    std::string continue_str;
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
    std::unique_ptr<BaseAST> const_exp;
    void Dump(std::string& ret_str) const override{
        std::cout << "VARDEF" << std::endl;
        if(branch.size() == 1){
            Symbol symb;
            std::string ans;

            // 也有可能是GLOBAL类型
            if(cur_func_name.empty()){
                symb.type = _NUM_GLOBAL;
                symb.sym_val = 0;
                global_var_table[ident] = symb;
                ans = "@" + ident + "_" + "global";
            }
            else {
                symb.type = _UNK;
                symb.sym_val = 0;
                cur_table->sym_table[ident] = symb;
                int depth = cur_table->table_index;
                std::string inside_func = cur_table->func_name;
                ans = "@" + ident + "_" + inside_func + "_" + std::to_string(depth);
            }
            
            ret_str += "    ";
            std::cout << "    ";
            
            if(symb.type == _NUM_GLOBAL){
                ret_str += "global ";
                std::cout << "global ";
            }
            
            ret_str += ans;
            ret_str += " = alloc i32";

            std::cout << ans;
            std::cout << " = alloc i32";

            if(symb.type == _NUM_GLOBAL){
                ret_str += ", ";
                ret_str += "zeroinit";

                std::cout << ", ";
                std::cout << "zeroinit";
            }
            
            ret_str += "\n";
            std::cout << "\n";
        }
        else if(branch.size() == 2 && branch[1]->type == _InitVal){
            std::string ans;
            std::string val;
            RetVal value;
            value = init_val->Calc_val();

            Symbol symb;

            if(cur_func_name.empty()){
                symb.type = _NUM_GLOBAL;
                symb.sym_val = value.value;
                global_var_table[ident] = symb;
                ans = "@" + ident + "_" + "global";
            }
            else{
                if(value.calcable){
                    symb.type = _NUM;
                    symb.sym_val = value.value;
                }
                else{
                    symb.type = _NUM_UNCALC;
                }
                cur_table->sym_table[ident] = symb;
                int depth = cur_table->table_index;
                std::string inside_func = cur_table->func_name;
                ans = "@" + ident + "_" + inside_func + "_" + std::to_string(depth);
            }
            
            val = init_val->Calc(ret_str); // 可能是立即数，也可能是%0

            ret_str += "    ";
            std::cout << "    ";
            
            if(symb.type == _NUM_GLOBAL){
                ret_str += "global ";
                std::cout << "global ";
            }
        
            ret_str += ans;
            ret_str += " = alloc i32";

            std::cout << ans;
            std::cout << " = alloc i32";

            if(symb.type == _NUM_GLOBAL){
                ret_str += ", ";
                ret_str += std::to_string(value.value);

                std::cout << ", ";
                std::cout << std::to_string(value.value);
            }

            ret_str += "\n";
            std::cout << "\n";
            
            if(symb.type == _NUM  || symb.type == _NUM_UNCALC){
                ret_str += "    store ";
                ret_str += val;
                ret_str += ", ";
                ret_str += ans;
                ret_str += "\n";

                std::cout << "  store ";
                std::cout << val;
                std::cout << ", ";
                std::cout << ans;
                std::cout << "\n";
            }
        }
        else if(branch.size() == 3){ // 数组自定义初始化
            std::cout << "VERDEF ARRAY WITH INIT VALUE" << std::endl;
            std::string ans;
            std::vector<std::string> val;
            RetVal value, len;
            int length = 0;
            len = const_exp->Calc_val();
            value = init_val->Calc_val();
            std::cout << "VARDEF VALUE SUCCESS" << std::endl;
            length = len.value;

            Symbol symb;
            if(cur_func_name.empty()){
                symb.type = _GLOBAL_ARRAY;
                symb.array_val = value.array_value;
                global_var_table[ident] = symb;
                ans = "@" + ident + "_" + "global";
            }
            else{
                if(value.calcable){
                    symb.type = _ARRAY;
                    symb.array_val = value.array_value;
                    int extra_zero = 0;
                    if(symb.array_val.size() < length){
                        extra_zero = length - symb.array_val.size();
                    }
                    for(int i = 0; i < extra_zero; ++ i){
                        symb.array_val.push_back(0);
                    }
                }
                else{
                    symb.type = _ARRAY_UNCALC;
                }
                cur_table->sym_table[ident] = symb;
                int depth = cur_table->table_index;
                std::string inside_func = cur_table->func_name;
                ans = "@" + ident + "_" + inside_func + "_" + std::to_string(depth);
            }
            std::cout << "VERDEF ARRAY STORE VALUE" << std::endl;
            val = init_val->ArrayInit(ret_str);
            std::cout << "VERDEF ARRAY INIT CALC" << val.size() << std::endl;
            for(int i = 0; i < val.size(); ++ i){
                std::cout << "val i" << val[i] << std::endl;
            }
            std::vector<std::string> array_init_vec;
            array_init_vec = val;
            
            // 补0
            int extra_zero = 0;
            if(array_init_vec.size() < length){
                extra_zero = length - array_init_vec.size();
            }
            for(int i = 0; i < extra_zero; ++ i){
                array_init_vec.push_back(std::to_string(0));
            }
            // symb.array_val = array_init_vec;

            ret_str += "    ";
            std::cout << "    ";
            
            if(symb.type == _GLOBAL_ARRAY){
                ret_str += "global ";
                std::cout << "global ";
            }
        
            ret_str += ans;
            ret_str += " = alloc [i32, ";
            ret_str += std::to_string(length);
            ret_str += "]";

            std::cout << ans;
            std::cout << " = alloc [i32, ";
            std::cout << std::to_string(length);
            std::cout << "]";

            if(symb.type == _GLOBAL_ARRAY){

                // global 用数字初始化
                ret_str += ", {";
                std::cout << ", {";

                if(length){
                    ret_str += std::to_string(symb.array_val[0]);
                    std::cout << std::to_string(symb.array_val[0]);
                }
                for(int i = 1; i < length; ++ i){
                    ret_str += ", ";
                    std::cout << ", ";
                    ret_str += std::to_string(symb.array_val[i]);
                    std::cout << std::to_string(symb.array_val[i]);
                }

                ret_str += "}";
                std::cout << "}";
            }

            ret_str += "\n";
            std::cout << "\n";
            
            if(symb.type == _ARRAY  || symb.type == _ARRAY_UNCALC){
                for(int i = 0; i < length; ++ i){
                    std::string init_index = "%" + std::to_string(var_count);
                    var_count ++;
                    ret_str += "    ";
                    ret_str += init_index;
                    ret_str += " = ";
                    ret_str += "getelemptr ";
                    ret_str += ans;
                    ret_str += ", ";
                    ret_str += std::to_string(i);
                    ret_str += "\n    store ";
                    ret_str += array_init_vec[i];
                    ret_str += ", ";
                    ret_str += init_index;
                    ret_str += "\n";

                    std::cout << "    ";
                    std::cout << init_index;
                    std::cout << " = ";
                    std::cout << "getelemptr ";
                    std::cout << ans;
                    std::cout << ", ";
                    std::cout << std::to_string(i);
                    std::cout << "\n    store ";
                    std::cout << array_init_vec[i];
                    std::cout << ", ";
                    std::cout << init_index;
                    std::cout << "\n";
                }
            }
        }
        else if(branch.size() == 2 && branch[1]->type == _ConstExp){ // 数组全0初始化
            std::string ans;
            std::vector<std::string> val;
            std::vector<int> val_int;
            val.clear();
            val_int.clear();
            RetVal value, len;
            int length = 0;
            len = const_exp->Calc_val();
            length = len.value;
            Symbol symb;

            for(int i = 0; i < length; ++ i){
                val.push_back(std::to_string(0));
                val_int.push_back(0);
            }

            if(cur_func_name.empty()){
                symb.type = _GLOBAL_ARRAY;
                symb.array_val = val_int;
                global_var_table[ident] = symb;
                ans = "@" + ident + "_" + "global";
            }
            else{
                symb.type = _ARRAY;
                symb.array_val = val_int;
                cur_table->sym_table[ident] = symb;
                int depth = cur_table->table_index;
                std::string inside_func = cur_table->func_name;
                ans = "@" + ident + "_" + inside_func + "_" + std::to_string(depth);
            }

            ret_str += "    ";
            std::cout << "    ";
            
            if(symb.type == _GLOBAL_ARRAY){
                ret_str += "global ";
                std::cout << "global ";
            }
        
            ret_str += ans;
            ret_str += " = alloc [i32, ";
            ret_str += std::to_string(length);
            ret_str += "]";

            std::cout << ans;
            std::cout << " = alloc [i32, ";
            std::cout << std::to_string(length);
            std::cout << "]";

            if(symb.type == _GLOBAL_ARRAY){
                ret_str += ", zeroinit";
                std::cout << ", zeroinit";
                /*// global 用数字初始化
                ret_str += ", {";
                std::cout << ", {";

                if(length){
                    ret_str += std::to_string(symb.array_val[0]);
                    std::cout << std::to_string(symb.array_val[0]);
                }
                for(int i = 1; i < length; ++ i){
                    ret_str += ", ";
                    std::cout << ", ";
                    ret_str += std::to_string(symb.array_val[i]);
                    std::cout << std::to_string(symb.array_val[i]);
                }

                ret_str += "}";
                std::cout << "}";*/ 
            }

            ret_str += "\n";
            std::cout << "\n";
            
            if(symb.type == _ARRAY  || symb.type == _ARRAY_UNCALC){
                for(int i = 0; i < length; ++ i){
                    std::string init_index = "%" + std::to_string(var_count);
                    var_count ++;
                    ret_str += "    ";
                    ret_str += init_index;
                    ret_str += " = ";
                    ret_str += "getelemptr ";
                    ret_str += ans;
                    ret_str += ", ";
                    ret_str += std::to_string(i);
                    ret_str += "\n";
                    ret_str += "    store ";
                    ret_str += val[i];
                    ret_str += ", ";
                    ret_str += init_index;
                    ret_str += "\n";

                    std::cout << "    ";
                    std::cout << init_index;
                    std::cout << " = ";
                    std::cout << "getelemptr ";
                    std::cout << ans;
                    std::cout << ", ";
                    std::cout << std::to_string(i);
                    std::cout << "\n";
                    std::cout << "    store ";
                    std::cout << val[i];
                    std::cout << ", ";
                    std::cout << init_index;
                    std::cout << "\n";
                }
            }
        }
    }
};

class ConstInitValAST : public BaseAST{
    public:
    ConstInitValAST(){
        type = _ConstInitVal;
    }
    std::unique_ptr<BaseAST> const_exp;
    std::unique_ptr<BaseAST> multi_const_array_init_val;
    void Dump(std::string& ret_str) const override{

    }
    RetVal Calc_val() override{
        RetVal ans;
        std::vector<int> ret_array_val;
        ret_array_val.clear();
        if(branch[0]->type == _ConstExp){
            ans = const_exp->Calc_val();
        }
        if(branch[0]->type == _MultiConstArrayInitVal){
            ans = multi_const_array_init_val->Calc_val();
        }
        return ans;
    }
};

class MultiConstArrayInitValAST : public BaseAST {
    public:
    MultiConstArrayInitValAST(){
        type = _MultiConstArrayInitVal;
    }
    std::unique_ptr<BaseAST> const_exp;
    std::unique_ptr<BaseAST> multi_const_array_init_val;
    void Dump(std::string& ret_str) const override {

    }
    RetVal Calc_val() override{
        std::cout << "MULTI CONST ARRAY INITVAL" << std::endl;
        RetVal ans;
        std::vector<int> ret_array_val;
        ret_array_val.clear();
        if(branch.size() == 0){ // 0初始化
            ans.calcable = true;
            // ret_array_val.push_back(0);
            ans.array_value = ret_array_val;
        }
        else if(branch[0]->type == _ConstExp){
            RetVal tmp_ans;
            tmp_ans = const_exp->Calc_val();
            ans.calcable = tmp_ans.calcable;
            ret_array_val.push_back(tmp_ans.value);
            ans.array_value = ret_array_val;
        }
        else if(branch[0]->type == _MultiConstArrayInitVal){
            RetVal tmp_ans = multi_const_array_init_val->Calc_val();
            RetVal exp_ans = const_exp->Calc_val();
            ans.calcable = tmp_ans.calcable && exp_ans.calcable;
            if(ans.calcable){
                int array_len = tmp_ans.array_value.size();
                for(int i = 0; i < array_len; ++ i){
                    ret_array_val.push_back(tmp_ans.array_value[i]);
                }
                ret_array_val.push_back(exp_ans.value);
                ans.array_value = ret_array_val;
            }
            else{
                std::cout << "CONST ARRAY INIT WRONG" << std::endl;
            }
        }
        return ans;
    }
};

class InitValAST : public BaseAST{
    public:
    InitValAST(){
        type =_InitVal;
    }
    std::unique_ptr<BaseAST> Exp;
    std::unique_ptr<BaseAST> multi_array_init_val;
    void Dump(std::string& ret_str) const override{

    }
    std::string Calc(std::string& ret_str) override{
        std::string ans;
        if(branch[0]->type == _Exp){
            ans = Exp->Calc(ret_str);
        }
        else if(branch[0]->type == _MultiArrayInitVal){
            std::cout << "WRONG ARRAY INNITVAL BRANCH" << std::endl;
        }
        return ans;
    }
    std::vector<std::string> ArrayInit(std::string& ret_str) override{
        std::cout << "ARRAYINIT" << int(branch[0]->type == _MultiArrayInitVal) << std::endl;
        std::vector<std::string> ans;
        if(branch.size() == 0){
            ans.clear();
        }
        else if(branch[0]->type == _Exp){
            std::string tmp_ans;
            tmp_ans = Exp->Calc(ret_str);
            ans.push_back(tmp_ans);
        }
        else if(branch[0]->type == _MultiArrayInitVal){
            std::cout << "MULTIARRAYINITVAL" << std::endl;
            ans = multi_array_init_val->ArrayInit(ret_str);
        }
        return ans;
    }
    RetVal Calc_val() override{
        RetVal ans;
        std::cout << int(branch[0]->type == _MultiArrayInitVal) << std::endl;
        if(branch[0]->type == _Exp){
            ans = Exp->Calc_val();
        }
        else if(branch[0]->type == _MultiArrayInitVal){
            std::cout << "INITVAL MULTIARRAY CALCVAL" << std::endl;
            ans = multi_array_init_val->Calc_val();
            std::cout << "INITVAL FINISH" << std::endl;
        }
        return ans;
    }
};

class MultiArrayInitValAST : public BaseAST {
    public:
    MultiArrayInitValAST(){
        type = _MultiArrayInitVal;
    }
    std::unique_ptr<BaseAST> Exp;
    std::unique_ptr<BaseAST> multi_array_init_val;
    void Dump(std::string& ret_str) const override {

    }
    std::string Calc(std::string& ret_str) override {
        std::cout << "ARRAY INIT WRONG BRANCH - MULTIARRAYINITVALAST" << std::endl;
        return "";
    }
    std::vector<std::string> ArrayInit(std::string& ret_str) override{
        std::vector<std::string> ans;
        if(branch.size() == 0){
            ans.push_back("0");
        }
        else if(branch[0]->type == _Exp){
            std::string tmp_ans;
            tmp_ans = Exp->Calc(ret_str);
            ans.push_back(tmp_ans);
        }
        else if(branch[0]->type == _MultiArrayInitVal){
            std::string tmp_ans;
            ans = multi_array_init_val->ArrayInit(ret_str);
            tmp_ans = Exp->Calc(ret_str);
            ans.push_back(tmp_ans);
        }
        return ans;
    }
    RetVal Calc_val() override {
        RetVal ans;
        if(branch.size() == 0){
            std::cout << "MULTIARRAY INITVAL NULL" << std::endl;
            ans.calcable = true;
            ans.array_value.clear();
            std::cout << "MULTIARRAY INITVAL FINISH" << std::endl;
        }
        else if(branch[0]->type == _Exp){
            std::cout << "MULTIARRAY INITVAL EXP" << std::endl;
            RetVal exp_ans = Exp->Calc_val();
            ans.calcable = exp_ans.calcable;
            ans.array_value.push_back(exp_ans.value);
        }
        else if(branch[0]->type == _MultiArrayInitVal){
            std::cout << "MULTIARRAY INITVAL CALCVAL" << std::endl;
            RetVal exp_ans;
            ans = multi_array_init_val->Calc_val();
            std::cout << "MULTIARRAY FINISH MULTIINIT" << std::endl;
            exp_ans = Exp->Calc_val();
            std::cout << "MULTIARRAY FINISH EXP" << std::endl;
            ans.calcable = ans.calcable && exp_ans.calcable;
            std::cout << "CALCABLE" << std::endl;
            ans.array_value.push_back(exp_ans.value);
            std::cout << "FINISH MULTIARRAY INITVAL" << std::endl;
            for(int i = 0; i < ans.array_value.size(); ++ i){
                std::cout << ans.array_value[i] << std::endl;
            }
        }
        std::cout << ans.calcable << std::endl;
        return ans;
    }
};

class ConstExpAST : public BaseAST{
    public:
    ConstExpAST(){
        type = _ConstExp;
    }
    std::unique_ptr<BaseAST> Exp;
    void Dump(std::string& ret_str) const override{

    }
    RetVal Calc_val() override{
        return Exp->Calc_val();
    }
};


