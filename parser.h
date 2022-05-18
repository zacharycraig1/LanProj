//----------------------------------------------------------------------
// NAME:Zachary Craig
// FILE:parser.h
// DATE:2/14/2021
// DESC:Imeplementation of parser descent functions
//----------------------------------------------------------------------

#ifndef PARSER_H
#define PARSER_H
using namespace std;
#include "token.h"
#include "mypl_exception.h"
#include "printer.h"
#include "ast.h"

#include <list>
class Parser {
public:
    // create a new recursive descent parser
    Parser(const Lexer& program_lexer);

    // run the parser
    void parse(Program& node);

private:
    Lexer lexer;
    Token curr_token;

    // helper functions
    void advance();
    void eat(TokenType t, std::string err_msg);
    void error(std::string err_msg);
    bool is_operator(TokenType t);

    // recursive descent functions
    void tdecl(TypeDecl* type_decl);
    void fdecl(FunDecl* fun_decl);
    // TODO: add additional recursive descent functions here

    void vdecls(TypeDecl* type_decl);
    void params(FunDecl* fun_decl);
    void dtype(Token* token_type);
    void stmts(list<Stmt*>& stmts_list);
    void stmt(list<Stmt*>& stmts_list);
    void vdecl_stmt(VarDeclStmt* var_decl_stmt);
    void assign_stmt(AssignStmt*);
    void assign_stmt_S(AssignStmt*);
    void lvalue(AssignStmt*);
    void cond_stmt(IfStmt*);
    void condt(IfStmt*);
    void while_stmt(WhileStmt*);
    void for_stmt(ForStmt*);
    void call_expr(CallExpr*);
    void call_expr_S(CallExpr*);
    void args(list<Expr*>&);
    void exit_stmt(ReturnStmt*);
    void expr(Expr* node);
    void operate();
    RValue* rvalue();
    
    
    void pval();
    void idrval(IDRValue*);
    void assign_call_mediator(list<Stmt*>& stmts_list);
    void idrval_S(IDRValue*);
};

// constructor
Parser::Parser(const Lexer& program_lexer)
    : lexer(program_lexer)
{
}

// Helper functions
void Parser::advance()
{
    curr_token = lexer.next_token();
}

void Parser::eat(TokenType t, std::string err_msg)
{
    if (curr_token.type() == t) {
        advance();
      }
    else
        error(err_msg);
}

void Parser::error(std::string err_msg)
{
    std::string s = err_msg + "found '" + curr_token.lexeme() + "'";
    int line = curr_token.line();
    int col = curr_token.column();
    throw MyPLException(SYNTAX, s, line, col);
}

bool Parser::is_operator(TokenType t)
{
    return t == PLUS or t == MINUS or t == DIVIDE or t == MULTIPLY or t == MODULO or t == AND or t == OR or t == EQUAL or t == LESS or t == GREATER or t == LESS_EQUAL or t == GREATER_EQUAL or t == NOT_EQUAL;
}
// Recursive-decent functio
void Parser::parse(Program& node)
{

    advance();
    while (curr_token.type() != EOS) {
        if (curr_token.type() == TYPE) {
            TypeDecl* type_decl = new TypeDecl;
            ;
            tdecl(type_decl);
            node.decls.push_back(type_decl);
        }
        else {
            FunDecl* fun_decl = new FunDecl;
            fdecl(fun_decl);
            node.decls.push_back(fun_decl);
        }
    }
    eat(EOS, "expecting end-of-file ");
}

void Parser::tdecl(TypeDecl* type_decl)
{
    // TODO
    eat(TYPE, "Expecting type");
    type_decl->id = curr_token;
    eat(ID, "Expecting ID here3");
    vdecls(type_decl);
    eat(END, "Expecting END");
}
//Function grammar
void Parser::fdecl(FunDecl* fun_decl)
{
    // TODO
    eat(FUN, "Expecting fun");
    if (curr_token.type() == NIL) {
        fun_decl->return_type = curr_token;
        eat(NIL, "expecting NIL");
    }
    else {
        Token* token_type = new Token;
        dtype(token_type);
        fun_decl->return_type = *token_type;
        delete token_type;
    }
    fun_decl->id = curr_token;
    eat(ID, "expecting id");
    eat(LPAREN, "expecting lparen");
    params(fun_decl);
    eat(RPAREN, "expecting rparen");
    list<Stmt*> stmts_list;
    stmts(stmts_list);
    fun_decl->stmts = stmts_list;
    eat(END, "expecting end1");
}
//Variable grammar
void Parser::vdecls(TypeDecl* type_decl)
{
    if (curr_token.type() == VAR) {
        VarDeclStmt* new_var_decl = new VarDeclStmt;
        vdecl_stmt(new_var_decl);
        type_decl->vdecls.push_back(new_var_decl);
        vdecls(type_decl);
    }
    else {
    }
}
//Function parameter grammar
void Parser::params(FunDecl* fun_decl)
{

    if (curr_token.type() == ID) {
        FunDecl::FunParam* fun_param = new FunDecl::FunParam;
        fun_param->id = curr_token;
        eat(ID, "Expecting ID here 2");
        eat(COLON, "Expecting colon");
        Token* token_type = new Token;
        dtype(token_type);
        fun_param->type = *token_type;
        fun_decl->params.push_back(*fun_param);
        while (curr_token.type() == COMMA) {
            FunDecl::FunParam* fun_param = new FunDecl::FunParam;
            eat(COMMA, "expecting comma");
            fun_param->id = curr_token;
            eat(ID, "expecting id");
            eat(COLON, "expecting colon");
            Token* token_type = new Token;
            dtype(token_type);
            fun_param->type = *token_type;
            fun_decl->params.push_back(*fun_param);
        }
    }
    else {
    }
}
//declaration type grammar
void Parser::dtype(Token* token_type)
{
    if (curr_token.type() == STRING_TYPE || curr_token.type() == DOUBLE_TYPE || curr_token.type() == CHAR_TYPE || curr_token.type() == ID || curr_token.type() == BOOL_TYPE || curr_token.type() == INT_TYPE || curr_token.type() == MATRIX_TYPE) {
        *token_type = curr_token;
        advance();
    }
    else {
        error("Expecting dtype here1");
    }
}
//General statements within a function body grammar
void Parser::stmts(list<Stmt*>& stmts_list)
{
    if (curr_token.type() == ID || curr_token.type() == VAR || curr_token.type() == IF || curr_token.type() == WHILE || curr_token.type() == FOR || curr_token.type() == RETURN) {
        stmt(stmts_list);
        stmts(stmts_list);
    }
}
//Single statement
void Parser::stmt(list<Stmt*>& stmts_list)
{
    if (curr_token.type() == VAR) {
        VarDeclStmt* new_var_decl = new VarDeclStmt;
        vdecl_stmt(new_var_decl);
        new_var_decl->id;
        stmts_list.push_back(new_var_decl);
    }
    else if (curr_token.type() == IF) {
        IfStmt* new_if_stmt = new IfStmt;
        cond_stmt(new_if_stmt);
        stmts_list.push_back(new_if_stmt);
    }
    else if (curr_token.type() == RETURN) {
        ReturnStmt* new_return_stmt = new ReturnStmt;
        exit_stmt(new_return_stmt);
        stmts_list.push_back(new_return_stmt);
    }
    else if (curr_token.type() == WHILE) {
        WhileStmt* new_while_stmt = new WhileStmt;
        while_stmt(new_while_stmt);
        stmts_list.push_back(new_while_stmt);
    }
    else if (curr_token.type() == FOR) {
        ForStmt* new_for_stmt = new ForStmt;
        for_stmt(new_for_stmt);
        stmts_list.push_back(new_for_stmt);
    }
    else if (curr_token.type() == ID) {
        assign_call_mediator(stmts_list);
    }
    else {
        error("Expecting stmt");
    }
}

//Mediates a special case of call_expr and assign_stmt in expr to keep expr() LL(1)
void Parser::assign_call_mediator(list<Stmt*>& stmts_list)
{
    Token new_token = curr_token;
   
    eat(ID, "expecting ID here1");

    if (curr_token.type() == LPAREN) {
        CallExpr* new_call_expr = new CallExpr;
        new_call_expr->function_id = new_token;
        call_expr_S(new_call_expr);
        stmts_list.push_back(new_call_expr);
    }
    else {
        AssignStmt* new_assign_stmt = new AssignStmt;
        new_assign_stmt->lvalue_list.push_back(new_token);
        assign_stmt_S(new_assign_stmt);
        stmts_list.push_back(new_assign_stmt);
    }
}
//variable declaration grammar
void Parser::vdecl_stmt(VarDeclStmt* var_decl_stmt)
{
    eat(VAR, "expecting var here2");
    var_decl_stmt->id = curr_token;
    eat(ID, "expecting id");
    if (curr_token.type() == COLON) {
        Token* new_type = new Token;
        eat(COLON, "expecting colon");
        dtype(new_type);
        var_decl_stmt->type = new_type;
    }
    else {
    }
    eat(ASSIGN, "expecting assign hi1");
    Expr* expr_node = new Expr;
    expr(expr_node);
    var_decl_stmt->expr = expr_node;
}
//Assigning grammar special case with two ID's in parent recursive function
void Parser::assign_stmt_S(AssignStmt* new_assign_stmt)
{
    Expr* expr_node = new Expr;
    while (curr_token.type() == DOT) {
        eat(DOT, "expecting dot");
        new_assign_stmt->lvalue_list.push_back(curr_token);
        eat(ID, "expecting id");
    }
    eat(ASSIGN, "expecting assign hi2");
    expr(expr_node);
    new_assign_stmt->expr = expr_node;
}
//Assigning grammar general case
void Parser::assign_stmt(AssignStmt* new_assign_stmt)
{
    Expr* expr_node = new Expr;
    lvalue(new_assign_stmt);
    eat(ASSIGN, "expecting assign hi3");
    expr(expr_node);
    new_assign_stmt->expr = expr_node;
}
//grammar for conditional statements
void Parser::lvalue(AssignStmt* new_assign_stmt)
{
    new_assign_stmt->lvalue_list.push_back(curr_token);
    eat(ID, "expecting id");
    while (curr_token.type() == DOT) {
        eat(DOT, "expecting dot");
        new_assign_stmt->lvalue_list.push_back(curr_token);
        eat(ID, "expecting id");
    }
}
//if stmt
void Parser::cond_stmt(IfStmt* new_if_stmt)
{
    BasicIf* new_basic_if = new BasicIf;
    Expr* expr_node = new Expr;
    eat(IF, "expecting if");
    expr(expr_node);
    new_basic_if->expr = expr_node;
    eat(THEN, "expecting then");
    list<Stmt*> stmts_list;
    stmts(stmts_list);
    new_basic_if->stmts = stmts_list;
    new_if_stmt->if_part = new_basic_if;
    condt(new_if_stmt);
    eat(END, "expecting end2");
}
//conditional grammar
void Parser::condt(IfStmt* new_if_stmt)
{
    if (curr_token.type() == ELSEIF) {
        BasicIf* new_basic_if = new BasicIf;
        Expr* expr_node = new Expr;
        eat(ELSEIF, "expecting elseif");
        expr(expr_node);
        new_basic_if->expr = expr_node;
        eat(THEN, "expecting then");
        list<Stmt*> stmts_list;
        stmts(stmts_list);
        new_basic_if->stmts = stmts_list;
        new_if_stmt->else_ifs.push_back(new_basic_if);
        condt(new_if_stmt);
    }
    else if (curr_token.type() == ELSE) {
        eat(ELSE, "expecting else");
        list<Stmt*> stmts_list;
        stmts(stmts_list);
        new_if_stmt->body_stmts = stmts_list;
    }
    else {
    }
}
//while loop grammar
void Parser::while_stmt(WhileStmt* new_while_stmt)
{
    Expr* expr_node = new Expr;
    eat(WHILE, "expecting while");
    expr(expr_node);
    new_while_stmt->expr = expr_node;
    eat(DO, "expecting do");
    list<Stmt*> stmts_list;
    stmts(stmts_list);
    new_while_stmt->stmts = stmts_list;
    eat(END, "expecting end3");
}
//for loop grammar
void Parser::for_stmt(ForStmt* new_for_stmt)
{
    Expr* expr_node = new Expr;
    eat(FOR, "expecting for");
    new_for_stmt->var_id = curr_token;
    eat(ID, "execting id");
    eat(ASSIGN, "expecting assign hi4");
    expr(expr_node);
    new_for_stmt->start = expr_node;
    eat(TO, "expecting to");
    Expr* expr_node2 = new Expr;
    expr(expr_node2);
    new_for_stmt->end = expr_node2;
    eat(DO, "expecting do");
    list<Stmt*> stmts_list;
    stmts(stmts_list);
    new_for_stmt->stmts = stmts_list;
    eat(END, "expecting end4");
}
//call expression grammar: special case where there are two LPAREN alterations in parent recursive function
void Parser::call_expr_S(CallExpr* new_call_expr)
{
    eat(LPAREN, "expecting lparen");
    list<Expr*> expr_list;
    args(expr_list);
    new_call_expr->arg_list = expr_list;
   
    eat(RPAREN, "expecting rparen");
}
//general call expression grammar
void Parser::call_expr(CallExpr* new_call_expr)
{
    new_call_expr->function_id = curr_token;
    eat(ID, "expecting ID");
    eat(LPAREN, "expecting lparen");
    list<Expr*> expr_list;
    args(expr_list);
    new_call_expr->arg_list = expr_list;
    
    eat(RPAREN, "expecting rparen");
}
//function arguments
void Parser::args(list<Expr*>& expr_list)
{
string current_token = curr_token.lexeme();

if (curr_token.lexeme() == ")") {

}
    else if (curr_token.type() == INT_VAL || DOUBLE_VAL || STRING_VAL || CHAR_VAL || BOOL_VAL || NIL || NEW || ID || NEG || MATRIX_VAL || TRANSPOSE) {
        Expr* expr_node = new Expr;

        expr(expr_node);
        expr_list.push_back(expr_node);
        
        while (curr_token.type() == COMMA) {
            Expr* expr_node = new Expr;
           
            eat(COMMA, "expecting comma");
            expr(expr_node);
            expr_list.push_back(expr_node);
        }
    }
    else {
    }
    
}
//Return statement grammar
void Parser::exit_stmt(ReturnStmt* new_return_stmt)
{
    eat(RETURN, "expecting return");
    Expr* expr_node = new Expr;
    expr(expr_node);
    new_return_stmt->expr = expr_node;
}
//general expression grammar
void Parser::expr(Expr* node)
{

    if (curr_token.type() == INT_VAL || curr_token.type() == STRING_VAL || curr_token.type() == CHAR_VAL || curr_token.type() == DOUBLE_VAL || curr_token.type() == BOOL_VAL || curr_token.type() == ID || curr_token.type() == NIL || curr_token.type() == NEW || curr_token.type() == NEG || curr_token.type() == L_BRACKET || curr_token.type() == TRANSPOSE) {
        SimpleTerm* new_sim_term = new SimpleTerm;
        new_sim_term->rvalue = rvalue();
         
        node->first = new_sim_term;
    }
    else if (curr_token.type() == NOT) {
        ComplexTerm* new_complex_term = new ComplexTerm;
        node->negated = true;
        eat(NOT, "expecting not");
        Expr* new_expr1 = new Expr;
        expr(new_expr1);
        new_complex_term->expr = new_expr1;
        node->first = new_complex_term;
    }
    else if (curr_token.type() == LPAREN) {
    	
        ComplexTerm* new_complex_term = new ComplexTerm;
        eat(LPAREN, "expecting lparen");
        Expr* new_expr2 = new Expr;
        expr(new_expr2);
        new_complex_term->expr = new_expr2;
        eat(RPAREN, "expecting rparen");
        node->first = new_complex_term;
    }
    else {
   
    }
    if (curr_token.type() == PLUS || curr_token.type() == MINUS || curr_token.type() == DIVIDE || curr_token.type() == MULTIPLY || curr_token.type() == MODULO || curr_token.type() == AND || curr_token.type() == OR || curr_token.type() == EQUAL || curr_token.type() == LESS || curr_token.type() == GREATER || curr_token.type() == LESS_EQUAL || curr_token.type() == GREATER_EQUAL || curr_token.type() == NOT_EQUAL || curr_token.type() == DOT_MULTIPLY || curr_token.type() == DOT_DIVIDE || curr_token.type() == DOT_EXPO || curr_token.type() == EXPO) {
        Token* new_token = new Token;
        *new_token = curr_token;
        node->op = new_token;
        operate();
        Expr* new_expr3 = new Expr;
        expr(new_expr3);
        node->rest = new_expr3;
    }
    else {
    }
}

void Parser::operate()
{

    if (curr_token.type() == PLUS) {
        eat(PLUS, "Expecting plus");
    }
    if (curr_token.type() == MINUS) {
        eat(MINUS, "Expecting MINUS");
    }
    if (curr_token.type() == DIVIDE) {
        eat(DIVIDE, "Expecting DIVIDE");
    }
    if (curr_token.type() == MULTIPLY) {
        eat(MULTIPLY, "Expecting MULTIPLY");
    }
    if (curr_token.type() == MODULO) {
        eat(MODULO, "Expecting MODULO");
    }
    if (curr_token.type() == AND) {
        eat(AND, "Expecting AND");
    }
    if (curr_token.type() == OR) {
        eat(OR, "Expecting OR");
    }
    if (curr_token.type() == EQUAL) {
        eat(EQUAL, "Expecting EQUAL");
    }
    if (curr_token.type() == LESS_EQUAL) {
        eat(LESS_EQUAL, "Expecting LESS_EQUAL");
    }
    if (curr_token.type() == GREATER_EQUAL) {
        eat(GREATER_EQUAL, "Expecting GREATER_EQUAL");
    }
    if (curr_token.type() == GREATER) {
        eat(GREATER, "Expecting GREATER");
    }
    if (curr_token.type() == LESS) {
        eat(LESS, "Expecting LESS");
    }
    if (curr_token.type() == DOT_MULTIPLY) {
        eat(DOT_MULTIPLY, "Expecting DOT_MULTIPLY");
    }
    if (curr_token.type() == DOT_DIVIDE) {
        eat(DOT_DIVIDE, "Expecting DOT_DIVIDE");
    }
    if (curr_token.type() == DOT_EXPO) {
        eat(DOT_EXPO, "Expecting DOT_EXPO");
    }
    if (curr_token.type() == EXPO) {
        eat(EXPO, "Expecting EXPO");
    }
    
}
//value type grammars
RValue* Parser::rvalue()
{


    if (curr_token.type() == INT_VAL || curr_token.type() == DOUBLE_VAL || curr_token.type() == STRING_VAL || curr_token.type() == CHAR_VAL || curr_token.type() == BOOL_VAL) {
        SimpleRValue* new_simple_r_val = new SimpleRValue;
        new_simple_r_val->value = curr_token;
        pval();
        return new_simple_r_val;
    }
    else if (curr_token.type() == L_BRACKET) {
    	MatrixValue* new_matrix_val = new MatrixValue;
    	new_matrix_val->M.push_back({});
    	new_matrix_val->first_bracket = curr_token;
    	int col_length = 0;
    	bool first_semiColon = false;
    	
    	eat(L_BRACKET,"Expecting L_BRACKET");
    	int column = 0;
    	int row = 0;
    	while (curr_token.type() != R_BRACKET) {
    	
    	if (curr_token.type() == SEMICOLON) {
    		
    		if (first_semiColon == false) {
    		first_semiColon = true;
    		col_length = column;
    		}
    		if (column != col_length) {
    		error("Incorrect column length");
    		}
    		
    		new_matrix_val->M.push_back({});
    		
    		
    		eat(SEMICOLON,"Expecting semicolon");
    		column = 0; 
    		row = row + 1;
    	}
    	else {
    		Expr* new_expr_node = new Expr;
    		expr(new_expr_node);
    		new_matrix_val->M.at(row).push_back(new_expr_node);
    		
    	if (curr_token.type() == R_BRACKET) {
    	eat(R_BRACKET,"Expecting right bracket");
    	return new_matrix_val;
    	
    	} 
    	else if (curr_token.type() == COMMA) {
    	eat(COMMA,"Expecting comma");
    	
    	}
    	column = column + 1;
    	}
    	
    	
    	
    	}
    
    }
    else if (curr_token.type() == NIL) {

        SimpleRValue* new_r_val = new SimpleRValue;
        new_r_val->value = curr_token;
        eat(NIL, "Expecting NIL");
        return new_r_val;
    }
    else if (curr_token.type() == NEW) {
        eat(NEW, "Expecting NEW");
        NewRValue* new_r_val = new NewRValue;
        new_r_val->type_id = curr_token;

        eat(ID, "expecting id");
        return new_r_val;
    }
    else if (curr_token.type() == ID) {
   
        Token hold_token = curr_token;
        eat(ID, "Expecting ID here5");
	if (curr_token.type() == R_BRACKET) {
	
	}
        else if (curr_token.type() == LPAREN) {
            CallExpr* new_call_expr = new CallExpr;
            call_expr_S(new_call_expr);
            new_call_expr->function_id = hold_token;
            return new_call_expr;
        }
        else {
            IDRValue* new_idr_value = new IDRValue;
            new_idr_value->path.push_back(hold_token);
            idrval_S(new_idr_value);
            return new_idr_value;
        }
    }
    else if (curr_token.type() == NEG) {
        NegatedRValue* new_neg_r_val = new NegatedRValue;

        eat(NEG, "Expecting NEG");
        Expr* new_expr = new Expr;
        expr(new_expr);
        new_neg_r_val->expr = new_expr;
        return new_neg_r_val;
    }
     else if (curr_token.type() == TRANSPOSE) {
        TransposedRValue* new_trans_r_val = new TransposedRValue;

        eat(TRANSPOSE, "Expecting TRANSPOSE");
        Expr* new_expr = new Expr;
        expr(new_expr);
        new_trans_r_val->expr = new_expr;
        return new_trans_r_val;
    }
}

void Parser::pval()
{
    if (curr_token.type() == INT_VAL) {
        eat(INT_VAL, "Expecting INT_VAL");
        
    }
    if (curr_token.type() == DOUBLE_VAL) {
        eat(DOUBLE_VAL, "Expecting DOUBLE_VAL");
    }
    if (curr_token.type() == STRING_VAL) {
        eat(STRING_VAL, "Expecting STRING_VAL");
    }
    if (curr_token.type() == CHAR_VAL) {
        eat(CHAR_VAL, "Expecting CHAR_VAL");
    }
    if (curr_token.type() == BOOL_VAL) {
        eat(BOOL_VAL, "Expecting BOOL_VAL");
    }
}
void Parser::idrval_S(IDRValue* new_idr_val)
{
    while (curr_token.type() == DOT) {
        eat(DOT, "expecting DOT");
        new_idr_val->path.push_back(curr_token);
        eat(ID, "expecting ID");
    }
}
//id and attribute grammar
void Parser::idrval(IDRValue* new_idr_val)
{
    new_idr_val->path.push_back(curr_token);
    eat(ID, "Expecting ID here4");
    while (curr_token.type() == DOT) {
        eat(DOT, "expecting DOT");
        new_idr_val->path.push_back(curr_token);
        eat(ID, "expecting ID");
    }
}

// TODO: implement the recursive descent functions you declared above

#endif
