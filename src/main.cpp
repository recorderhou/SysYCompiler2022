#include "koopa.h"
#include "ast_class.h"
#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <cstring>
#include <stdio.h>
#include <map>

using namespace std;

// 声明 lexer 的输入, 以及 parser 函数
// 为什么不引用 sysy.tab.hpp 呢? 因为首先里面没有 yyin 的定义
// 其次, 因为这个文件不是我们自己写的, 而是被 Bison 生成出来的
// 你的代码编辑器/IDE 很可能找不到这个文件, 然后会给你报错 (虽然编译不会出错)
// 看起来会很烦人, 于是干脆采用这种看起来 dirty 但实际很有效的手段
extern FILE *yyin;
extern FILE *yyout;
extern int yyparse(unique_ptr<BaseAST> &ast);
int var_count = 0;
int register_count = 0; 
int block_count = 0;
int end_count = 0;
int if_count = 0;
int else_count = 0;
int tmp_result_count = 0; // 用于短路求值
int while_count = 0;
int prev_cur_while = 0;
int cur_while = 0;
int cur_while_end;
bool break_continue = false;

Symbol symm;
SymTable sym_table;
FuncTable func_table;
GlobalVarTable global_var_table;
SymTableList sym_table_list;
ProgramSymTableList program_sym_table;
SymbolTable* cur_table = NULL;
SymTableList* cur_sym_table_list;
std::string cur_func_name = "default_global";

map<koopa_raw_function_t, int> func_sp; // 记录每个函数占据栈的大小
map<koopa_raw_value_t, int> inst_sp; // 记录栈中每条指令的位置
map<koopa_raw_function_t, int> func_ra; // 记录每个函数要不要保存ra
map<koopa_raw_function_t, int> func_param; // 记录每个函数需要为函数调用的参数开辟的最大空间
koopa_raw_function_t cur_func;

std::string riscv_str;

// 函数声明略
// ...
void Visit(const koopa_raw_program_t &program);
void Visit(const koopa_raw_slice_t &slice);
void Visit(const koopa_raw_function_t &func);
void Visit(const koopa_raw_basic_block_t &bb);
void Visit(const koopa_raw_value_t &value);
void Visit(const koopa_raw_return_t &ret);
void Visit(const koopa_raw_integer_t &integer);
void Visit(const koopa_raw_binary_t &binary);
void Visit(const koopa_raw_store_t &store);
void Visit(const koopa_raw_load_t &load);
void Visit(const koopa_raw_branch_t &branch);
void Visit(const koopa_raw_jump_t &jump);
void Visit(const koopa_raw_call_t &call);
void Visit(const koopa_raw_func_arg_ref_t &func_arg_ref);
void Visit(const koopa_raw_global_alloc_t &global_alloc);

void Prologue(const koopa_raw_function_t &func);
void Epilogue(const koopa_raw_function_t &func);

void li_lw(const koopa_raw_value_t &value, string dest_reg);
void sw(const koopa_raw_value_t &dest, string src_reg);

void Prologue(koopa_raw_function_t &func){
  int sp = func_sp[func];
  if(sp){
    cout << " addi  sp, sp, " + to_string(-sp) << endl;
  }
}

void Epilogue(koopa_raw_function_t &func){
  int sp = func_sp[func];
  int ra = func_ra[func];
  if(sp){
    if(ra){
      cout << " lw ra, " + to_string(sp-4) + "(sp)\n";
    }
    cout << " addi  sp, sp, " + to_string(sp) << endl;
  }
}

void li_lw(const koopa_raw_value_t &value, string dest_reg="t0"){
  if(inst_sp.find(value) == inst_sp.end()){
    if(value->kind.tag == KOOPA_RVT_INTEGER){
      int number;
      number = value->kind.data.integer.value;
      cout << " li " << dest_reg << ", " << to_string(number) << endl;
    }
    else if(value->kind.tag == KOOPA_RVT_GLOBAL_ALLOC){
      string var_name = value->name;
      cout << " la " << dest_reg << ", ";
      for(int i = 1; i < var_name.size(); ++ i){
        cout << var_name[i];
      }
      cout << endl;
      cout << " lw " << dest_reg << ", " << "0(" << dest_reg << ")" << endl;
    }
  }
  else{ // 不是立即数，不是全局变量，%0是指针指向对应指令
    int cur_inst_sp = inst_sp[value];
    cout << " lw " << dest_reg << ", " << to_string(cur_inst_sp) << "(sp)" << endl;
  }
}

void sw(const koopa_raw_value_t &dest, string src_reg="t0"){
  if(inst_sp.find(dest) == inst_sp.end()){
    if(dest->kind.tag == KOOPA_RVT_GLOBAL_ALLOC){
      string var_name = dest->name;
      // 要修改？
      cout << " la t6, ";
      for(int i = 1; i < var_name.size(); ++ i){
        cout << var_name[i];
      }
      cout << endl;
      cout << " sw " << src_reg << ", " << "0(t6)" << endl;
    }
  }
  else{
    int cur_inst_sp = inst_sp[dest];
    cout << " sw " << src_reg << ", " << to_string(cur_inst_sp) << "(sp)" << endl;
  }
}

// 访问 raw program
void Visit(const koopa_raw_program_t &program) {
  riscv_str.clear();
  // 执行一些其他的必要操作
  cout << " .data " << endl;
  cout << " .globl ";

  riscv_str += " .data \n";
  riscv_str += " .globl ";

  for(size_t i = 0; i < program.values.len; ++ i){
    assert(program.values.kind == KOOPA_RSIK_VALUE);
    koopa_raw_value_t value = (koopa_raw_value_t) program.values.buffer[i];
    for(int i = 1; i < strlen(value->name); ++ i){
      cout << value->name[i];
      riscv_str += value->name[i];
    }
    cout << endl;
    riscv_str += "\n";
    for(int i = 1; i < strlen(value->name); ++ i){
      cout << value->name[i];
      riscv_str += value->name[i];
    }
    cout << ":" << endl;
    Visit(value);
  }
  cout << endl;
  for(size_t i = 0; i < program.funcs.len; ++ i){
    assert(program.funcs.kind == KOOPA_RSIK_FUNCTION);
    koopa_raw_function_t func = (koopa_raw_function_t) program.funcs.buffer[i];
    if(func->bbs.len == 0){
      continue;
    }
    Visit(func);
  }
}

// 访问 raw slice
void Visit(const koopa_raw_slice_t &slice) {
  for (size_t i = 0; i < slice.len; ++i) {
    auto ptr = slice.buffer[i];
    // 根据 slice 的 kind 决定将 ptr 视作何种元素
    switch (slice.kind) {
      case KOOPA_RSIK_FUNCTION:
        // 访问函数
        Visit(reinterpret_cast<koopa_raw_function_t>(ptr));
        break;
      case KOOPA_RSIK_BASIC_BLOCK:
        // 访问基本块
        Visit(reinterpret_cast<koopa_raw_basic_block_t>(ptr));
        break;
      case KOOPA_RSIK_VALUE:
        // 访问指令
        Visit(reinterpret_cast<koopa_raw_value_t>(ptr));
        break;
      default:
        // 我们暂时不会遇到其他内容, 于是不对其做任何处理
        assert(false);
    }
  }
}

// 访问函数
void Visit(const koopa_raw_function_t &func) {
  // 执行一些其他的必要操作
  // ...
  // cout << "function " << func->name << "visited" << endl;
  int cur_sp = 0;
  int cur_ra = 0;
  int cur_param = 0; // caller save 的部分，自己调用别人
  int cur_param_size = 0;

  if(func->bbs.len == 0){
    return;
  }
  cout << "\n .text" << endl;
  cout << " .globl " ;

  for(int i = 1; i < strlen(func->name); ++ i){
    cout << func->name[i];
  }
  cout << endl;
  for(int i = 1; i < strlen(func->name); ++ i){
    cout << func->name[i];
  }
  cout << ":" << endl;
  for(size_t i = 0; i < func->bbs.len; ++ i){
    koopa_raw_basic_block_t bb = (koopa_raw_basic_block_t) func->bbs.buffer[i];
    for(size_t j = 0; j < bb->insts.len; ++ j){
      koopa_raw_value_t inst = (koopa_raw_value_t) bb->insts.buffer[j];
      if(inst->ty->tag != KOOPA_RTT_UNIT) {
        inst_sp[inst] = cur_sp;
        cur_sp += 4;
      }
      if(inst->kind.tag == KOOPA_RVT_CALL){
        func_ra[func] = 4;
        cur_ra = 4;
        int callee_args = inst->kind.data.call.args.len;
        cur_param = max(cur_param, callee_args);
      }
    }
  }
  cur_param_size = max(cur_param_size, (cur_param - 8) * 4);
  cur_sp += cur_param_size;
  cur_sp += cur_ra;
  if(cur_sp % 16){
    cur_sp = int((cur_sp / 16) + 1) * 16;
  }
  func_sp[func] = cur_sp;

  for(size_t i = 0; i < func->bbs.len; ++ i){
    koopa_raw_basic_block_t bb = (koopa_raw_basic_block_t) func->bbs.buffer[i];
    for(size_t j = 0; j < bb->insts.len; ++ j){
      koopa_raw_value_t inst = (koopa_raw_value_t) bb->insts.buffer[j];
      if(inst->ty->tag != KOOPA_RTT_UNIT) {
        inst_sp[inst] += cur_param_size;
      }
    }
  }

  koopa_raw_function_t prev_func = cur_func;
  cur_func = func;
  if(cur_sp){
    cout << " addi  sp, sp, " + to_string(-cur_sp) << endl;
    if(cur_ra){
      cout << " sw ra, " + to_string(cur_sp-4) + "(sp)\n" << endl;
    }
  }
  for(size_t i = 0; i < func->params.len; ++ i){
    // cout << "----------" << endl;
    // cout << func->params.kind << endl;
    koopa_raw_value_t arg = (koopa_raw_value_t) func->params.buffer[i];
    Visit(arg);
  }
  for(size_t i = 0; i < func->bbs.len; ++ i){
    assert(func->bbs.kind == KOOPA_RSIK_BASIC_BLOCK);
    koopa_raw_basic_block_t bb = (koopa_raw_basic_block_t) func->bbs.buffer[i];
    Visit(bb);
  }
}

// 访问基本块
void Visit(const koopa_raw_basic_block_t &bb) {
  // 执行一些其他的必要操作
  // ...
  // 访问所有指令
  // string bb_name = new string(bb->name);
  int str_length = strlen(bb->name);
  string bbname = bb->name;
  if(bbname != "%entry"){
    for(int i = 1; i < str_length; ++ i){
      cout << bb->name[i];
    }
    cout << ":" << endl;
  }
  for(size_t i = 0; i < bb->insts.len; ++ i){
    koopa_raw_value_t inst = (koopa_raw_value_t) bb->insts.buffer[i];
    Visit(inst);
  }
  // Visit(bb->insts);
}

// 访问指令
void Visit(const koopa_raw_value_t &value) {
  // 根据指令类型判断后续需要如何访问
  const auto &kind = value->kind;
  switch (kind.tag) {
    case KOOPA_RVT_RETURN:
      // 访问 return 指令
      Visit(kind.data.ret);
      break;
    case KOOPA_RVT_INTEGER:
      // 访问 integer 指令
      cout << "INTEGER" << endl;
      Visit(kind.data.integer);
      break;
    case KOOPA_RVT_BINARY:
      // 运算
      Visit(kind.data.binary);
      sw(value, "t0");
      break;
    case KOOPA_RVT_STORE:
      // store
      Visit(kind.data.store);
      break;
    case KOOPA_RVT_LOAD:
      // load
      Visit(kind.data.load);
      sw(value, "t0");
      break;
    case KOOPA_RVT_BRANCH:
      // branch
      // cout << "in branch" << endl;
      Visit(kind.data.branch);
      break;
    case KOOPA_RVT_JUMP:
      // jump
      Visit(kind.data.jump);
      break;
    case KOOPA_RVT_ALLOC:
      // alloc
      // Visit(kind.data.alloc);
      break;
    case KOOPA_RVT_CALL:
      // call
      Visit(kind.data.call);
      sw(value, "a0");
      break;
    case KOOPA_RVT_FUNC_ARG_REF:
      Visit(kind.data.func_arg_ref);
      break;
    case KOOPA_RVT_GLOBAL_ALLOC:
      Visit(kind.data.global_alloc);
      break;
    case KOOPA_RVT_ZERO_INIT:
      break;
    default:
      // 其他类型暂时遇不到
      assert(false);
  }
}

void Visit(const koopa_raw_call_t &call){
  koopa_raw_slice_t call_args = call.args;
  koopa_raw_function_t callee = call.callee;
  int args_num = call_args.len;
  for(int i = 0; i < args_num; ++ i){
    if(i < 8){
      string dest_reg = "a" + to_string(i);
      koopa_raw_value_t cur_arg = (koopa_raw_value_t) call_args.buffer[i];
      li_lw(cur_arg, dest_reg);
    }
    else{
      string dest_stack = to_string((i - 8) * 4) + "(sp)";
      koopa_raw_value_t cur_arg = (koopa_raw_value_t) call_args.buffer[i];
      li_lw(cur_arg, "t0");
      cout << " sw t0, " << dest_stack << endl;
    }
  }
  cout << " call ";
  int str_len = strlen(callee->name);
  for(int i = 1; i < str_len; ++ i){
    cout << callee->name[i];
  }
  cout << endl;
}
void Visit(const koopa_raw_func_arg_ref_t &func_arg_ref){

}
void Visit(const koopa_raw_global_alloc_t &global_alloc){
  koopa_raw_value_t init = global_alloc.init;
  if(init->kind.tag == KOOPA_RVT_INTEGER){
    cout << " .word ";
    cout << to_string(init->kind.data.integer.value) << endl;
  }else if(init->kind.tag == KOOPA_RVT_ZERO_INIT){
    cout << " .zero ";
    cout << to_string(4) << endl;
  }
}

// 访问对应类型指令的函数定义略
// 视需求自行实现
void Visit(const koopa_raw_return_t &ret){
  koopa_raw_value_t value = ret.value;
  if(value){
    li_lw(value, "a0");
  }
  // 这里之前出过错
  // cout << ret.value->kind.data.integer.value << endl;
  Epilogue(cur_func);
  cout << " ret" << endl;
}

void Visit(const koopa_raw_branch_t &branch) {
  koopa_raw_value_t cond = branch.cond;
  koopa_raw_basic_block_t true_bb = branch.true_bb;
  koopa_raw_basic_block_t false_bb = branch.false_bb;
  int true_length = strlen(true_bb->name);
  int false_length = strlen(false_bb->name);
  li_lw(cond, "t0");
  cout << " bnez t0, ";
  for(int i = 1; i < true_length; ++ i){
    cout << true_bb->name[i];
  }
  cout << endl;
  cout << " j  ";
  for(int i = 1; i < false_length; ++ i){
    cout << false_bb->name[i];
  }
  cout << "\n";
}

void Visit(const koopa_raw_jump_t &jump) {
  koopa_raw_basic_block_t target = jump.target;
  int str_length = strlen(target->name);
  cout << " j  ";
  for(int i = 1; i < str_length; ++ i){
    cout << target->name[i];
  }
  cout << endl;
}

// op, lhs, rhs
void Visit(const koopa_raw_binary_t &binary) {
  // cout << "li   t0, " << binary.lhs->kind.data.integer.value << endl;
  // cout << "li   t1, " << binary.rhs->kind.data.integer.value << endl;
  li_lw(binary.lhs, "t0");
  li_lw(binary.rhs, "t1");
  if(binary.op == KOOPA_RBO_EQ){
    cout << " xor  t0, t0, t1" << endl;
    cout << " seqz t0, t0" << endl;
  }
  else if(binary.op == KOOPA_RBO_NOT_EQ){
    cout << " xor  t0, t0, t1" << endl;
    cout << " snez t0, t0" << endl;
  }
  else if(binary.op == KOOPA_RBO_GT){
    cout << " sgt  t0, t0, t1" << endl;
  }
  else if(binary.op == KOOPA_RBO_LT){
    cout << " slt  t0, t0, t1" << endl;
  }
  else if(binary.op == KOOPA_RBO_GE){
    cout << " slt  t0, t0, t1" << endl;
    cout << " seqz  t0, t0" << endl;
  }
  else if(binary.op == KOOPA_RBO_LE){
    cout << " sgt  t0, t0, t1" << endl;
    cout << " seqz  t0, t0" << endl;
  }
  else if(binary.op == KOOPA_RBO_ADD){
    cout << " add  t0, t0, t1" << endl;
  }
  else if(binary.op == KOOPA_RBO_SUB){
    cout << " sub  t0, t0, t1" << endl;
  }
  else if(binary.op == KOOPA_RBO_MUL){
    cout << " mul  t0, t0, t1" << endl;
  }
  else if(binary.op == KOOPA_RBO_DIV){
    cout << " div  t0, t0, t1" << endl;
  }
  else if(binary.op == KOOPA_RBO_MOD){
    cout << " rem  t0, t0, t1" << endl;
  }
  else if(binary.op == KOOPA_RBO_AND){
    cout << " and  t0, t0, t1" << endl;
  }
  else if(binary.op == KOOPA_RBO_OR){
    cout << " or  t0, t0, t1" << endl;
  }
  // cout << binary.lhs->kind.tag << endl;
  // cout << "binary_lhs_ty" << binary.lhs->ty << endl;
  // cout << "bbinary_lhs_ty" << binary.rhs->ty << endl;
}

void Visit(const koopa_raw_store_t &store){
  koopa_raw_value_t value = store.value;
  koopa_raw_value_t dest = store.dest;
  li_lw(value, "t0");
  sw(dest, "t0");
}

void Visit(const koopa_raw_load_t &load){
  koopa_raw_value_t src = load.src;
  li_lw(src, "t0");
}

void Visit(const koopa_raw_integer_t &integer){
  cout << integer.value << endl;
}

int main(int argc, const char *argv[]) {
  // 解析命令行参数. 测试脚本/评测平台要求你的编译器能接收如下参数:
  // compiler 模式 输入文件 -o 输出文件
  assert(argc == 5);
  auto mode = argv[1];
  auto input = argv[2];
  auto output = argv[4];

  cout << mode << endl;

  // 打开输入文件, 并且指定 lexer 在解析的时候读取这个文件
  yyin = fopen(input, "r");
  assert(yyin);
  yyout = fopen(output, "w");

  // 调用 parser 函数, parser 函数会进一步调用 lexer 解析输入文件的
  unique_ptr<BaseAST> ast;
  auto rett = yyparse(ast);
  // assert(!rett);

  string ret_str;
  // 输出解析得到的 AST, 其实就是个字符串
  ast->Dump(ret_str);
  cout << endl;

  freopen(output, "w", stdout);
  cout << ret_str << endl;
  fclose(stdout);

  const char* str = ret_str.c_str();
  // 解析字符串 str, 得到 Koopa IR 程序
  koopa_program_t program;
  koopa_error_code_t ret = koopa_parse_from_string(str, &program);
  assert(ret == KOOPA_EC_SUCCESS);  // 确保解析时没有出错
  // 创建一个 raw program builder, 用来构建 raw program
  koopa_raw_program_builder_t builder = koopa_new_raw_program_builder();
  // 将 Koopa IR 程序转换为 raw program
  koopa_raw_program_t raw = koopa_build_raw_program(builder, program);
  // 释放 Koopa IR 程序占用的内存
  koopa_delete_program(program);

  if(strcmp(mode, "-koopa") == 0){
    freopen(output, "w", stdout);
    cout << ret_str << endl;
    fclose(stdout);
  }

  else if (strcmp(mode, "-riscv") == 0){
    // 处理 raw program
    freopen(output, "w", stdout);
    Visit(raw);
    fclose(stdout);
  }

  // 处理完成, 释放 raw program builder 占用的内存
  // 注意, raw program 中所有的指针指向的内存均为 raw program builder 的内存
  // 所以不要在 raw program 处理完毕之前释放 builder
  koopa_delete_raw_program_builder(builder);
  // cout << endl;
  return 0;
}
