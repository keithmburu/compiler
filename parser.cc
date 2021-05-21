#include <iostream>
#include <cstdlib>  // for 'exit' function to abandon the program
#include <hc_list_helpers.h>
#include "parser.h"
#include "scanner.h"

using std::string;
using HaverfordCS::list;
using HaverfordCS::ez_list;
using std::cout;
using std::cerr;
using std::endl;

#include "streams.h"

// match this:
// START -> E   <followed by end-of-input>
// E -> integer
// E -> boolean
// E -> identifier
// E -> ( E_IN_PARENS )
// E_IN_PARENS -> OP E E
// E_IN_PARENS -> declarations
// declarations -> [ E_IN_BRACKETS ] declarations
// declarations -> ""
// E_IN_PARENS -> EXIT
// E_IN_PARENS -> GETINT   // New in this version, gets user input
// E_IN_PARENS -> if
// if -> E E E

// OP --> +|-|*|<=|=|>= OP_COMPARE

// Declare all functions, so they can call each other in any order
//  (note: "static" means it's restricted to this scope, e.g. this file
static ParserResult matchE();
static ParserResult matchEInParens();
static string matchOp();


// FIRST and FOLLOW sets for those,
//  built with lists rather than sets, for familiarity
//  defined in terms of kindOfToken from scanner-regexp.h
static list<kindOfToken> FIRST_OP  = ez_list(PLUS, MINUS, TIMES, OP_COMPARE);
static list<kindOfToken> FIRST_EIP = FIRST_OP;
static list<kindOfToken> FIRST_E   = ez_list(INT_LITERAL, BOOL_LITERAL, IDENTIFIER, LPAREN, LBRACKET);

static list<kindOfToken> FOLLOW_OP  = FIRST_E;
static list<kindOfToken> FOLLOW_EIP = ez_list(RPAREN);
static list<kindOfToken> FOLLOW_E   = /* append(FOLLOW_EIP, FIRST_E) */
		                              list<kindOfToken>(RPAREN, FIRST_E);
	                                  /* Dave should put "append" into list_helpers */

// first, some helpful functions:

// mustGetNextToken:
//    we call this when we need to get more input and we must find something there,
//    i.e. when we want to move current_token along and end-of-input would be an error
void mustGetNextToken()
{
	getNextToken();
	if (!tokenAvailable()) {
		cerr << "unexpected end of input at token #" << tokenNumber() << endl;
		exit(5);
	}
}


// currentTokenThenMove
//   record currentToken (which must NOT be end-of-input), then move past it, return what it was
static std::string currentTokenThenMove()
{
	if (!tokenAvailable()) {
		cerr << "unexpected end of input at token #" << tokenNumber() << endl;
		exit(5);
	}
	std::string curr = currentToken();
	getNextToken();
	return curr;
}

// confirmLiteral
//   match a literal, assuming the token HAS been scanned already,
//    i.e. that "currentToken" is _on_ the literal we wish to match
//   leave "currentToken" on the very last token of the matched pattern ... this is not a "match" function
static void confirmLiteral(string what)
{
	if (!tokenAvailable()) {
		cerr << "unexpected end of input at token #" << tokenNumber() << endl;
		exit(5);
	}
	if (currentToken() != what) {
		cerr << "got " << currentToken() << " instead of " << what << " at token #" << tokenNumber() << endl;
		exit(2);
	}	
}


// match an "E", i.e, anything on the right hand side of any "E-->..." production
//  assume the first token of the E has been scanned
//   (i.e., assuming currentToken is the first token of the "E" we're matching)
//  leave "currentToken" AFTER the very last token of the matched pattern
static ParserResult matchE()
{
	trace << "Entering matchE, current token is " << currentToken() << endl;
	if (currentTokenKind() == INT_LITERAL) {
        return new IntLiteralNode(std::stoi(currentTokenThenMove()));
    } else if (currentTokenKind() == BOOL_LITERAL) {
        return new BoolLiteralNode(currentTokenThenMove());
	} else if (currentTokenKind() == IDENTIFIER) {
        return new VarUseNode(currentTokenThenMove());
	} else if (currentTokenKind() == LPAREN) {	
		confirmLiteral("(");
		mustGetNextToken();
		ParserResult it = matchEInParens();
		trace << "After matchEInParens, back in matchE, current token is: " << currentToken() << endl;
		// if that left off AFTER the end of the E_IN_PARENS, we still need a ")" in the E we're matching
		confirmLiteral(")");
		getNextToken();	 // we're AFTER the ) now
		return it;
	} else {				
		std::cerr << "Illegal token (" << currentToken() << ") at token #" << tokenNumber() << endl;
		exit(3);
	}
}

// match an "E_IN_PARENS",
//  assuming that the currentToken is at the start of the E_IN_PARENS, e.g. a "+"
//  leave the currentToken AFTER the last part of what was matched,
//  i.e. *on* the ")" that should come after the E_IN_PARENS
static ParserResult matchEInParens() {
	trace << "Entering matchEInParens, current token is " << currentToken() << endl;
	if (find(currentTokenKind(), FIRST_OP)) {
        bool its_a_comparison_op = (currentTokenKind() == OP_COMPARE);
        string theOp = matchOp();
        ParserResult firstChild = matchE();
        ParserResult secondChild = matchE();
        if (its_a_comparison_op) { // a comparison
            return new ComparisonNode(theOp, firstChild, secondChild);
        } else {
            return new ArithmeticNode(theOp, ez_list(firstChild, secondChild));
        }
    } else if (currentTokenKind() == LBRACKET) {
        list<ExprNode *> declarations = list<ExprNode *>();
	    while (currentTokenKind() != RPAREN) {
            mustGetNextToken();
            declarations = list(matchEinBrackets(), declarations);
            confirmLiteral("]");
            mustGetNextToken();
        }
	    return new DeclarationsNode(reverse(declarations, list<ExprNode *>()));
    } else if (currentTokenKind() == IDENTIFIER && currentToken() == "if") {
	    mustGetNextToken();
	    ParserResult condition = matchE();
        ParserResult expriftrue = matchE();
        ParserResult expriffalse = matchE();
        return new ConditionalNode(condition, expriftrue, expriffalse);
    } else if (currentTokenKind() == IDENTIFIER && currentToken() == "letstar") {
        mustGetNextToken();
        ParserResult declarations = matchE();
        list<ExprNode *> expressions = list<ExprNode *>();
        while (currentToken() != ")") {
            expressions = list(matchE(), expressions);
        }
	    return new LetNode(declarations, reverse(expressions, list<ExprNode *>()));
    } else if (currentTokenKind() == IDENTIFIER && currentToken() == "exit") {
		return new CallNode(currentTokenThenMove(), list<ParserResult>());
	} else if (currentTokenKind() == IDENTIFIER && currentToken() == "getint") {
		return new CallNode(currentTokenThenMove(), list<ParserResult>());
	} else {
		std::cerr << "Illegal token (" << currentToken() << ") at token #" << tokenNumber() << endl;
		exit(3);
	}
}

static ParserResult matchEinBrackets() {
    if (currentTokenKind() == IDENTIFIER) {
        VarUseNode variable = VarUseNode(currentTokenThenMove());
        if (currentTokenKind() == IDENTIFIER) {
            VarUseNode definedVar = VarUseNode(currentTokenThenMove());
            return new DeclarationNode(variable, definedVar, "definedVar");
        }
        else if (currentTokenKind() == INT_LITERAL) {
            IntLiteralNode literal = IntLiteralNode(stoi(currentTokenThenMove()));
            return new DeclarationNode(variable, literal, "IntLiteralNode");
        }
        else if (currentTokenKind() == BOOL_LITERAL) {
            BoolLiteralNode literal = BoolLiteralNode(currentTokenThenMove());
            return new DeclarationNode(variable, literal, "BoolLiteralNode");
        }
        else if (currentTokenKind() == LPAREN) {
            ParserResult expr = matchE();
            return new DeclarationNode(variable, expr, "ExprNode");
        }
    }
    std::cerr << "Illegal token (" << currentToken() << ") at token #" << tokenNumber() << endl;
    exit(3);
}

// match an operator, assuming that it is the currentToken
//  leave the currentToken AFTER the last part of what was matched, i.e. unchanged
static string matchOp()
{
	trace << "Entering matchOp, current token is " << currentToken() << endl;
	// could do three cases here, but that's so tedious...
	assert (find(currentTokenKind(), FIRST_OP));
	return currentTokenThenMove();
}


ParserResult matchStartSymbolAndEOF()
{
	getNextToken();  // this will be the first one
	if (!tokenAvailable()) {
		cerr << "Illegal end of input" << endl;
		exit(2);
	}

	ParserResult fullExpression = matchE();  // "E" is our start symbol

	// now make sure there isn't anything else!
	getNextToken();
	if (tokenAvailable()) {
		cerr << "Warning: extra input after end: " << currentToken() << endl;
		exit (1);
	}

	return fullExpression;
}

list<ExprNode *> reverse(list<ExprNode *> list, ::list<ExprNode *> newList) {
    if (empty(list)) {
        return newList;
    }
    if (length(list) == 1) {
        return ::list(first(list), newList);
    }
    return reverse(rest(list), ::list(first(list), newList));
}