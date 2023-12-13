 //----------------------------------------------------------------------
// NAME:Zachary Craig
// FILE:Lexer.h
// DATE:2/2/2021
// DESC:Implementation of lexer.  Essentially uses next_token function to grab the next token from input stream
//----------------------------------------------------------------------

#ifndef LEXER_H
#define LEXER_H

#include <istream>
#include <string>
#include "token.h"
#include "mypl_exception.h"
using namespace std;
class Lexer {
public:
    // construct a new lexer from the input stream
    Lexer(std::istream& input_stream);

    // return the next available token in the input stream (including
    // EOS if at the end of the stream)
    Token next_token();

private:
    // input stream, current line, and current column
    std::istream& input_stream;
    int line;
    int column;

    // return a single character from the input stream and advance
    char read();

    // return a single character from the input stream without advancing
    char peek();

    // create and throw a mypl_exception (exits the lexer)
    void error(const std::string& msg, int line, int column) const;
};
//Constructor
Lexer::Lexer(std::istream& input_stream)
    : input_stream(input_stream)
    , line(1)
    , column(1)
{
}
//Moves cursor and returns char from input stream
char Lexer::read()
{
    return input_stream.get();
}
//Peeks char but does not move cursor
char Lexer::peek()
{
    return input_stream.peek();
}
//Throws exception with comment
void Lexer::error(const std::string& msg, int line, int column) const
{
    throw MyPLException(LEXER, msg, line, column);
}

//Takes grabs and returns next token
Token Lexer::next_token()
{
//Lexeme which accumulates chars
    std::string lexeme = "";
    char ch; //char grabbed from input stream
    char peeker;//peeked char
    if (peek() == EOF) {
        return Token(EOS, "", line, column);
    }
    //Loop to accumulate chars into a single lexeme
    while (true) {
  
        bool found_dot = false;//bool used to determine if a number is double or int
        int column_holder = 0;//Holds start of string
        int line_holder = 0;//Holds start of line/row
        ch = read();
        column++;
        lexeme += ch;
        if (isspace(ch)) {
            lexeme = "";
            while (isspace(ch)) {
                lexeme = "";
                if (isspace(ch) && ch != ' ') {
                    column = 1;
                    line++;
                    if(ch != '\n') {
                    --line;
                    
                    }
                }
                else if (ch == ' ') {
                    column++;
                }
                ch = read();
            }
            lexeme += ch;
        }
        if (ch == '#') {
            while (!isspace(ch) || ch ==' ') {
                ch = read();
                column++;
            }
            line++;
            if(ch != '\n') {
            --line;
            }
            column = 1;
        }
      
   
        if (ch == '(') {
        	lexeme = "";
            return Token(LPAREN, "(", line, column);
        }
        if (ch == ')') {
        	lexeme = "";
            return Token(RPAREN, ")", line, column);
        }
       
        if (ch == EOF) {
            lexeme = "";
            return Token(EOS, "", line, column);
        }
        if (ch == '.') {
        
        	if (peek() == '/') {
        	read();
        	lexeme = "";
        	return Token(DOT_DIVIDE, "./", line, column);
        	
        	}
        	else if (peek() == '*') {
        	read();
        	lexeme = "";
        	return Token(DOT_MULTIPLY, ".*", line, column);
        	
        	}
        	else if (peek() == '^') {
        	read();
        	lexeme = "";
        	return Token(DOT_EXPO, ".^", line, column);
        	
        	}
        	
        
        
            lexeme = "";
            line_holder = line;
            column_holder = column;
            if (isdigit(peek())) {
                lexeme += ch;
                while (!isspace(ch)) {
                    ch = read();
                    column++;
                    lexeme += ch;
                    if (!isdigit(ch)) {
                        error("Incorrect Syntax for a double value at ", line_holder, column_holder - 1);
                    }
                }
                return Token(DOUBLE_VAL, lexeme, line_holder, column_holder);
            }
            lexeme = "";
            return Token(DOT, ".", line, column);
        }
        if (ch == ',') {
            lexeme = "";
            return Token(COMMA, ",", line, column);
        }
        if (ch == '[') {
            lexeme = "";
            return Token(L_BRACKET, "[", line, column);
        }
        if (ch == ']') {
            lexeme = "";
         
            return Token(R_BRACKET, "]", line, column);
        }
        if (ch == ':') {
            lexeme = "";
            return Token(COLON, ":", line, column);
        }
         if (ch == ';') {
            lexeme = "";
            return Token(SEMICOLON, ";", line, column);
        }
        if (ch == '+') {
            lexeme = "";
            return Token(PLUS, "+", line, column);
        }
        if (ch == '-') {
            lexeme = "";
            return Token(MINUS, "-", line, column);
        }
        if (ch == '*') {
            lexeme = "";
            return Token(MULTIPLY, "*", line, column);
        }
        if (ch == '/') {
            lexeme = "";
            return Token(DIVIDE, "/", line, column);
        }
        if (ch == '%') {
            lexeme = "";
            return Token(MODULO, "%", line, column);
        }
        if (ch == '~') {
        lexeme = "";
        return Token(TRANSPOSE,"~",line,column);
        
        }
         if (ch == '^') {
        
        lexeme = "";
        return Token(EXPO,"^",line,column);
        
        }
        if (ch == '=') {
            if (isspace(peek()) || isalpha(peek()) || isdigit(peek()) || peek() == '"' || peek() == '(' || peek() == ')') {
                lexeme = "";
                return Token(ASSIGN, "=", line, column);
            }

            if (peek() == '=') {
                read();
                column++;
                return Token(EQUAL, "==", line, column - 1);
            }
        }
        if (ch == '>') {
            if (peek() == '=') {
                read();
                column++;
                return Token(GREATER_EQUAL, ">=", line, column - 1);
            }
            else if (peek() == ' ') {
                lexeme = "";
                return Token(GREATER, ">", line, column);
            }
        }
        if (ch == '<') {
            if (peek() == '=') {
                read();
                column++;
                return Token(LESS_EQUAL, "<=", line, column - 1);
            }
            else if (peek() == ' ') {
                lexeme = "";
                return Token(LESS, "<", line, column);
            }
        }
        if (ch == '!') {
            if (peek() == '=') {
                column++;
                read();
                return Token(NOT_EQUAL, "!=", line, column - 1);
            }
            else {
                error("Error was found expecting =", line, column);
            }
        }
        if (ch == '\'') {
            line_holder = line;
            column_holder = column;
            lexeme = "";
            ch = read();
            column++;
            lexeme += ch;
            while (ch != '\'') {
                if (isspace(ch) || ch == EOF) {
                    error("Error was found expecting '", line_holder, column_holder - 1);
                }
                ch = read();
                column++;
            }
           
            return Token(CHAR_VAL, lexeme, line_holder, column_holder - 1);
        }
        
        if (ch == '"') {
            lexeme = "";
            
            if (peek() == '"') {
                read();
                
                column++;
              
            
              
                return Token(STRING_VAL, lexeme, line, column - 1);
            }
            line_holder = line;
            column_holder = column;
            ch = read();
            lexeme += ch;
            column++;
            while (ch != '"') {
                if (ch == EOF) {
                    error("Error was found expecting \"", line_holder, column_holder);
                }
                ch = read();
                column++;
                lexeme += ch;
            }
            if (lexeme.at(lexeme.size() - 1) == '"') {
              lexeme.pop_back();
              
              }
           
              
            return Token(STRING_VAL, lexeme, line_holder, column_holder);
        }
        if (isdigit(ch)) {
        
            lexeme = "";
            line_holder = line;
            column_holder = column;
            lexeme += ch;
            while (peek() != ' ' && peek() != '\n' && peek() != '#' && peek() != '"' && peek() != '\'' && peek() != '('&& peek() != ')' && peek() != '-' && peek() != '%' && peek() != '+' && peek() != '/' && peek() != '*' && peek() != ',' && peek() != ';' && peek() != ']' && peek() != '[') {
                ch = read();
                column++;
                lexeme += ch;
                if (ch == '.') {
                    found_dot = !found_dot;
                }
                
            }
            if (found_dot == false) {
           
           
                return Token(INT_VAL, lexeme, line_holder, column_holder - 1);
            }
            else {
        
                return Token(DOUBLE_VAL, lexeme, line_holder, column_holder - 1);
            }
        }
        if (isalpha(ch)) {
            lexeme = "";
            line_holder = line;
            column_holder = column;
            lexeme += ch;
            while (!isspace(peek()) && (peek() == '_' ||isalpha(peek()) || isdigit(peek())) && peek() != '=') {
                ch = read();
                column++;
                lexeme += ch;
                
                if (ch == '"') {
                    error("Invalid syntax use of \"", line_holder, column_holder);
                }
            }
            
          if (lexeme == "nil") {
                lexeme = "";
                return Token(NIL, "nil", line, column - 3);
            }
            if (lexeme == "and" ) {
                lexeme = "";
                return Token(AND, "and", line, column - 3);
            }
            if (lexeme == "neg") {
                lexeme = "";
                return Token(NEG, "neg", line, column - 3);
            }
            if (lexeme == "not") {
                lexeme = "";
                return Token(NOT, "not", line, column - 3);
            }
            if (lexeme == "type") {
                lexeme = "";
                return Token(TYPE, "type", line, column - 4);
            }
            if (lexeme == "while") {
                lexeme = "";
                return Token(WHILE, "while", line, column - 5);
            }
            if (lexeme == "for") {
                lexeme = "";
                return Token(FOR, "for", line, column - 2);
            }
            if (lexeme == "or") {
                lexeme = "";
                return Token(OR, "or", line, column - 1);
            }
            if (lexeme == "to") {
                lexeme = "";
                return Token(TO, "to", line, column - 1);
            }
            if (lexeme == "do") {
                return Token(DO, "do", line, column - 1);
            }
            if (lexeme == "double") {
                return Token(DOUBLE_TYPE, "double", line, column - 6);
            }
            if (lexeme == "if") {
                lexeme = "";
                return Token(IF, "if", line, column - 1);
            }
            if (lexeme == "then") {
                lexeme = "";
                return Token(THEN, "then", line, column - 3);
            }
            if (lexeme == "else") {
                
                lexeme = "";
                return Token(ELSE, "else", line, column - 3);
            }
            if (lexeme == "elseif") {
                    
               
                   
                   
                    return Token(ELSEIF, "elseif", line, column - 5);
                }
            
            if (lexeme == "end") {
                lexeme = "";
                return Token(END, "end", line, column - 2);
            }
            if (lexeme == "fun" && isspace(peek())) {
                lexeme = "";
                return Token(FUN, "fun", line, column - 2);
            }
            if (lexeme == "var" && isspace(peek())) {
                lexeme = "";
                return Token(VAR, "var", line, column - 2);
            }
            if (lexeme == "return" && isspace(peek())) {
                lexeme = "";
                return Token(RETURN, "return", line, column - 5);
            }
            if (lexeme == "new" && isspace(peek())) {
                lexeme = "";
                return Token(NEW, "new", line, column - 2);
            }
            if (lexeme == "bool" && isspace(peek())) {
                lexeme = "";
                return Token(BOOL_TYPE, "bool", line, column - 3);
            }
            if (lexeme == "int" && isspace(peek())) {
                lexeme = "";
                return Token(INT_TYPE, "int", line, column - 3);
            }
            if (lexeme == "matrix" && isspace(peek())) {
            lexeme = "";
            return Token(MATRIX_TYPE, "matrix", line, column - 6);
            
            
            }
            
            if (lexeme == "char" && isspace(peek())) {
                lexeme = "";
                
                return Token(CHAR_TYPE, "char", line, column - 4);
            }
            if (lexeme == "string" && isspace(peek())) {
                lexeme = "";
                return Token(STRING_TYPE, "string", line, column - 6);
            }
            if (lexeme == "true") {
                lexeme = "";
           
                return Token(BOOL_VAL, "true", line, column - 4);
            }
            if (lexeme == "false") {
                lexeme = "";
              
                return Token(BOOL_VAL, "false", line, column - 4);
            }
          
            return Token(ID, lexeme, line_holder, column_holder -1);
        }
    }
}
#endif
