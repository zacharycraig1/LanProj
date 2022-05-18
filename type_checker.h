//----------------------------------------------------------------------
// NAME: Zachary  Craig
// FILE: type_checker.h
// DATE: 3/15/2021
// DESC: Implementation of type_checker for MyPl
//----------------------------------------------------------------------

#ifndef TYPE_CHECKER_H
#define TYPE_CHECKER_H

#include <iostream>
#include "ast.h"
#include "symbol_table.h"

class TypeChecker : public Visitor {
public:
    // top-level
    void visit(Program& node);
    void visit(FunDecl& node);
    void visit(TypeDecl& node);
    // statements
    void visit(VarDeclStmt& node);
    void visit(AssignStmt& node);
    void visit(ReturnStmt& node);
    void visit(IfStmt& node);
    void visit(WhileStmt& node);
    void visit(ForStmt& node);
    // expressions
    void visit(Expr& node);
    void visit(SimpleTerm& node);
    void visit(ComplexTerm& node);
    // rvalues
    void visit(SimpleRValue& node);
    void visit(NewRValue& node);
    void visit(CallExpr& node);
    void visit(IDRValue& node);
    void visit(NegatedRValue& node);
    void visit(MatrixValue& node);
    void visit(TransposedRValue& node);
private:
    // the symbol table
    SymbolTable sym_table;

    // the previously inferred type
    std::string curr_type;

    // helper to add built in functions
    void initialize_built_in_types();

    // error message
    void error(const std::string& msg, const Token& token);
    void error(const std::string& msg);
};

void TypeChecker::error(const std::string& msg, const Token& token)
{
    throw MyPLException(SEMANTIC, msg, token.line(), token.column());
}

void TypeChecker::error(const std::string& msg)
{
    throw MyPLException(SEMANTIC, msg);
}

void TypeChecker::initialize_built_in_types()
{
    // print function
    sym_table.add_name("print");
    sym_table.set_vec_info("print", StringVec{ "string", "nil" });
    // stoi function
    sym_table.add_name("stoi");
    sym_table.set_vec_info("stoi", StringVec{ "string", "int" });
    //stod
    sym_table.add_name("stod");
    sym_table.set_vec_info("stod", StringVec{ "string", "double" });
    //itos
    sym_table.add_name("itos");
    sym_table.set_vec_info("itos", StringVec{ "int", "string" });
    //dtos
    sym_table.add_name("dtos");
    sym_table.set_vec_info("dtos", StringVec{ "double", "string" });
    //get
    sym_table.add_name("get");
    sym_table.set_vec_info("get", StringVec{ "int", "string", "char" });
    //length
    sym_table.add_name("length");
    sym_table.set_vec_info("length", StringVec{ "string", "int" });
    //read
    sym_table.add_name("read");
    sym_table.set_vec_info("read", StringVec{ "nil" });
    //Print Matrix
     sym_table.add_name("m_print");
    sym_table.set_vec_info("m_print",StringVec{"matrix","nil"});
    
     sym_table.add_name("m_get");
     sym_table.set_vec_info("m_get",StringVec{"matrix","int","int","double"});
     sym_table.add_name("m_singleton");
     sym_table.set_vec_info("m_singleton",StringVec{"double", "int", "int", "matrix"});
    
    // TODO: finish the rest of the built-in functions: stod, itos,
    // dtos, get, length, and read
}
void TypeChecker::visit(MatrixValue& node) {

for (int row = 0; row < node.M.size(); row++) {
	
	for (int column = 0; column < node.M.at(row).size();column++) {
		node.M.at(row).at(column)->accept(*this);
		if (curr_type != "double") {
		error("Matrix needs to be of all double types",node.first_bracket);
		
		}



	}



}

curr_type = "matrix";

}



void TypeChecker::visit(Program& node)
{
    // push the global environment
    sym_table.push_environment();
    // add built-in functions
    initialize_built_in_types();
    // push
    for (Decl* d : node.decls)
        d->accept(*this);
    // check for a main function
    if (sym_table.name_exists("main") and sym_table.has_vec_info("main")) {
        StringVec the_type;
        sym_table.get_vec_info("main", the_type);
        if (the_type[the_type.size() - 1] != "int") {
            error("Incorrect main function signature");
        }
        // TODO: finish checking that the main function is defined with
        // the correct signature
    }
    else {
        // NOTE: the only time the 1-argument version of error should be
        // called!
        error("undefined 'main' function");
    }
    // pop the global environment
    sym_table.pop_environment();
}
// TODO: Implement the remaining visitor functions

void TypeChecker::visit(FunDecl& node)
{
//We ensure that the function is not previously defined
    if (sym_table.has_vec_info(node.id.lexeme())) {
        error("Previously declared function", node.id);
    }
    //Extract param types
    StringVec the_type;
    for (FunDecl::FunParam iter : node.params) {
        the_type.push_back(iter.type.lexeme());
    }
    
    the_type.push_back(node.return_type.lexeme());
    sym_table.add_name(node.id.lexeme());
    sym_table.set_vec_info(node.id.lexeme(), the_type);
    //Params and body
    if (!sym_table.has_vec_info(node.id.lexeme())) {
        error("No matching function for id", node.id);
    }
    
    StringVec second_type;
    sym_table.get_vec_info(node.id.lexeme(), second_type);
    string return_type = second_type[second_type.size() - 1];
    sym_table.push_environment();
    sym_table.add_name("return");
    sym_table.set_str_info("return", return_type);
    //Check for duplicate param types
    for (FunDecl::FunParam iter : node.params) {
        if (sym_table.has_str_info(iter.id.lexeme())) {
            error("duplicate types", iter.id);
        }
        
        sym_table.add_name(iter.id.lexeme());
        sym_table.set_str_info(iter.id.lexeme(), iter.type.lexeme());
    }
    //Function body
    for (Stmt* iter : node.stmts) {
        iter->accept(*this);
    }
    sym_table.pop_environment();
}

void TypeChecker::visit(TypeDecl& node)
{
//Check for type redeclaration
if (sym_table.has_map_info(node.id.lexeme())) {
error("Type redeclaration",node.id);

}
    sym_table.add_name(node.id.lexeme());
    StringMap the_type;
    sym_table.push_environment();
    //member variables
    for (VarDeclStmt* iter : node.vdecls) {
        iter->accept(*this);
        the_type[iter->id.lexeme()] = curr_type;
    }
    sym_table.pop_environment();
    sym_table.set_map_info(node.id.lexeme(), the_type);
}
// statements
void TypeChecker::visit(VarDeclStmt& node)
{
    node.expr->accept(*this);
    string exp_type = curr_type;
    string var_name = node.id.lexeme();
    //Shadowing
    if (sym_table.has_str_info(var_name) && sym_table.name_exists_in_curr_env(var_name)) {
        error("variable \'" + var_name + "\' redefined in current scope", node.id);
    }
    sym_table.add_name(var_name);
    string lhs_type;
    if (node.type == nullptr) {
        lhs_type = "nul";
    }
    else {
        lhs_type = node.type->lexeme();
    }
    //Checking for flexible type declaration
    if (lhs_type == "nul") {
        sym_table.set_str_info(var_name, exp_type);
        curr_type = exp_type;
    }
    //Type checking
    else {
        if (lhs_type == exp_type || exp_type == "nil") {
            sym_table.add_name(var_name);
            sym_table.set_str_info(var_name, lhs_type);
            curr_type = lhs_type;
        }
        else {
            error("invalid type for variable \'" + node.id.lexeme() + "\'", *node.type);
        }
    }
}

void TypeChecker::visit(AssignStmt& node)
{

    node.expr->accept(*this);
    string rhs_type = curr_type;
    string lhs_type;
    //check to see if we have str info
    if (!sym_table.has_str_info(node.lvalue_list.front().lexeme())) {
        error("ID does not exist: \'" + node.lvalue_list.front().lexeme(), node.lvalue_list.front());
    }
    //If the lvalue is not a udt, 
    if (node.lvalue_list.size() == 1) {
        if (!sym_table.has_str_info(node.lvalue_list.front().lexeme())) {
            error("ID does not exist901", node.lvalue_list.front());
        }
        else {
            sym_table.get_str_info(node.lvalue_list.front().lexeme(), lhs_type);
        }
    }
    else {
    //When we have a udt
        StringMap prev_type_udt;
        string the_name;
        string the_type;
        the_name = node.lvalue_list.front().lexeme();
        //if id doesn\t have type info
        if (!sym_table.has_str_info(the_name)) {
            error("ID does not exist5", node.lvalue_list.front());
        }
        sym_table.get_str_info(the_name, the_type);
        //Ensures that our lvalue sequence is actually a udt
        if (!sym_table.has_map_info(the_type)) {
            error("Type doesn't exist3", node.lvalue_list.front());
        }
        sym_table.get_map_info(the_type, prev_type_udt);
        int inter = 0;
        //Go through lvalue list and check
        for (Token iter : node.lvalue_list) {
            if (inter != 0 && inter < node.lvalue_list.size() - 1) {
                if (prev_type_udt.count(iter.lexeme()) > 0) {
                    the_type = prev_type_udt[iter.lexeme()];
                    the_name = iter.lexeme();
                    if (sym_table.has_map_info(the_type)) {
                        sym_table.get_map_info(the_type, prev_type_udt);
                    }
                    else {
                    //Error when we try to access a member variable which does not exist
                        error("Variable not a user-defined type", node.lvalue_list.front());
                    }
                }
                else {
                    error("ID does not exist967", iter);
                }
            }
            inter++;
        }
        lhs_type = prev_type_udt[node.lvalue_list.back().lexeme()];
    }
    if (rhs_type != lhs_type && rhs_type != "nil") {
        error("Mismatched types in assignment", node.lvalue_list.front());
    }
}
void TypeChecker::visit(ReturnStmt& node)
{
//Grab the expression type in after return token
    node.expr->accept(*this);
    string the_type;
    if (sym_table.has_str_info("return")) {
        sym_table.get_str_info("return", the_type);
        if (the_type == "nil") {
            if (the_type != curr_type) {
            	//we cannot return anything but nil to a nil return type
                error("invalid return expression type", node.expr->first_token());
            }
        }
        //We cannot return mismatched types unless the return type is nil
        else if (the_type != curr_type && curr_type != "nil") {
            error("invalid return expression type", node.expr->first_token());
        }
    }
    else {
        error("could not find return type", node.expr->first_token());
    }
}

void TypeChecker::visit(IfStmt& node)
{
//If if does not exist but we still have else ifs statements, there is a problem
    if (node.if_part == nullptr && (node.else_ifs.size() > 0)) {
        error("else if declared without previous if", node.else_ifs.front()->expr->first_token());
    }
    
    else if (node.if_part == nullptr && (node.else_ifs.size() > 0)) {
        error("else declared without previous if");
    }
    //Checks if expression for bool
    node.if_part->expr->accept(*this);
    if (curr_type != "bool") {
        error("Expected bool in if expression", node.if_part->expr->first_token());
    }
    sym_table.push_environment();
    for (Stmt* iter2 : node.if_part->stmts) {
        iter2->accept(*this);
    }
    sym_table.pop_environment();

    if (node.else_ifs.size() != 0) {
        for (BasicIf* iter : node.else_ifs) {
            iter->expr->accept(*this);
            if (curr_type != "bool") {
                error("expecting bool", iter->expr->first_token());
            }
            sym_table.push_environment();
            list<Stmt*> mango_stmt = iter->stmts;
            for (Stmt* iter2 : mango_stmt) {
                iter2->accept(*this);
            }
            sym_table.pop_environment();
        }
    }
    sym_table.push_environment();
    if (node.body_stmts.size() != 0) {
        for (Stmt* iter2 : node.body_stmts) {
            iter2->accept(*this);
        }
    }
    sym_table.pop_environment();
}
void TypeChecker::visit(WhileStmt& node)
{
    if (node.expr == nullptr) {
        error("expecting while condition");
    }
    //Checks while expression for bool
    node.expr->accept(*this);
    if (curr_type != "bool") {
        error("Expecting bool expression", node.expr->first_token());
    }
    sym_table.push_environment();
    for (Stmt* iter : node.stmts) {
        iter->accept(*this);
    }
    sym_table.pop_environment();
}

void TypeChecker::visit(ForStmt& node)
{

    sym_table.push_environment();
    sym_table.add_name(node.var_id.lexeme());
    sym_table.set_str_info(node.var_id.lexeme(), "int");
    node.start->accept(*this);
    //Ensures that x start condition is int
    if (curr_type != "int") {
        error("expecting \'int\' in start expression", node.start->first_token());
    }
    node.end->accept(*this);
    //Ensures that x end condition is int
    if (curr_type != "int") {
        error("expecting \'int\' in end expression", node.end->first_token());
    }

    //Body statements
    for (Stmt* iter : node.stmts) {
        iter->accept(*this);
    }
    sym_table.pop_environment();
}
// expressions
void TypeChecker::visit(Expr& node)
{
//Checks for missing expr parts
    if (node.first != nullptr) {
        node.first->accept(*this);
    }
    //Right hand side is first expr
    string rhs_type = curr_type;
    if (node.rest != nullptr) {
        node.rest->accept(*this);
    }
    //operation type checking rules
    if (node.op != nullptr) {
        Token* op = node.op;
        if (node.op->lexeme() == "%") {
       
            if (curr_type == "int" && rhs_type == "int") {
                curr_type = "int";
            }
            else if (curr_type == "int" && rhs_type == "matrix") {
           
            curr_type = "matrix";
            }
            else {
                error("incorrect modulo", node.first_token());
            }
        }
        else if ((node.op->lexeme() == ".*" || node.op->lexeme() == "./" || node.op->lexeme() == ".^")) {
        	
        	
        	if (curr_type == "matrix" && rhs_type == "matrix") {
        	curr_type = "matrix";
        	
        	}
        	else {
        	error("Expecting matrix values",node.first_token());
        	
        	}

        	
        
        }
        

        	
        
        else if (node.op->lexeme() == "^") {
       	 if (curr_type == "int" && rhs_type == "matrix") {
        	curr_type = "matrix";
        
        
       	 }
        	else if (curr_type == "int" && rhs_type == "int") {
        	curr_type == "int";
        	
        	}
        	else if (curr_type == "int" && rhs_type == "double") {
        	curr_type == "double";
        	
        	}
        	else {
        	error("Improper use of '^'", node.first_token());
        	
        	}
        }
        else if ((node.op->lexeme() == "-" || node.op->lexeme() == "*" || node.op->lexeme() == "/") && (rhs_type == "char" || curr_type == "char" || rhs_type == "string" || curr_type == "string")) {
            error("mismatched types", node.first_token());
        }
        else if ((node.op->lexeme() == "+" || node.op->lexeme() == "-" || node.op->lexeme() == "*" || node.op->lexeme() == "/") && (curr_type != "char" && rhs_type != "char" && curr_type != "string" && rhs_type != "string")) {
            if (curr_type == "int" && rhs_type == "int") {
                curr_type = "int";
            }
            else if (curr_type == "double" && rhs_type == "double") {
                curr_type = "double";
            }
             else if (curr_type == "matrix" && rhs_type == "matrix") {
            curr_type = "matrix";
            
            }
             else if (curr_type == "matrix" && (rhs_type == "double" ||rhs_type == "int") && node.op->lexeme() == "*") {
            curr_type = "matrix";
            
            }
             else if (rhs_type == "matrix" && (curr_type == "double" || curr_type == "int") && node.op->lexeme() == "*") {
            curr_type = "matrix";
            
            }
            
             else if (rhs_type == "matrix" && (curr_type == "double" || curr_type == "int") && node.op->lexeme() == "/") {
            curr_type = "matrix";
            
            }
            else {
                error("incorrect math operation", node.first_token());
            }
        }
        
        else if (node.op->lexeme() == "+") {
            if ((curr_type == "char" || curr_type == "string") && (rhs_type == "char" || rhs_type == "string")) {
                curr_type = "string";
            }
           
            else {
                error("mismatched types in " + op->lexeme(), node.first_token());
            }
        }
        else if (node.op->lexeme() == "not") {
            if (curr_type == "bool" && rhs_type == "DNE") {
                curr_type == "bool";
            }
            else {
                error("expecting bool", node.first_token());
            }
        }
        else if (node.op->lexeme() == "and") {
            if (curr_type == "bool" && rhs_type == "bool") {
                curr_type == "bool";
            }
            else {
                error("expecting boolean", node.first_token());
            }
        }
        else if (node.op->lexeme() == "or") {
            if (curr_type == "bool" && rhs_type == "bool") {
                curr_type == "bool";
            }
            else {
                error("expecting boolean", node.first_token());
            }
        }
        else if (node.op->lexeme() == "==" || node.op->lexeme() == "!=") {
            if (curr_type == "nil" && rhs_type == "nil") {
                curr_type = "bool";
            }
            else if ((curr_type == "string" || curr_type == "nil") && (rhs_type == "string" || rhs_type == "nil")) {
                curr_type = "bool";
            }
            else if ((curr_type == "int" || curr_type == "nil") && (rhs_type == "int" || rhs_type == "nil")) {
                curr_type = "bool";
            }
            else if ((curr_type == "char" || curr_type == "nil") && (rhs_type == "char" || rhs_type == "nil")) {
                curr_type = "bool";
            }
            else if ((curr_type == "bool" || curr_type == "nil") && (rhs_type == "bool" || rhs_type == "nil")) {
                curr_type = "bool";
            }
            else if ((curr_type == "double" || curr_type == "nil") && (rhs_type == "double" || rhs_type == "nil")) {
                curr_type = "bool";
            }
            else if (sym_table.has_map_info(curr_type) || sym_table.has_map_info(rhs_type)) {
                if (sym_table.has_map_info(curr_type) && sym_table.has_map_info(rhs_type) && curr_type == rhs_type) {
                    curr_type = "bool";
                }
                else if (sym_table.has_map_info(curr_type) && rhs_type == "nil") {
                    curr_type = "bool";
                }
                else if (sym_table.has_map_info(rhs_type) && curr_type == "nil") {
                    curr_type = "bool";
                }
                else {
                    error("incorrect use of equality" + node.op->lexeme(), node.first_token());
                }
            }
            else {
                error("mismatched types", node.first_token());
            }
        }
        //More type checking rules for equality comparison
        else if (node.op->lexeme() == "<" || node.op->lexeme() == "<=" || node.op->lexeme() == ">" || node.op->lexeme() == ">=") {
            if ((curr_type == "string" && rhs_type == "string") || (curr_type == "int" && rhs_type == "int") || (curr_type == "char" && rhs_type == "char") || (curr_type == "double" && rhs_type == "double")) {
                curr_type = "bool";
            }
            else {
                error("mismatched types", node.first_token());
            }
        }
    }
    //We want our expr type that we are negating to be bool
    if (node.negated == true) {
        if (curr_type == "bool") {
        }
        else {
            error("expecting bool expression", node.first_token());
        }
    }
}
//Nothing to type check in simple term
void TypeChecker::visit(SimpleTerm& node)
{
    node.rvalue->accept(*this);
    //HERE
}
void TypeChecker::visit(ComplexTerm& node)
{
    node.expr->accept(*this);
}

// assign curr_type to new r value rvalues
void TypeChecker::visit(SimpleRValue& node)
{
//sets curr type to whichever rvalue
    if (node.value.type() == CHAR_VAL) {
        curr_type = "char";
    }
    else if (node.value.type() == STRING_VAL) {
        curr_type = "string";
    }
    else if (node.value.type() == INT_VAL) {
        curr_type = "int";
    }
    else if (node.value.type() == BOOL_VAL) {
        curr_type = "bool";
    }
    else if (node.value.type() == DOUBLE_VAL) {
        curr_type = "double";
    }
    else if (node.value.type() == NIL) {
        curr_type = "nil";
    }
}

void TypeChecker::visit(NewRValue& node)
{
    if (sym_table.has_map_info(node.type_id.lexeme()) || sym_table.name_exists(node.type_id.lexeme())) {
        curr_type = node.type_id.lexeme();
    }
    else {
        error("Undeclared Type", node.type_id);
    }
}
//Type checks call expressions
void TypeChecker::visit(CallExpr& node)
{
    string fun_name = node.function_id.lexeme();
    StringVec fun_type;
    sym_table.get_vec_info(fun_name, fun_type);
    //ensures that the function exists
    if (sym_table.has_vec_info(fun_name) == false) {
        error("function id not found", node.function_id);
    }
    if (fun_type.size() == 1 && node.arg_list.size() == 0) {
        curr_type = fun_type[fun_type.size() - 1];
    }
    //too many arguments
    else if (fun_type.size() - 1 < node.arg_list.size()) {
        error("Too many arguments in function call at " + node.function_id.to_string());
    }
    //too few
    else if (fun_type.size() - 1 > node.arg_list.size()) {
        error("Too few arguments in function call at " + node.function_id.to_string());
    }
    //checks that the params have correct types
    else {
        int iterator = 0;
        for (Expr* iter : node.arg_list) {
            if (iterator < fun_type.size() - 1) {
                iter->accept(*this);
                if (curr_type != fun_type.at(iterator) && curr_type != "nil") {
                    error("Mismatched types in function call,", iter->first_token());
                }
            }
            ++iterator;
        }
        curr_type = fun_type[fun_type.size() - 1];
    }
}

void TypeChecker::visit(IDRValue& node)
{
    StringMap prev_type_udt;
    string the_name;
    string the_type;
    //If it is not a udt member variable grab the front type
    if (node.path.size() == 1) {
        if (!sym_table.has_str_info(node.path.front().lexeme())) {
            error("Undeclared variable \'" + node.path.front().lexeme() + "\'", node.first_token());
        }
        else {
            the_name = node.path.front().lexeme();
            sym_table.get_str_info(the_name, the_type);
            curr_type = the_type;
        }
    }
    else {
        StringMap prev_type_udt;
        string the_name;
        string the_type;
        the_name = node.path.front().lexeme();
        //Possible error
        if (!sym_table.has_str_info(the_name)) {
            error("Undeclared variable \'" + the_name + "\'", node.path.front());
        }
        sym_table.get_str_info(the_name, the_type);
        //ensures that we have a udt
        if (!sym_table.has_map_info(the_type)) {
            error("\'" + node.path.back().lexeme() + "\' not defined in \'" + the_type + "\'", node.path.front());
        }
        sym_table.get_map_info(the_type, prev_type_udt);
        int inter = 0;
        //check path types to ensure that we aren't accessing any member variables that don't exist
        for (Token iter : node.path) {
            if (inter != 0 && inter < node.path.size() - 1) {
                if (prev_type_udt.count(iter.lexeme()) > 0) {
                    the_type = prev_type_udt[iter.lexeme()];
                    the_name = iter.lexeme();
                    if (sym_table.has_map_info(the_type)) {
                        sym_table.get_map_info(the_type, prev_type_udt);
                    }
                    else {
                        error("variable not a user-defined type", iter);
                    }
                }
                else {
                    error("ID does not exist7", iter);
                }
            }
            inter++;
        }
        curr_type = prev_type_udt[node.path.back().lexeme()];
    }
}
void TypeChecker::visit(NegatedRValue& node)
{
//Ensures that negating an rvalue negates an int or double
    node.expr->accept(*this);
    if (curr_type == "int" || curr_type == "double") {
    }
    else {
        error("improper use of negation on rvalue",node.first_token());
    }
}
void TypeChecker::visit(TransposedRValue& node) {
node.expr->accept(*this);
if (curr_type == "matrix") {



}

else {
error("Transpose '~' operation only works on matrices", node.first_token());

}


}

#endif
