#include <hc_list.h>
#include <hc_list_helpers.h>
#include "parser.h"  // for n_errors count; this should really be refactored
#include "AST.h"

using std::string;
using HaverfordCS::list;

// This file has the constructors and destructors;
//   all the generateHERA methods are together in generateHERA.cc

Expr_Node::~Expr_Node()  // nothing to do, but this ensures all subclasses will have virtual destructors, which C++ likes
{
}

// C++ Usage Note:
// the v(value) is like having v=value, but initializes v rather than assigning to it
// i.e., writing "v = value" in the constructor _body_ is like writing
//     int i;
//     i = 12;
// whereas writing v(value) before the body is like writing
//     int i=12;  // i is created with "12" from the start
//
Int_Literal_Node::Int_Literal_Node(int value) : v(value)
{
}



Comparison_Node::Comparison_Node(string op, Expr_Node *lhs, Expr_Node *rhs) :
	o(op),
	left(lhs),
	right(rhs)
{
}

Arithmetic_Node::Arithmetic_Node(string op, list<Expr_Node *>operands) :
	o(op),
	subexps(operands)
{
}


Var_Use_Node::Var_Use_Node(string name) : n(name)
{
}
