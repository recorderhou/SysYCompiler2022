%code requires {
  #include <memory>
  #include <string>
}

%{

#include "ast_class.h"
#include <iostream>
#include <memory>
#include <string>

// 声明 lexer 函数和错误处理函数
int yylex();
void yyerror(std::unique_ptr<BaseAST> &ast, const char *s);

using namespace std;

%}

// 定义 parser 函数和错误处理函数的附加参数
// 我们需要返回一个字符串作为 AST, 所以我们把附加参数定义成字符串的智能指针
// 解析完成后, 我们要手动修改这个参数, 把它设置成解析得到的字符串
%parse-param { std::unique_ptr<BaseAST> &ast }

// yylval 的定义, 我们把它定义成了一个联合体 (union)
// 因为 token 的值有的是字符串指针, 有的是整数
// 之前我们在 lexer 中用到的 str_val 和 int_val 就是在这里被定义的
// 至于为什么要用字符串指针而不直接用 string 或者 unique_ptr<string>?
// 请自行 STFW 在 union 里写一个带析构函数的类会出现什么情况
%union {
  std::string *str_val;
  int int_val;
  class BaseAST *ast_val;
}

// lexer 返回的所有 token 种类的声明
// 注意 IDENT 和 INT_CONST 会返回 token 的值, 分别对应 str_val 和 int_val
%token INT RETURN
%token <str_val> IDENT CONST LEQ GEQ NEQ EQ AND OR
%token <int_val> INT_CONST

// 非终结符的类型定义
%type <ast_val> FuncDef FuncType Block Stmt Number Exp UnaryExp PrimaryExp UnaryOp 
%type <ast_val> MulExp AddExp LOrExp LAndExp RelExp EqExp
%type <ast_val> Decl ConstDecl BType MultiConstDef ConstDef ConstInitVal ConstExp MultiBlockItem BlockItem LVal 
%type <ast_val> VarDecl MultiVarDef VarDef InitVal
// %type <int_val> Number
%%

// 开始符, CompUnit ::= FuncDef, 大括号后声明了解析完成后 parser 要做的事情
// 之前我们定义了 FuncDef 会返回一个 str_val, 也就是字符串指针
// 而 parser 一旦解析完 CompUnit, 就说明所有的 token 都被解析了, 即解析结束了
// 此时我们应该把 FuncDef 返回的结果收集起来, 作为 AST 传给调用 parser 的函数
// $1 指代规则里第一个符号的返回值, 也就是 FuncDef 的返回值
CompUnit
  : FuncDef {
    auto comp_unit = make_unique<CompUnitAST>();
    comp_unit->func_def = unique_ptr<BaseAST>($1);
    ast = move(comp_unit);
  }
  | error {
    printf("error CompUnit, unbelievable\n");
  }
  ;

// FuncDef ::= FuncType IDENT '(' ')' Block;
// 我们这里可以直接写 '(' 和 ')', 因为之前在 lexer 里已经处理了单个字符的情况
// 解析完成后, 把这些符号的结果收集起来, 然后拼成一个新的字符串, 作为结果返回
// $$ 表示非终结符的返回值, 我们可以通过给这个符号赋值的方法来返回结果
// 你可能会问, FuncType, IDENT 之类的结果已经是字符串指针了
// 为什么还要用 unique_ptr 接住它们, 然后再解引用, 把它们拼成另一个字符串指针呢
// 因为所有的字符串指针都是我们 new 出来的, new 出来的内存一定要 delete
// 否则会发生内存泄漏, 而 unique_ptr 这种智能指针可以自动帮我们 delete
// 虽然此处你看不出用 unique_ptr 和手动 delete 的区别, 但当我们定义了 AST 之后
// 这种写法会省下很多内存管理的负担
FuncDef
  : FuncType IDENT '(' ')' Block {
    auto ast = new FuncDefAST();
    ast->func_type = unique_ptr<BaseAST>($1);
    ast->ident = *unique_ptr<string>($2);
    ast->block = unique_ptr<BaseAST>($5);
    $$ = ast;
  }
  | error {
    printf("error FuncDef, unbelievable\n");
  }
  ;

// 同上, 不再解释
FuncType
  : INT {
    auto ast = new FuncTypeAST();
    ast->func_type_str = string("int");
    $$ = ast;
  }
  | error {
    printf("error FuncType, unbelievable\n");
  }
  ;

Block
  : '{' MultiBlockItem '}' {
    auto ast = new BlockAST();
    ast->multi_block_item = unique_ptr<BaseAST>($2);
    ast->branch.push_back($2);
    $$ = ast;
  }
  | error {
    printf("error Block, unbelievable\n");
  }
  ;

MultiBlockItem
  :  /* NULL */ {
    printf("in null\n");
    auto ast = new MultiBlockItemAST();
    $$ = ast;
  }
  | BlockItem {
    printf("in single\n");
    auto ast = new MultiBlockItemAST();
    ast->block_item = unique_ptr<BaseAST>($1);
    ast->branch.push_back($1);
    $$ = ast;
  }
  | MultiBlockItem BlockItem {
    printf("in multi\n");
    auto ast = new MultiBlockItemAST();
    ast->multi_block_item = unique_ptr<BaseAST>($1);
    ast->block_item = unique_ptr<BaseAST>($2);
    ast->branch.push_back($1);
    ast->branch.push_back($2);
    $$ = ast;
  }
  | error {
    printf("error MultiBlockItem, unbelievable\n");
  }
  ;

BlockItem
  : Decl {
    printf("in decl\n");
    auto ast = new BlockItemAST();
    ast->decl = unique_ptr<BaseAST>($1);
    ast->branch.push_back($1);
    $$ = ast;
  }
  | Stmt {
    printf("in stmt\n");
    auto ast = new BlockItemAST();
    ast->stmt = unique_ptr<BaseAST>($1);
    ast->branch.push_back($1);
    $$ = ast;
  }
  | error {
    printf("error BlockItem, unbelievable\n");
  }
  ;

Decl
  : ConstDecl {
    printf("in constdecl\n");
    auto ast = new DeclAST();
    ast->const_decl = unique_ptr<BaseAST>($1);
    ast->branch.push_back($1);
    $$ = ast;
  }
  | VarDecl {
    auto ast = new DeclAST();
    ast->var_decl = unique_ptr<BaseAST>($1);
    ast->branch.push_back($1);
    $$ = ast;
  }
  | error { 
    printf("error Decl, unbelievable\n");
  }
  ;

ConstDecl
  : CONST BType MultiConstDef ';' {
    printf("in constdecl\n");
    auto ast = new ConstDeclAST();
    ast->btype = unique_ptr<BaseAST>($2);
    ast->multi_const_def = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | error {
    printf("error ConstDecl, unbelievable\n");
  }
  ;

VarDecl
  : BType MultiVarDef ';' {
    auto ast = new VarDeclAST();
    ast->btype = unique_ptr<BaseAST>($1);
    ast->multi_var_def = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  ;

BType
  : INT {
    printf("in btype\n");
    auto ast = new BTypeAST();
    ast->btype_name = "int";
    $$ = ast;
  }
  ;

MultiConstDef
  : ConstDef {
    printf("in single constdef\n");
    auto ast = new MultiConstDefAST();
    ast->const_def = unique_ptr<BaseAST>($1);
    ast->branch.push_back($1);
    $$ = ast;
  }
  | MultiConstDef ',' ConstDef {
    printf("in multi constdef\n");
    auto ast = new MultiConstDefAST();
    ast->multi_const_def = unique_ptr<BaseAST>($1);
    ast->const_def = unique_ptr<BaseAST>($3);
    ast->branch.push_back($1);
    ast->branch.push_back($3);
    $$ = ast;
  }
  | error {
    printf("error in Multiconstdef\n");
  }
  ;

MultiVarDef
  : VarDef {
    auto ast = new MultiVarDefAST();
    ast->var_def = unique_ptr<BaseAST>($1);
    ast->branch.push_back($1);
    $$ = ast;
  }
  | MultiVarDef ',' VarDef {
    auto ast = new MultiVarDefAST();
    ast->multi_var_def = unique_ptr<BaseAST>($1);
    ast->var_def = unique_ptr<BaseAST>($3);
    ast->branch.push_back($1);
    ast->branch.push_back($3);
    $$ = ast;
  }
  ;

ConstDef
  : IDENT '=' ConstInitVal {
    printf("in constdef\n");
    auto ast = new ConstDefAST();
    ast->ident = *unique_ptr<string>($1);
    ast->const_init_val = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | error {
    printf("error in constdef\n");
  }
  ;

VarDef
  : IDENT {
    auto ast = new VarDefAST();
    ast->ident = *unique_ptr<string>($1);
    auto ident_ast = new IdentAST();
    ident_ast->ident = ast->ident;
    ast->branch.push_back(ident_ast);
    $$ = ast;
  }
  | IDENT '=' InitVal {
    auto ast = new VarDefAST();
    ast->ident = *unique_ptr<string>($1);
    ast->init_val = unique_ptr<BaseAST>($3);
    auto ident_ast = new IdentAST();
    ident_ast->ident = ast->ident;
    ast->branch.push_back(ident_ast);
    ast->branch.push_back($3);
    $$ = ast;
  }
  ;

ConstInitVal
  : ConstExp{
    printf("in constinitval\n");
    auto ast = new ConstInitValAST();
    ast->const_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

InitVal
  : Exp {
    auto ast = new InitValAST();
    ast->Exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

ConstExp
  : Exp {
    printf("in constexp\n");
    auto ast = new ConstExpAST();
    ast->Exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

Stmt
  : LVal '=' Exp ';' {
    auto ast = new StmtAST();
    ast->LVal = unique_ptr<BaseAST>($1);
    ast->Exp = unique_ptr<BaseAST>($3);
    ast->branch.push_back($1);
    ast->branch.push_back($3);
    $$ = ast;
  }
  | RETURN Exp ';' {
    auto ast = new StmtAST();
    ast->Exp = unique_ptr<BaseAST>($2);
    ast->branch.push_back($2);
    $$ = ast;
  }
  | error {
    printf("error Stmt, unbelievable\n");
  }
  ;

Exp
  : LOrExp {
    printf("in exp\n");
    auto ast = new ExpAST();
    ast->lor_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | error {
    printf("error Exp, unbelievable\n");
  }
  ;

UnaryExp
  : PrimaryExp {
    printf("in primary\n");
    auto ast = new UnaryExpAST();
    ast->primary_exp = unique_ptr<BaseAST>($1);
    // what's inside the $1?
    cout << ast->primary_exp->type << endl;
    ast->branch.push_back($1);
    $$ = ast;
  }
  | UnaryOp UnaryExp{
    auto ast = new UnaryExpAST();
    ast->unary_op = unique_ptr<BaseAST>($1);
    ast->unary_exp = unique_ptr<BaseAST>($2);
    ast->branch.push_back($1);
    ast->branch.push_back($2);
    $$ = ast;
  }
  ;

LVal
  : IDENT {
    auto ast = new LValAST();
    ast->ident = *unique_ptr<string>($1);
    cout << ast->ident << endl;
    $$ = ast;
  }
  ;

PrimaryExp
  : '(' Exp ')' {
    auto ast = new PrimaryExpAST();
    ast->Exp = unique_ptr<BaseAST>($2);
    ast->branch.push_back($2);
    $$ = ast;
  }
  | LVal {
    printf("in lval\n");
    auto ast = new PrimaryExpAST();
    ast->LVal = unique_ptr<BaseAST>($1);
    cout << ast->type << endl;
    ast->branch.push_back($1);
    $$ = ast;
  }
  | Number {
    auto ast = new PrimaryExpAST();
    ast->number = unique_ptr<BaseAST>($1);
    ast->branch.push_back($1);
    $$ = ast;
  }
  ;

UnaryOp
  : '+'{
    auto ast = new UnaryOpAST();
    ast->op = '+';
    $$ = ast;
  }
  | '-' {
    auto ast = new UnaryOpAST();
    ast->op = '-';
    $$ = ast;
  }
  | '!' {
    auto ast = new UnaryOpAST();
    ast->op = '!';
    $$ = ast;
  }
  ;

Number
  : INT_CONST {
    printf("in number\n");
    auto ast = new NumberAST();
    ast->number = $1;
    $$ = ast;
  }
  ;

MulExp
  : UnaryExp{
    printf("in unary\n"); 
    auto ast = new MulExpAST();
    ast->unary_exp = unique_ptr<BaseAST>($1);
    ast->branch.push_back($1);
    $$ = ast;
  }
  | MulExp '*' UnaryExp {
    auto ast = new MulExpAST();
    ast->mul_exp = unique_ptr<BaseAST>($1);
    ast->op = '*';
    ast->unary_exp = unique_ptr<BaseAST>($3);
    ast->branch.push_back($1);
    ast->branch.push_back($3);
    $$ = ast;
  }
  | MulExp '/' UnaryExp {
    auto ast = new MulExpAST();
    ast->mul_exp = unique_ptr<BaseAST>($1);
    ast->op = '/';
    ast->unary_exp = unique_ptr<BaseAST>($3);
    ast->branch.push_back($1);
    ast->branch.push_back($3);
    $$ = ast;
  }
  | MulExp '%' UnaryExp {
    auto ast = new MulExpAST();
    ast->mul_exp = unique_ptr<BaseAST>($1);
    ast->op = '%';
    ast->unary_exp = unique_ptr<BaseAST>($3);
    ast->branch.push_back($1);
    ast->branch.push_back($3);
    $$ = ast;
  }
  ;

AddExp
  : MulExp {
    printf("in mul\n");
    auto ast = new AddExpAST();
    ast->mul_exp = unique_ptr<BaseAST>($1);
    ast->branch.push_back($1);
    $$ = ast;
  }
  | AddExp '+' MulExp {
    auto ast = new AddExpAST();
    ast->add_exp = unique_ptr<BaseAST>($1);
    ast->op = '+';
    ast->mul_exp = unique_ptr<BaseAST>($3);
    ast->branch.push_back($1);
    ast->branch.push_back($3);
    $$ = ast;
  }
  | AddExp '-' MulExp {
    auto ast = new AddExpAST();
    ast->add_exp = unique_ptr<BaseAST>($1);
    ast->op = '-';
    ast->mul_exp = unique_ptr<BaseAST>($3);
    ast->branch.push_back($1);
    ast->branch.push_back($3);
    $$ = ast;
  }
  ;

RelExp
  : AddExp {
    printf("in add\n");
    auto ast = new RelExpAST();
    ast->add_exp = unique_ptr<BaseAST>($1);
    ast->branch.push_back($1);
    $$ = ast;
  }
  | RelExp '<' AddExp {
    auto ast = new RelExpAST();
    ast->rel_exp = unique_ptr<BaseAST>($1);
    ast->op = "<";
    ast->add_exp = unique_ptr<BaseAST>($3);
    ast->branch.push_back($1);
    ast->branch.push_back($3);
    $$ = ast;
  }
  | RelExp '>' AddExp {
    auto ast = new RelExpAST();
    ast->rel_exp = unique_ptr<BaseAST>($1);
    ast->op = ">";
    ast->add_exp = unique_ptr<BaseAST>($3);
    ast->branch.push_back($1);
    ast->branch.push_back($3);
    $$ = ast;
  }
  | RelExp LEQ AddExp {
    auto ast = new RelExpAST();
    ast->rel_exp = unique_ptr<BaseAST>($1);
    ast->op = "<=";
    ast->add_exp = unique_ptr<BaseAST>($3);
    ast->branch.push_back($1);
    ast->branch.push_back($3);
    $$ = ast;
  }
  | RelExp GEQ AddExp {
    auto ast = new RelExpAST();
    ast->rel_exp = unique_ptr<BaseAST>($1);
    ast->op = ">=";
    ast->add_exp = unique_ptr<BaseAST>($3);
    ast->branch.push_back($1);
    ast->branch.push_back($3);
    $$ = ast;
  }
  ;

EqExp
  : RelExp {
    printf("in rel\n");
    auto ast = new EqExpAST();
    ast->rel_exp = unique_ptr<BaseAST>($1);
    ast->branch.push_back($1);
    $$ = ast;
  }
  | EqExp EQ RelExp {
    auto ast = new EqExpAST();
    ast->eq_exp = unique_ptr<BaseAST>($1);
    ast->op = "==";
    ast->rel_exp = unique_ptr<BaseAST>($3);
    ast->branch.push_back($1);
    ast->branch.push_back($3);
    $$ = ast;
  }
  | EqExp NEQ RelExp {
    auto ast = new EqExpAST();
    ast->eq_exp = unique_ptr<BaseAST>($1);
    ast->op = "!=";
    ast->rel_exp = unique_ptr<BaseAST>($3);
    ast->branch.push_back($1);
    ast->branch.push_back($3);
    $$ = ast;
  }
  ;

LAndExp
  : EqExp {
    printf("in eq\n");
    auto ast = new LAndExpAST();
    ast->eq_exp = unique_ptr<BaseAST>($1);
    ast->branch.push_back($1);
    $$ = ast;
  }
  | LAndExp AND EqExp {
    auto ast = new LAndExpAST();
    ast->land_exp = unique_ptr<BaseAST>($1);
    ast->op = "&&";
    ast->eq_exp = unique_ptr<BaseAST>($3);
    ast->branch.push_back($1);
    ast->branch.push_back($3);
    $$ = ast;
  }
  ;

LOrExp
  : LAndExp {
    printf("in land\n");
    auto ast = new LOrExpAST();
    ast->land_exp = unique_ptr<BaseAST>($1);
    ast->branch.push_back($1);
    $$ = ast;
  }
  | LOrExp OR LAndExp {
    printf("in lor OR land\n");
    auto ast = new LOrExpAST();
    ast->lor_exp = unique_ptr<BaseAST>($1);
    ast->op = "||";
    ast->land_exp = unique_ptr<BaseAST>($3);
    ast->branch.push_back($1);
    ast->branch.push_back($3);
    $$ = ast;
  }
  ;


%%

// 定义错误处理函数, 其中第二个参数是错误信息
// parser 如果发生错误 (例如输入的程序出现了语法错误), 就会调用这个函数
void yyerror(unique_ptr<BaseAST> &ast, const char *s) {
  cerr << "error: " << s << endl;
}
