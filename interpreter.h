//----------------------------------------------------------------------
// NAME:Zachary Craig
// FILE:interpreter.h
// DATE:4/5/2021
// DESC: Implementation of interpreter
//----------------------------------------------------------------------
#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <iostream>
#include <unordered_map>
#include <regex>
#include "ast.h"
#include "symbol_table.h"
#include "data_object.h"
#include "heap.h"
#include <vector>

class Interpreter : public Visitor {
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
    // return code from calling main
    int return_code() const;

private:
    // return exception
    class MyPLReturnException : public std::exception {
    };

    // the symbol table
    SymbolTable sym_table;

    // holds the previously computed value
    DataObject curr_val;

    // the heap
    Heap heap;

    // the next oid
    size_t next_oid = 0;

    // the functions (all within the global environment)
    std::unordered_map<std::string, FunDecl*> functions;

    // the user-defined types (all within the global environment)
    std::unordered_map<std::string, TypeDecl*> types;

    // the global environment id
    int global_env_id = 0;

    // the program return code
    int ret_code = 0;

    // error message
    void error(const std::string& msg, const Token& token);
    void error(const std::string& msg);
};

int Interpreter::return_code() const
{
    return ret_code;
}

void Interpreter::error(const std::string& msg, const Token& token)
{
    throw MyPLException(RUNTIME, msg, token.line(), token.column());
}

void Interpreter::error(const std::string& msg)
{
    throw MyPLException(RUNTIME, msg);
}






void Interpreter::visit(MatrixValue& node) {
Expr* iter;

vector<vector<double>> evals;
for (int row = 0; row < node.M.size(); row++) {
	evals.push_back({});
	
	
	for (int column = 0; column < node.M.at(row).size();column++) {
		node.M.at(row).at(column)->accept(*this);
		double temp = 0.0;
		curr_val.value(temp);
		evals.at(row).push_back(temp);


	}



}

curr_val = evals;
}







void Interpreter::visit(Program& node)
{

    sym_table.push_environment();
    global_env_id = sym_table.get_environment_id();
    for (Decl* d : node.decls) {
        d->accept(*this);
    }
    CallExpr expr;
    expr.function_id = functions["main"]->id;
    expr.accept(*this);
    sym_table.pop_environment(); 
    
    
}

void Interpreter::visit(FunDecl& node)
{
//Store function decl node
    functions.insert({ node.id.lexeme(), &node });
}

void Interpreter::visit(TypeDecl& node)
{
//store type decl type
    types.insert({ node.id.lexeme(), &node });
}
// statements
void Interpreter::visit(VarDeclStmt& node)
{

    sym_table.add_name(node.id.lexeme());
    node.expr->accept(*this);
    if (!curr_val.is_oid()) {
    //If its not a udt, then simply store value in sym_Table
        sym_table.set_val_info(node.id.lexeme(), curr_val);
    }
    else {
    //If it is a oid and its not already in the sym table, grab it from the heap
        size_t new_oid;
        curr_val.value(new_oid);
        if (!sym_table.has_val_info(node.id.lexeme())) {
            HeapObject t1;
            heap.get_obj(new_oid, t1);
            sym_table.set_val_info(node.id.lexeme(), new_oid);
        }
        //Otherwise do a shallow copy of the two udt variables
        else {
            sym_table.set_val_info(node.id.lexeme(), new_oid);
        }
    }
}

void Interpreter::visit(AssignStmt& node)
{

    node.expr->accept(*this);
    if (node.lvalue_list.size() > 1) {
    //Navigate to the end of the lvalue list by grabbing attributes from the heap
        DataObject t1;
        size_t my_oid;
        HeapObject x;
        sym_table.get_val_info(node.lvalue_list.front().lexeme(), t1);
        t1.value(my_oid);
        int iterator = 0;
        heap.get_obj(my_oid, x);
        for (Token iter : node.lvalue_list) {
            if (iterator > 0 && iterator < node.lvalue_list.size() - 1) {
                x.get_val(iter.lexeme(), t1);
                t1.value(my_oid);
                heap.get_obj(my_oid, x);
            }
            iterator++;
        }
        x.set_att(node.lvalue_list.back().lexeme(), curr_val);
        heap.set_obj(my_oid, x);
    }
    else {
    //if not accessing an attribute just set the variable to curr_val
        sym_table.set_val_info(node.lvalue_list.front().lexeme(), curr_val);
    }
}
void Interpreter::visit(ReturnStmt& node)
{
    node.expr->accept(*this);//Throw a return exception
    throw new MyPLReturnException;
}

void Interpreter::visit(IfStmt& node)
{
//Test to see if we should execute statements within if stmt
    node.if_part->expr->accept(*this);
    bool eval = false;
    curr_val.value(eval);
    if (eval == true) {
        sym_table.push_environment();
        for (Stmt* iter : node.if_part->stmts) {
            iter->accept(*this);
        }
        sym_table.push_environment();
        return;
    }
    else {
    
        if (node.else_ifs.size() != 0) {
        
            for (BasicIf* iter : node.else_ifs) {
                iter->expr->accept(*this);
                curr_val.value(eval);
                //Test to see if we should execute statements within if stmt
                if (eval == true) {
                    sym_table.push_environment();
                    list<Stmt*> mango_stmt = iter->stmts;
                    for (Stmt* iter2 : mango_stmt) {
                        iter2->accept(*this);
                    }
                    sym_table.pop_environment();
                    return;
                }
            }
        }
    }
    sym_table.push_environment();
    for (Stmt* iter : node.body_stmts) {
        iter->accept(*this);
    }
    sym_table.pop_environment();
}

void Interpreter::visit(WhileStmt& node)
{
//Should we execute the while stmt for a first time
    bool expr_cond = false;
    node.expr->accept(*this);
    curr_val.value(expr_cond);
    sym_table.push_environment();
    while (expr_cond) {
        for (Stmt* iter : node.stmts) {
            iter->accept(*this);
        }
        //Continually update while expr condition when stmts finsihed executing
        node.expr->accept(*this);
        curr_val.value(expr_cond);
    }
    sym_table.pop_environment();
}
void Interpreter::visit(ForStmt& node)
{
    sym_table.push_environment();
    sym_table.add_name(node.var_id.lexeme());
    node.start->accept(*this);
    sym_table.set_val_info(node.var_id.lexeme(), curr_val);
    //Ensures that x start condition is int
    int iterator;
    curr_val.value(iterator);
    node.end->accept(*this);
    int end;
    curr_val.value(end);
    while (iterator <= end) {
        for (Stmt* iter : node.stmts) {
            iter->accept(*this);
        }
        DataObject t;
        //check to see if the iterator was changed in the proccess of executing for's stmt body
        sym_table.get_val_info(node.var_id.lexeme(), t);
        t.value(iterator);
        ++iterator; //Update iterator
        sym_table.set_val_info(node.var_id.lexeme(), iterator);
    }
    //Ensures that x end condition is int
    sym_table.pop_environment();
}
// expressions
void Interpreter::visit(Expr& node)
{
    if (node.negated) {
        node.first->accept(*this);
        bool val = false;
        curr_val.value(val);
        curr_val.set(!val);
    }
    else {
        node.first->accept(*this);
        if (node.op) {
            DataObject lhs_object = curr_val;
            node.rest->accept(*this);
            DataObject rhs_object = curr_val;
            if (rhs_object.is_matrix() && lhs_object.is_matrix()) {
            vector<vector<double>> A;
            vector<vector<double>> B;
            lhs_object.value(A);
            rhs_object.value(B);
            if (node.op->lexeme() == "*") {
            	if (A.at(0).size() != B.size()) {
            	error("Inner dimensions must match for '*' operation", node.first_token());
            	
            	}
            
            }
            else if (node.op->lexeme() == ".*" || node.op->lexeme() == "./" || node.op->lexeme() == "+" || node.op->lexeme() == "-" || node.op->lexeme() == "./") {
            	if (A.size() != B.size() || A.at(0).size() != B.at(0).size()) {
            	error("Matrix dimensions must be equivalent", node.first_token());
            	
            	}
            
            }
            
            }
            
            //modulo cases
            if (node.op->lexeme() == "%") {
             //GRAB THIS
             	if (lhs_object.is_integer() && rhs_object.is_integer()) {
             	
             	
             	
                int lhs_val;
                int rhs_val;
                lhs_object.value(lhs_val);
                rhs_object.value(rhs_val);
                curr_val.set(lhs_val % rhs_val);}
                else if (lhs_object.is_matrix()) {
                
                vector<vector<double>> lhs_val;
                vector<vector<double>> z;
                int rhs_val;
                
                lhs_object.value(lhs_val);
                rhs_object.value(rhs_val);
                for (int row = 0; row < lhs_val.size(); row++) {
		
			z.push_back({});
	
			for (int column = 0; column < lhs_val.at(row).size();column++) {
			z.at(row).push_back(static_cast<double>(static_cast<int>(lhs_val.at(row).at(column)) % rhs_val));


			}



		}
		curr_val.set(z);
                
                }
            }
            else if (node.op->lexeme() == "^") {
            	if (rhs_object.is_integer() && lhs_object.is_integer()) {
            	int rhs_val;
            	int lhs_val;
            	lhs_object.value(lhs_val);
            	rhs_object.value(rhs_val);
            	int placeHolder = lhs_val;
            	for (int index = 1; index<rhs_val; index++){
            	    placeHolder = placeHolder * lhs_val;
            	}
            	curr_val = placeHolder;
            	
            	
            	//HETRER
            	}
		else if (rhs_object.is_integer() && lhs_object.is_matrix()) {
		int rhs_val;
		rhs_object.value(rhs_val);
                vector<vector<double>> a;
                lhs_object.value(a);
                vector<vector<double>> b;
                lhs_object.value(b);
                for(int index = 1; index<rhs_val; index++){
                vector<vector<double>> mul;
            		for(int p=0;p<a.size();p++)
				{
				mul.push_back({});
				for(int j=0;j<a.at(0).size();j++) {
				mul.at(p).push_back(0);
					mul[p][j]=0;
					for(int k=0;k<a.at(0).size();k++) {
				mul[p][j]+=a[p][k]*b[k][j];
				
			}
		}
		
		}
		a = mul;
	}
	curr_val = a;
            
            	
            	
            	
            	}
            
            
            }
            //Add cases
            else if (node.op->lexeme() == "+") {
                if (lhs_object.is_integer()) {
                    int lhs_val;
                    int rhs_val;
                    lhs_object.value(lhs_val);
                    rhs_object.value(rhs_val);
                    curr_val.set(lhs_val + rhs_val);
                }
                else if (lhs_object.is_matrix()) {
                vector<vector<double>> x;
                lhs_object.value(x);
                vector<vector<double>> y;
                 vector<vector<double>> z;
                rhs_object.value(y);
               for (int row = 0; row < x.size(); row++) {
		
			z.push_back({});
	
			for (int column = 0; column < x.at(row).size();column++) {
			z.at(row).push_back( (x.at(row).at(column) + y.at(row).at(column)));


			}



		}
		curr_val = z;
                
                 
        }
                else if (lhs_object.is_double()) {
                    double lhs_val;
                    double rhs_val;
                    lhs_object.value(lhs_val);
                    rhs_object.value(rhs_val);
                    curr_val.set(lhs_val + rhs_val);
                }
                else if (lhs_object.is_string() && rhs_object.is_string()) {
                    string lhs_val;
                    string rhs_val;
                    lhs_object.value(lhs_val);
                    rhs_object.value(rhs_val);
                    curr_val.set(lhs_val + rhs_val);
                }
                else if (lhs_object.is_char() && rhs_object.is_string()) {
                    char lhs_val;
                    string rhs_val;
                    lhs_object.value(lhs_val);
                    rhs_object.value(rhs_val);
                    curr_val.set(lhs_val + rhs_val);
                }
                else if (lhs_object.is_string() && rhs_object.is_char()) {
                    string lhs_val;
                    char rhs_val;
                    lhs_object.value(lhs_val);
                    rhs_object.value(rhs_val);
                    curr_val.set(lhs_val + rhs_val);
                }
                else if (lhs_object.is_char() && rhs_object.is_char()) {
                    char lhs_val;
                    char rhs_val;
                    lhs_object.value(lhs_val);
                    rhs_object.value(rhs_val);
                    curr_val.set(lhs_val + rhs_val);
                }
                else if ((lhs_object.is_char()) && rhs_object.is_nil()) {
                    char lhs_val;
                    lhs_object.value(lhs_val);
                    curr_val.set(lhs_val);
                }
                else if ((lhs_object.is_string()) && rhs_object.is_nil()) {
                    string lhs_val;
                    lhs_object.value(lhs_val);
                    curr_val.set(lhs_val);
                }
                else if ((rhs_object.is_char()) && lhs_object.is_nil()) {
                    char rhs_val;
                    rhs_object.value(rhs_val);
                    curr_val.set(rhs_val);
                }
                else if ((rhs_object.is_string()) && lhs_object.is_nil()) {
                    string rhs_val;
                    rhs_object.value(rhs_val);
                    curr_val.set(rhs_val);
                }
            }
	//Subraction cases
            else if (node.op->lexeme() == "-") {
                if (lhs_object.is_integer()) {
                    int lhs_val;
                    int rhs_val;
                    lhs_object.value(lhs_val);
                    rhs_object.value(rhs_val);
                    curr_val.set(lhs_val - rhs_val);
                }
                else if (lhs_object.is_double()) {
                    double lhs_val;
                    double rhs_val;
                    lhs_object.value(lhs_val);
                    rhs_object.value(rhs_val);
                    curr_val.set(lhs_val - rhs_val);
                }
                 else if (lhs_object.is_matrix()) {
                vector<vector<double>> x;
                lhs_object.value(x);
                vector<vector<double>> y;
                 vector<vector<double>> z;
                rhs_object.value(y);
               for (int row = 0; row < x.size(); row++) {
		
			z.push_back({});
	
			for (int column = 0; column < x.at(row).size();column++) {
			z.at(row).push_back( (x.at(row).at(column) - y.at(row).at(column)));


			}



		}
		curr_val = z;
                
                 
        }
            }
            else if (node.op->lexeme() == "/") {
                if (lhs_object.is_integer() && rhs_object.is_double()) {
                    int lhs_val;
                    int rhs_val;
                    lhs_object.value(lhs_val);
                    rhs_object.value(rhs_val);
                    curr_val.set(lhs_val / rhs_val);
                }
                else if (lhs_object.is_double() && rhs_object.is_double()) {
                    double lhs_val;
                    double rhs_val;
                    lhs_object.value(lhs_val);
                    rhs_object.value(rhs_val);
                    curr_val.set(lhs_val / rhs_val);
                }
                 else if (lhs_object.is_matrix() && rhs_object.is_double()) {
                
                vector<vector<double>> x;
                lhs_object.value(x);
                double y = 0;
                 vector<vector<double>> z;
                rhs_object.value(y);
               for (int row = 0; row < x.size(); row++) {
		
			z.push_back({});
	
			for (int column = 0; column < x.at(row).size();column++) {
			z.at(row).push_back( (x.at(row).at(column) / y));


			}



		}
		curr_val = z;
                
                 
        }
         else if (lhs_object.is_matrix() && rhs_object.is_integer()) {
                
                vector<vector<double>> x;
                lhs_object.value(x);
                int y = 0;
                 vector<vector<double>> z;
                rhs_object.value(y);
               for (int row = 0; row < x.size(); row++) {
		
			z.push_back({});
	
			for (int column = 0; column < x.at(row).size();column++) {
			z.at(row).push_back( (x.at(row).at(column) / y));


			}



		}
		curr_val = z;
                
                 
        }
            }
            
            else if (node.op->lexeme() == ".*") {
            
             vector<vector<double>> x;
                lhs_object.value(x);
                vector<vector<double>> y;
                 vector<vector<double>> z;
                rhs_object.value(y);
               for (int row = 0; row < x.size(); row++) {
		
			z.push_back({});
	
			for (int column = 0; column < x.at(row).size();column++) {
			z.at(row).push_back((x.at(row).at(column) * y.at(row).at(column)));


			}



		}
		curr_val = z;
            
            
            }
            else if (node.op->lexeme() == "./") {
            
             vector<vector<double>> x;
                lhs_object.value(x);
                vector<vector<double>> y;
                 vector<vector<double>> z;
                rhs_object.value(y);
               for (int row = 0; row < x.size(); row++) {
		
			z.push_back({});
	
			for (int column = 0; column < x.at(row).size();column++) {
			z.at(row).push_back((x.at(row).at(column) / y.at(row).at(column)));


			}



		}
		curr_val = z;
            
            
            }
            else if (node.op->lexeme() == "*") {
            
            	if (rhs_object.is_integer() && lhs_object.is_matrix()) {
            	 vector<vector<double>> lhs_val;
                    int rhs_val;
                    vector<vector<double>> z;
                    lhs_object.value(lhs_val);
                    rhs_object.value(rhs_val);
                    for (int i = 0; i < lhs_val.size(); i++) {
                    	z.push_back({});
                    	for (int j = 0; j < lhs_val.at(0).size(); j++) {
                    	z.at(i).push_back(lhs_val.at(i).at(j) * rhs_val);
                    	
                    	}
                    
                    }
                    curr_val = z;
            	
            	}
            	else if (rhs_object.is_double() && lhs_object.is_matrix()) {
            	 vector<vector<double>> lhs_val;
                    double rhs_val;
                    vector<vector<double>> z;
                    lhs_object.value(lhs_val);
                    rhs_object.value(rhs_val);
                    for (int i = 0; i < lhs_val.size(); i ++) {
                    	z.push_back({});
                    	for (int j = 0; j < lhs_val.at(0).size(); j++) {
                    	z.at(i).push_back(lhs_val.at(i).at(j) * rhs_val);
                    	
                    	}
                    
                    }
                    curr_val = z;
            	
            	}
            	else if (lhs_object.is_integer() && rhs_object.is_matrix()) {
            	
            	int lhs_val;
            	vector<vector<double>> rhs_val;
                    vector<vector<double>> z;
                    lhs_object.value(lhs_val);
                    rhs_object.value(rhs_val);
                    for (int i = 0; i < rhs_val.size(); i ++) {
                    	z.push_back({});
                    	for (int j = 0; j < rhs_val.at(0).size(); j++) {
                    	z.at(i).push_back(rhs_val.at(i).at(j) * lhs_val);
                    	
                    	}
                    
                    }
                    curr_val = z;
            	
            	
            	
            	}
            	else if (rhs_object.is_matrix() && lhs_object.is_matrix()) {
            	vector<vector<double>> a;
                lhs_object.value(a);
                vector<vector<double>> b;
                rhs_object.value(b);
                vector<vector<double>> mul;
            		for(int p=0;p<a.size();p++)
				{
				mul.push_back({});
				for(int j=0;j<a.at(0).size();j++) {
					mul.at(p).push_back(0);
					mul[p][j]=0;
					for(int k=0;k<a.at(0).size();k++) {
						mul[p][j]+=a[p][k]*b[k][j];
					}
				}
			}
            	curr_val = mul;
            	}
                else if (rhs_object.is_integer() && lhs_object.is_integer()) {
                    int lhs_val;
                    int rhs_val;
                    lhs_object.value(lhs_val);
                    rhs_object.value(rhs_val);
                    curr_val.set(lhs_val * rhs_val);
                }
                else if (lhs_object.is_double()) {
                    if (rhs_object.is_matrix()) {
                     vector<vector<double>> rhs_val;
                    double lhs_val;
                    vector<vector<double>> z;
                    lhs_object.value(lhs_val);
                    rhs_object.value(rhs_val);
                    for (int i = 0; i < rhs_val.size(); i ++) {
                    	z.push_back({});
                    	for (int j = 0; j < rhs_val.at(0).size(); j++) {
                    	z.at(i).push_back(rhs_val.at(i).at(j) * lhs_val);
                    	
                    	}
                    
                    }
                    curr_val = z;
                    
                    
                    }
                    else {
                    double lhs_val;
                    double rhs_val;
                    lhs_object.value(lhs_val);
                    rhs_object.value(rhs_val);
                    curr_val.set(lhs_val * rhs_val);}
                }
                
            }
            //and cases
            else if (node.op->lexeme() == "and") {
                bool lhs_val = true;
                bool rhs_val = true;
                lhs_object.value(lhs_val);
                rhs_object.value(rhs_val);
                curr_val.set(lhs_val && rhs_val);
            }
            //or cases
            else if (node.op->lexeme() == "or") {
                bool lhs_val;
                bool rhs_val;
                lhs_object.value(lhs_val);
                rhs_object.value(rhs_val);
                curr_val.set(lhs_val || rhs_val);
            }
            //not cases
            else if (node.op->lexeme() == "not") {
                bool lhs_val;
                lhs_object.value(lhs_val);
                curr_val.set(!lhs_val);
            }
            //comparison equal cases
            else if (node.op->lexeme() == "==") {
                if (lhs_object.is_integer() && rhs_object.is_integer()) {
                    int lhs_val;
                    int rhs_val;
                    lhs_object.value(lhs_val);
                    rhs_object.value(rhs_val);
                    curr_val.set(lhs_val == rhs_val);
                }
                else if (lhs_object.is_double() && rhs_object.is_double()) {
                    double lhs_val;
                    double rhs_val;
                    lhs_object.value(lhs_val);
                    rhs_object.value(rhs_val);
                    curr_val.set(lhs_val == rhs_val);
                }
                else if (lhs_object.is_string() && rhs_object.is_string()) {
                    string lhs_val;
                    string rhs_val;
                    lhs_object.value(lhs_val);
                    rhs_object.value(rhs_val);
                    curr_val.set(lhs_val == rhs_val);
                }
                else if (lhs_object.is_char() && rhs_object.is_char()) {
                    char lhs_val;
                    char rhs_val;
                    lhs_object.value(lhs_val);
                    rhs_object.value(rhs_val);
                    curr_val.set(lhs_val == rhs_val);
                }
                else if (lhs_object.is_bool() && rhs_object.is_bool()) {
                    bool lhs_val;
                    bool rhs_val;
                    lhs_object.value(lhs_val);
                    rhs_object.value(rhs_val);
                    curr_val.set(lhs_val == rhs_val);
                }
                //LE
                else if (lhs_object.is_integer() && rhs_object.is_nil()) {
                    curr_val.set(false);
                }
                else if (lhs_object.is_double() && rhs_object.is_nil()) {
                    curr_val.set(false);
                }
                else if (lhs_object.is_string() && rhs_object.is_nil()) {
                    curr_val.set(false);
                }
                else if (lhs_object.is_char() && rhs_object.is_nil()) {
                    curr_val.set(false);
                    ;
                }
                else if (lhs_object.is_bool() && rhs_object.is_nil()) {
                    curr_val.set(false);
                }
                //RH
                else if (rhs_object.is_integer() && lhs_object.is_nil()) {

                    curr_val.set(false);
                }
                else if (rhs_object.is_double() && lhs_object.is_nil()) {
                    curr_val.set(false);
                }
                else if (rhs_object.is_string() && lhs_object.is_nil()) {
                    curr_val.set(false);
                }
                else if (rhs_object.is_char() && lhs_object.is_nil()) {
                    curr_val.set(false);
                }
                else if (rhs_object.is_bool() && lhs_object.is_nil()) {
                    curr_val.set(false);
                }
                else if (rhs_object.is_nil() && lhs_object.is_nil()) {
                    curr_val.set(true);
                }
                else if (rhs_object.is_oid() && lhs_object.is_nil()) {
                    curr_val.set(false);
                }
                else if (rhs_object.is_oid() && lhs_object.is_oid()) {
                    size_t R;
                    size_t L;
                    rhs_object.value(R);
                    lhs_object.value(L);
                    curr_val.set(R == L);
                }
                else if (rhs_object.is_nil() && lhs_object.is_oid()) {
                    curr_val.set(false);
                }
            }
            //not equal cases
            else if (node.op->lexeme() == "!=") {
                if (lhs_object.is_integer() && rhs_object.is_integer()) {
                    int lhs_val;
                    int rhs_val;
                    lhs_object.value(lhs_val);
                    rhs_object.value(rhs_val);
                    curr_val.set(lhs_val != rhs_val);
                }
                else if (lhs_object.is_double() && rhs_object.is_double()) {
                    double lhs_val;
                    double rhs_val;
                    lhs_object.value(lhs_val);
                    rhs_object.value(rhs_val);
                    curr_val.set(lhs_val != rhs_val);
                }
                else if (lhs_object.is_string() && rhs_object.is_string()) {
                    string lhs_val;
                    string rhs_val;
                    lhs_object.value(lhs_val);
                    rhs_object.value(rhs_val);
                    curr_val.set(lhs_val != rhs_val);
                }
                else if (lhs_object.is_char() && rhs_object.is_char()) {
                    char lhs_val;
                    char rhs_val;
                    lhs_object.value(lhs_val);
                    rhs_object.value(rhs_val);
                    curr_val.set(lhs_val != rhs_val);
                }
                else if (lhs_object.is_bool() && rhs_object.is_bool()) {
                    bool lhs_val;
                    bool rhs_val;
                    lhs_object.value(lhs_val);
                    rhs_object.value(rhs_val);
                    curr_val.set(lhs_val != rhs_val);
                }
                //LE
                else if (lhs_object.is_integer() && rhs_object.is_nil()) {
                    curr_val.set(true);
                }
                else if (lhs_object.is_double() && rhs_object.is_nil()) {
                    curr_val.set(true);
                }
                else if (lhs_object.is_string() && rhs_object.is_nil()) {
                    curr_val.set(true);
                }
                else if (lhs_object.is_char() && rhs_object.is_nil()) {
                    curr_val.set(true);
                }
                else if (lhs_object.is_bool() && rhs_object.is_nil()) {
                    curr_val.set(true);
                }
                //RH
                else if (rhs_object.is_integer() && lhs_object.is_nil()) {

                    curr_val.set(true);
                }
                else if (rhs_object.is_double() && lhs_object.is_nil()) {
                    curr_val.set(true);
                }
                else if (rhs_object.is_string() && lhs_object.is_nil()) {
                    curr_val.set(true);
                }
                else if (rhs_object.is_char() && lhs_object.is_nil()) {
                    curr_val.set(true);
                }
                else if (rhs_object.is_bool() && lhs_object.is_nil()) {
                    curr_val.set(true);
                }
                else if (rhs_object.is_nil() && lhs_object.is_nil()) {
                    curr_val.set(false);
                }
                else if (rhs_object.is_oid() && lhs_object.is_nil()) {
                    curr_val.set(true);
                }
                else if (rhs_object.is_oid() && lhs_object.is_oid()) {
                    size_t R;
                    size_t L;
                    rhs_object.value(R);
                    lhs_object.value(L);
                    curr_val.set(R != L);
                }
                else if (rhs_object.is_nil() && lhs_object.is_oid()) {
                    curr_val.set(true);
                }
                else if (rhs_object.is_nil() && lhs_object.is_nil()) {
                    curr_val.set(false);
                }
            }
            //Less than cases
            else if (node.op->lexeme() == "<") {
                if (lhs_object.is_integer() && rhs_object.is_integer()) {
                    int lhs_val;
                    int rhs_val;
                    lhs_object.value(lhs_val);
                    rhs_object.value(rhs_val);
                    curr_val.set(lhs_val < rhs_val);
                }
                else if (lhs_object.is_double() && rhs_object.is_double()) {
                    double lhs_val;
                    double rhs_val;
                    lhs_object.value(lhs_val);
                    rhs_object.value(rhs_val);
                    curr_val.set(lhs_val < rhs_val);
                }
                else if (lhs_object.is_string() && rhs_object.is_string()) {
                    string lhs_val;
                    string rhs_val;
                    lhs_object.value(lhs_val);
                    rhs_object.value(rhs_val);
                    curr_val.set(lhs_val < rhs_val);
                }
                else if (lhs_object.is_char() && rhs_object.is_char()) {
                    char lhs_val;
                    char rhs_val;
                    lhs_object.value(lhs_val);
                    rhs_object.value(rhs_val);
                    curr_val.set(lhs_val < rhs_val);
                }
            }
            //greater than cases
            else if (node.op->lexeme() == ">") {
                if (lhs_object.is_integer() && rhs_object.is_integer()) {
                    int lhs_val;
                    int rhs_val;
                    lhs_object.value(lhs_val);
                    rhs_object.value(rhs_val);
                    curr_val.set(lhs_val > rhs_val);
                }
                else if (lhs_object.is_double() && rhs_object.is_double()) {
                    double lhs_val;
                    double rhs_val;
                    lhs_object.value(lhs_val);
                    rhs_object.value(rhs_val);
                    curr_val.set(lhs_val > rhs_val);
                }
                else if (lhs_object.is_string() && rhs_object.is_string()) {
                    string lhs_val;
                    string rhs_val;
                    lhs_object.value(lhs_val);
                    rhs_object.value(rhs_val);
                    curr_val.set(lhs_val > rhs_val);
                }
                else if (lhs_object.is_char() && rhs_object.is_char()) {
                    char lhs_val;
                    char rhs_val;
                    lhs_object.value(lhs_val);
                    rhs_object.value(rhs_val);
                    curr_val.set(lhs_val > rhs_val);
                }

                //LE
            }
            //greater than or equal cases
            else if (node.op->lexeme() == ">=") {
                if (lhs_object.is_integer() && rhs_object.is_integer()) {
                    int lhs_val;
                    int rhs_val;
                    lhs_object.value(lhs_val);
                    rhs_object.value(rhs_val);
                    curr_val.set(lhs_val >= rhs_val);
                }
                else if (lhs_object.is_double() && rhs_object.is_double()) {
                    double lhs_val;
                    double rhs_val;
                    lhs_object.value(lhs_val);
                    rhs_object.value(rhs_val);
                    curr_val.set(lhs_val >= rhs_val);
                }
                else if (lhs_object.is_string() && rhs_object.is_string()) {
                    string lhs_val;
                    string rhs_val;
                    lhs_object.value(lhs_val);
                    rhs_object.value(rhs_val);
                    curr_val.set(lhs_val >= rhs_val);
                }
                else if (lhs_object.is_char() && rhs_object.is_char()) {
                    char lhs_val;
                    char rhs_val;
                    lhs_object.value(lhs_val);
                    rhs_object.value(rhs_val);
                    curr_val.set(lhs_val >= rhs_val);
                }
            }
            //Less than or equal cases
            else if (node.op->lexeme() == "<=") {
                if (lhs_object.is_integer() && rhs_object.is_integer()) {
                    int lhs_val;
                    int rhs_val;
                    lhs_object.value(lhs_val);
                    rhs_object.value(rhs_val);

                    curr_val.set(lhs_val <= rhs_val);
                }
                else if (lhs_object.is_double() && rhs_object.is_double()) {
                    double lhs_val;
                    double rhs_val;
                    lhs_object.value(lhs_val);
                    rhs_object.value(rhs_val);
                    curr_val.set(lhs_val <= rhs_val);
                }
                else if (lhs_object.is_string() && rhs_object.is_string()) {
                    string lhs_val;
                    string rhs_val;
                    lhs_object.value(lhs_val);
                    rhs_object.value(rhs_val);
                    curr_val.set(lhs_val <= rhs_val);
                }
                else if (lhs_object.is_char() && rhs_object.is_char()) {
                    char lhs_val;
                    char rhs_val;
                    lhs_object.value(lhs_val);
                    rhs_object.value(rhs_val);
                    curr_val.set(lhs_val <= rhs_val);
                }
            }
        }
    }
}
//just accept the next rvalue
void Interpreter::visit(SimpleTerm& node)
{
    node.rvalue->accept(*this);
}
//Complex term just accepts a new expr
void Interpreter::visit(ComplexTerm& node)
{
    node.expr->accept(*this);
}
// rvalues
void Interpreter::visit(SimpleRValue& node)
{
    if (node.value.type() == CHAR_VAL) {
        curr_val.set(node.value.lexeme().at(0));
    }

    else if (node.value.type() == STRING_VAL) {
        curr_val.set(node.value.lexeme());
    }
    else if (node.value.type() == INT_VAL) {
        try {
            curr_val.set(stoi(node.value.lexeme()));
        }
        catch (const invalid_argument& e) {
            error("internal error", node.value);
        }
        catch (const out_of_range& e) {
            error("Int out of range", node.value);
        }
    }
    else if (node.value.type() == DOUBLE_VAL) {
        try {
            curr_val.set(stod(node.value.lexeme()));
        }
        catch (const invalid_argument& e) {
            error("internal error", node.value);
        }
        catch (const out_of_range& e) {
            error("Double out of range", node.value);
        }
    }
    else if (node.value.type() == BOOL_VAL) {

        if (node.value.lexeme() == "true")
            curr_val.set(true);
        else {
            curr_val.set(false);
        }
    }
    else if (node.value.type() == NIL) {
        curr_val.set_nil();
    }
}

void Interpreter::visit(NewRValue& node)
{
//Create and define a new heap object.  do not store in symbol table along with oid yet because we don't have an variable name
    HeapObject t1;
    size_t new_oid = next_oid;
    TypeDecl* my_type_decl = types[node.type_id.lexeme()];
    int current_environment = sym_table.get_environment_id();
    sym_table.set_environment_id(global_env_id);
    sym_table.push_environment();
    //Initialize udt
    for (VarDeclStmt* iter : my_type_decl->vdecls) {
        iter->accept(*this);
        t1.set_att(iter->id.lexeme(), curr_val);
    }
    sym_table.pop_environment();
    sym_table.set_environment_id(current_environment);
    next_oid++;
    heap.set_obj(new_oid, t1);
    curr_val.set(new_oid);
}

void Interpreter::visit(CallExpr& node)
{
//Built in functions
    std::string fun_name = node.function_id.lexeme();
    if (fun_name == "print") {
        node.arg_list.front()->accept(*this);
        std::string s = curr_val.to_string();
        s = std::regex_replace(s, std::regex("\\\\n"), "\n");
        s = std::regex_replace(s, std::regex("\\\\t"), "\t");
        std::cout << s;
    }
  
    else if (fun_name == "m_print") {
    node.arg_list.front()->accept(*this);
    vector<vector<double>> x;
    
    curr_val.value(x);
    for (int row = 0; row < x.size(); row++) {
    	cout << endl;
    	for (int column = 0; column < x.at(row).size(); column++) {
    	cout << x.at(row).at(column) << " "; 
    	
    	
    	}
    
    }
    cout << endl;
    
    }
      else if (fun_name == "m_singleton") {
      int iterator3 = 0;
      vector<DataObject> list;
    for (Expr* iter : node.arg_list) {
            if (iterator3 < node.arg_list.size()) {
                iter->accept(*this);
                list.push_back(curr_val);
            }
            iterator3++;
        }
    
    vector<vector<double>> x;
   int R;
   int C;
    double V;
    list.at(1).value(R);
    list.at(2).value(C);
    list.at(0).value(V);
  
   
    for (int row = 0; row < R; row++) {
    	x.push_back({});
    	for (int column = 0; column < C; column++) {
    	x.at(row).push_back(V);
    	
    	
    	}
    
    }
    
    curr_val = x;
    
    }
    else if (fun_name == "stod") {
        node.arg_list.front()->accept(*this);
        string string_arg = "";
        curr_val.value(string_arg);
        curr_val.set(stod(string_arg));
    }
    else if (fun_name == "stoi") {
        node.arg_list.front()->accept(*this);
        string string_arg = "";
        curr_val.value(string_arg);
        curr_val.set(stoi(string_arg));
    }
    else if (fun_name == "dtos") {
        node.arg_list.front()->accept(*this);
        double string_arg = 0.0;
        curr_val.value(string_arg);
        curr_val.set(to_string(string_arg));
    }
    else if (fun_name == "itos") {
        node.arg_list.front()->accept(*this);
        int string_arg = 0.0;
        curr_val.value(string_arg);
        curr_val.set(to_string(string_arg));
    }
    else if (fun_name == "read") {
        string string_arg = "";
        cin.clear();
        getline(cin, string_arg);
        cin.clear();
        if (string_arg.size() > 0) {
            if (string_arg.at(string_arg.size() - 1) == '\n') {
                string_arg.pop_back();
            }
        }
        curr_val.set(string_arg);
    }
    else if (fun_name == "length") {
        node.arg_list.front()->accept(*this);
        string string_arg = "";
        curr_val.value(string_arg);
        curr_val.set(string_arg.size());
    }
    else if (fun_name == "m_get") {
        vector<DataObject> list;
        vector<vector<double>> M;
        int col;
        int row;
        int iterator3 = 0;
        for (Expr* iter : node.arg_list) {
            if (iterator3 < node.arg_list.size()) {
                iter->accept(*this);
                list.push_back(curr_val);
            }
            iterator3++;
        }
	list.at(0).value(M);
	list.at(1).value(row);
	list.at(2).value(col);
	if (row >= M.size() || col >= M.at(0).size()) {
	error("Accessing out of bounds matrix",node.function_id);
	
	}
	curr_val = M.at(row).at(col);
	
    }
    else if (fun_name == "get") {
        vector<DataObject> list;
        int iterator3 = 0;
        for (Expr* iter : node.arg_list) {
            if (iterator3 < node.arg_list.size()) {
                iter->accept(*this);
                list.push_back(curr_val);
            }
            iterator3++;
        }
        string string_arg = "";
        DataObject second_arg = list.at(1);
        second_arg.value(string_arg);
        int first_arg = 0;
        DataObject first_arg_object = list.at(0);
        first_arg_object.value(first_arg);
        curr_val.set(string_arg.at(first_arg));
    }
    else {
        // call the function
        // 1. evaluate the args and save
        int iterator = 0;
        list<DataObject> resolved_args;
        for (Expr* iter : node.arg_list) {
            iter->accept(*this);
            resolved_args.push_back(curr_val);
            ++iterator;
        }
        // 2. save the current environment
        int curr_environment = sym_table.get_environment_id();
        // 3. go to the gobal environment
        sym_table.set_environment_id(global_env_id);
        // 4. push a new environment
        sym_table.push_environment();
        // 5. add param values ( from 1)
        int iterator1 = 0;
        for (FunDecl::FunParam iter : functions[fun_name]->params) {
            sym_table.add_name(iter.id.lexeme());
            DataObject resolved_arg = resolved_args.front();
            sym_table.set_val_info(iter.id.lexeme(), resolved_arg);
            resolved_args.pop_front();
        }
        // 6. eval each statement
        for (Stmt* stmt_iter : functions[fun_name]->stmts) {
            try {
                stmt_iter->accept(*this);
            }
            catch (const MyPLReturnException* e) {
                if (fun_name == "main") {
                    curr_val.value(ret_code);
                    
                }
            }
        }
        sym_table.pop_environment();
        // 9. return to saved environment
        sym_table.set_environment_id(curr_environment);
        FunDecl* fun_node = functions[fun_name];
    }
}
void Interpreter::visit(IDRValue& node)
{
//If it is a udt...
    if (node.path.size() > 1) {
        DataObject t1;
        size_t my_oid;
        HeapObject x;
        sym_table.get_val_info(node.path.front().lexeme(), t1);
        t1.value(my_oid);
        int iterator = 0;
        heap.get_obj(my_oid, x);
        if (!t1.is_nil()) {
            for (Token iter : node.path) {//Navigate to the end of the attribute path
                if (t1.is_nil()) {
                    curr_val.set_nil();
                    break;
                }
                if (iterator > 0 && iterator < node.path.size() - 1) {
                    x.get_val(iter.lexeme(), t1);
                    if (t1.is_nil()) {
                        curr_val.set_nil();
                        break;
                    }
                    t1.value(my_oid);
                    heap.get_obj(my_oid, x);
                }

                iterator++;
            }
            //return attribute value in curr_val
            if (t1.is_nil()) {
                curr_val.set_nil();
            }
            else {
                x.get_val(node.path.back().lexeme(), t1);
                curr_val = t1;
            }
        }
    }
    else {
    //if not a udt set curr val to the variables value
        DataObject P;
        sym_table.get_val_info(node.path.front().lexeme(), P);
        curr_val = P;
    }
}
void Interpreter::visit(NegatedRValue& node)
{
//Negate integer or double rvalue
    node.expr->accept(*this);
    if (curr_val.is_integer()) {
        int value = 0;
        curr_val.value(value);
        curr_val.set(-1 * value);
    }
    else if (curr_val.is_double()) {
        double value = 0;
        curr_val.value(value);
        curr_val.set(-1 * value);
    }
}

void Interpreter::visit(TransposedRValue& node) {
node.expr->accept(*this);
vector<vector<double>> N;
vector<vector<double>> O;
curr_val.value(O);

	for (int i = 0; i < O.at(0).size(); i++) {
		N.push_back({});
		for (int j = 0; j < O.size(); j++) {
		N.at(i).push_back(O.at(j).at(i));
		
		}
	
	
	}
	curr_val = N;


}
#endif
