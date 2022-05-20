#ifndef PARSER_H_
#define PARSER_H_


#include "AST.h"

// The typedef below makes the name "translatedResult"
//  mean a "Tree" object from our Expr_Node type heirarchy.
typedef ExprNode *ParserResult;

ParserResult matchStartSymbolAndEOF();
static ParserResult matchE();
static ParserResult matchEinParens();
static ParserResult matchEinBrackets();
static std::string matchOp();
list<ExprNode *> reverse(list<ExprNode *> list, ::list<ExprNode *> newList);

#endif /*PARSER_H_*/
