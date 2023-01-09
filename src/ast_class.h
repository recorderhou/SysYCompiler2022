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

struct SymbolTable{
    SymTable sym_table;
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

typedef std::vector<SymbolTable> SymTableList;

extern SymTable sym_table;
extern SymTableList sym_table_list;
extern int block_count;
extern SymbolTable* cur_table;
extern int end_count;
extern int if_count;
extern int else_count;


enum TYPE{
  _UnaryExp, _PrimaryExp, _UnaryOp, _Number, \
  _Exp, _MulExp, _AddExp, _RelExp, _EqExp, _LAndExp, _LOrExp, \
  _MultiConstDef, _ConstDef, _MultiBlockItem, _BlockItem, _Decl, _Stmt, \
  _LVal, _ConstDecl, _VarDecl, _VarDef, _MultiVarDef, _Ident, _Return, _Block, \
  _OpenStmt, _MatchedStmt, 
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

    ret_str += " { \n";
    ret_str += "%entry:\n";
    std::cout << " {" << std::endl;
    std::cout << "%entry:" << std::endl;

    block->Dump(ret_str);

    ret_str += "\n}";
    std::cout << std::endl << "}";
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
        symbol_table.parent = cur_table;
        symbol_table.returned = false;
        symbol_table.blocking = false;
        //将符号表改为当前符号表（之前的符号表一定是这个block的parent）
        cur_table = &symbol_table;
        sym_table_list.push_back(symbol_table);

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
        sym_table_list.pop_back();
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
    void Dump(std::string& ret_str) const override{

    }
    std::string Calc(std::string& ret_str) override{
        std::cout << "LVALCALC "  << ident << std::endl;
        int depth = cur_table->table_index;
        SymbolTable *used_table = cur_table;

        // 看看ident到底是哪儿定义的
        while(used_table->sym_table.find(ident) == used_table->sym_table.end()){
            used_table = used_table->parent;
        }
        if(used_table == NULL)
            std::cout << "LVAL-SYMTABLE WRONG" << std::endl;
        depth = used_table->table_index;
        Symbol tmp_ans = used_table->sym_table[ident];

        std::string ans;
        if(tmp_ans.type == _CONST){
            ans = std::to_string(tmp_ans.sym_val);
        }
        else if(tmp_ans.type == _NUM || tmp_ans.type == _UNK){
            std::cout << "NUM OR UNK" << std::endl;
            ans = "@" + ident + "_" + std::to_string(depth);
        }
        return ans;
    }
    int Calc_val() override{
        std::cout << "LVALCALC" << std::endl;

        SymbolTable *used_table = cur_table;
        // 看看ident到底是哪儿定义的
        while(used_table->sym_table.find(ident) == used_table->sym_table.end()){
            used_table = used_table->parent;
        }
        Symbol tmp_ans = used_table->sym_table[ident];
        // Symbol tmp_ans = cur_table->sym_table[ident];
        int ans = 0;
        if(tmp_ans.type != _STR){
            ans = tmp_ans.sym_val;
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
        if(branch[0]->type == _MatchedStmt){
            matched_stmt->Dump(ret_str);
        }
        else if(branch[0]->type == _OpenStmt){
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
            if(tmp1[0] == '@'){
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
            symbol_table_if_then.parent = cur_table;
            symbol_table_if_then.returned = false;
            symbol_table_if_then.blocking = true; //if的{}是blocking的，在if内部return并不影响在main和else中return
            //将符号表改为当前符号表（之前的符号表一定是这个block的parent）
            cur_table = &symbol_table_if_then;
            sym_table_list.push_back(symbol_table_if_then);

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
            sym_table_list.pop_back();

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
            if(tmp1[0] == '@'){
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
            symbol_table_if.parent = cur_table;
            symbol_table_if.returned = false;
            symbol_table_if.blocking = true; //if的{}是blocking的，在if内部return并不影响在main和else中return
            //将符号表改为当前符号表（之前的符号表一定是这个block的parent）
            cur_table = &symbol_table_if;
            sym_table_list.push_back(symbol_table_if);

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
            sym_table_list.pop_back();
            /* } */
            
            /* else {*/
            // 初始化else分支的符号表
            block_count ++;
            SymbolTable symbol_table_else;
            symbol_table_else.table_index = block_count;
            symbol_table_else.parent = cur_table;
            symbol_table_else.returned = false;
            symbol_table_else.blocking = true; //if的{}是blocking的，在if内部return并不影响在main和else中return
            //将符号表改为当前符号表（之前的符号表一定是这个block的parent）
            cur_table = &symbol_table_else;
            sym_table_list.push_back(symbol_table_else);

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
            sym_table_list.pop_back();
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
    void Dump(std::string& ret_str) const override {
        std::cout << "STMT" << std::endl;
        if(branch.size() == 0){
            return;
        }
        else if (branch[0]->type == _Return){
            std::cout << "STMTRETEXP" << std::endl;
            std::string ans;
            ans = Exp->Calc(ret_str);
            std::cout << "AAA" << std::endl;
            if(branch.size() == 1){
                ret_str += "    ret ";
                std::cout << "   ret ";
            }
            else if(branch.size() == 2){
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
        else if (branch[0]->type == _LVal){
            std::cout << "STMTLVAL" << std::endl;
            LValAST * cur_branch = (LValAST *)branch[0];
            std::string tmp1;
            std::string ans1;
            std::string ans2;
            std::string var_name = cur_branch->ident;
            int value;
            value = cur_branch->Calc_val();

            // int depth = cur_table->table_index;
            // cur table 会不会变？
            SymbolTable* used_table = cur_table;

            // 看看var_name到底是哪儿定义的
            while(used_table->sym_table.find(var_name) == used_table->sym_table.end()){
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
            used_table->sym_table[var_name] = symb; // 存到对应的symtable中
            
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

                if(tmp1[0] == '@'){
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
                symbol_table_if.parent = cur_table;
                symbol_table_if.returned = false;
                symbol_table_if.blocking = true; //if的{}是blocking的，在if内部return并不影响在main和else中return
                //将符号表改为当前符号表（之前的符号表一定是这个block的parent）
                cur_table = &symbol_table_if;
                sym_table_list.push_back(symbol_table_if);

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
                sym_table_list.pop_back();

                /* } */

                /* else { */
                // 初始化else分支的符号表
                block_count ++;
                SymbolTable symbol_table_else;
                symbol_table_else.table_index = block_count;
                symbol_table_else.parent = cur_table;
                symbol_table_else.returned = false;
                symbol_table_else.blocking = true; //if的{}是blocking的，在if内部return并不影响在main和else中return
                //将符号表改为当前符号表（之前的符号表一定是这个block的parent）
                cur_table = &symbol_table_else;
                sym_table_list.push_back(symbol_table_else);

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
                sym_table_list.pop_back();

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
        cur_table->sym_table[ident] = symb;
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
            cur_table->sym_table[ident] = symb;
            int depth = cur_table->table_index;

            std::string ans;
            ans = "@" + ident + "_" + std::to_string(depth);

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
            cur_table->sym_table[ident] = symb;
            int depth = cur_table->table_index;
            // init_val->Dump(ret_str);

            ans = "@" + ident + "_" + std::to_string(depth);
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


