//----------------------------------------------------------------------
// NAME:Zachary Craig
// FILE:Printer.h
// DATE:12/24/2021
// DESC:Implementation of pretty-print object.  Navigates AST tree to produce pretty-printed mypl code.
//----------------------------------------------------------------------

#ifndef PRINTER_H
#define PRINTER_H

#include <iostream>
#include "ast.h"

class Printer : public Visitor {
public:
    // constructor
    Printer(std::ostream& output_stream)
        : out(output_stream)
    {
    }

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
    void visit(TransposedRValue& node);
private:
    std::ostream& out;
    int indent = 0;

    void inc_indent() { indent += 3; }
    void dec_indent() { indent -= 3; }
    std::string get_indent() { return std::string(indent, ' '); }
};
// TODO: Implement the visitor functions
//Visit root node

void Printer::visit(Program& node)
{
    //Returns if ast is empty
    if (node.decls.size() == 0) {
        return;
    }
    //Iterate through each declaration
    for (Decl* iter : node.decls) {
        iter->accept(*this);
    }
}

//visit function declaration
void Printer::visit(FunDecl& node)
{
    string s = "fun " + node.return_type.lexeme() + " " + node.id.lexeme() + "(";
    //Output function parameters
    for (FunDecl::FunParam iter : node.params) {
        s += iter.id.lexeme() + ": " + iter.type.lexeme();
        s += ", ";
    }
    //Delete extra comma
    if (node.params.size() != 0) {
        s.erase(s.size() - 1);
        s.erase(s.size() - 1);
    }
    s += ")";
    out << s << endl;
    inc_indent();
    //Output function body
    for (Stmt* iter : node.stmts) {
        out << get_indent();
        iter->accept(*this);
        out << endl;
    }
    dec_indent();
    out << "end" << endl
        << endl;
}

//Visit Type Declaration
void Printer::visit(TypeDecl& node)
{
    string s;
    s += "type " + node.id.lexeme() + '\n';
    out << s;
    inc_indent();
    //output all types
    for (VarDeclStmt* iter : node.vdecls) {
        out << get_indent();
        iter->accept(*this);
        out << endl;
    }
    dec_indent();
    out << "end" << endl
        << endl;
}
// var declaration statements
void Printer::visit(VarDeclStmt& node)
{
    string s = "var " + node.id.lexeme();
    if (node.type != nullptr) {
        s += ": " + node.type->lexeme();
    }
    s += " = ";
    out << s;
    node.expr->accept(*this);
}
//Visit Assignment statement
void Printer::visit(AssignStmt& node)
{
    string s = "";
    int inter = 0;
    //lhs
    for (Token iter : node.lvalue_list) {
        out << iter.lexeme();
        if (inter >= 0 && inter < node.lvalue_list.size() - 1) {
            out << ".";
        }
        inter++;
    }
    out << " "
        << "="
        << " ";
    //rhs
    node.expr->accept(*this);
}
//Visit Return Statement
void Printer::visit(ReturnStmt& node)
{
    out << "return ";
    node.expr->accept(*this);
}
//Visit if statement
void Printer::visit(IfStmt& node)
{
    out << "if"
        << " ";
    node.if_part->expr->accept(*this);
    out << " then " << endl;
    inc_indent();
    //First part of if then
    for (Stmt* iter2 : node.if_part->stmts) {
        out << get_indent();
        iter2->accept(*this);
        out << endl;
    }
    dec_indent();
    //else if output
    if (node.else_ifs.size() != 0) {
        for (BasicIf* iter : node.else_ifs) {
            out << get_indent() << "elseif"
                << " ";
            iter->expr->accept(*this);
            out << " then " << endl;
            inc_indent();
            list<Stmt*> mango_stmt = iter->stmts;
            for (Stmt* iter2 : mango_stmt) {
                out << get_indent();
                iter2->accept(*this);
                out << endl;
            }
            dec_indent();
        }
    }
    //else output
    if (node.body_stmts.size() != 0) {
        out << get_indent() << "else" << endl;
        inc_indent();
        for (Stmt* iter2 : node.body_stmts) {
            out << get_indent();

            iter2->accept(*this);
            out << endl;
        }
        dec_indent();
    }
    out << get_indent() << "end";
}
//Visit while Stmt
void Printer::visit(WhileStmt& node)
{
    out << "while ";
    //Params of while stmt
    node.expr->accept(*this);
    out << "";
    out << " do" << endl;
    inc_indent();
    //Body of while statement
    for (Stmt* iter : node.stmts) {
        out << get_indent();
        iter->accept(*this);
        out << endl;
    }
    dec_indent();
    out << get_indent() << "end";
}
//Visit for stmt
void Printer::visit(ForStmt& node)
{
    out << "for " << node.var_id.lexeme() << " ";
    out << "= ";
    //first expr of for argument
    node.start->accept(*this);
    out << " "
        << "to"
        << " ";
    //second expr of for argument
    node.end->accept(*this);
    out << endl;
    inc_indent();
    for (Stmt* iter : node.stmts) {
        out << get_indent();
        iter->accept(*this);
        out << endl;
    }
    dec_indent();
    out << get_indent() << "end";
}

//Visit expression
// expressions
void Printer::visit(Expr& node)
{
    string s = "";
    //Checks for non-single rvalue expression to add (
    if (node.op != nullptr) {
        out << "(";
    }
    if (node.negated == true) {
        out << "not ";
        node.first->accept(*this);
    }
    else {
        node.first->accept(*this);
    }
    if (node.op != nullptr) {
        out << " " << node.op->lexeme() << " ";
    }
    if (node.rest != nullptr) {
        node.rest->accept(*this);
    }
    if (node.op != nullptr) {

        out << ")";
    }
}

//Visit Simple Term
void Printer::visit(SimpleTerm& node)
{
    node.rvalue->accept(*this);
}

//Visit Simple Term
void Printer::visit(ComplexTerm& node)
{
    node.expr->accept(*this);
}

// Visit SimpleRVAlue
void Printer::visit(SimpleRValue& node)
{
    out << node.value.lexeme();
}

//Visit NewRValue Term
void Printer::visit(NewRValue& node)
{
    out << "new ";
    out << node.type_id.lexeme();
}

//Visit Call Expr
void Printer::visit(CallExpr& node)
{
    out << node.function_id.lexeme();
    out << "(";
    int inter = 0;
    for (Expr* iter : node.arg_list) {
        iter->accept(*this);
        if (inter < node.arg_list.size() - 1) {
            out << ", ";
        }
        inter++;
    }
    out << ")";
}

//visit IDRValue
void Printer::visit(IDRValue& node)
{
    string s = "";
    int pass_by = 0;
    for (Token iter : node.path) {
        out << iter.lexeme();
        if (pass_by >= 0 && pass_by < node.path.size() - 1) {
            out << ".";
        }
        pass_by += 1;
    }
}

//visit NegatedRValue
void Printer::visit(NegatedRValue& node)
{
    out << "neg ";
    node.expr->accept(*this);
}
void Printer::visit(TransposedRValue& node) {


}
#endif
