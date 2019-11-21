#include <hc_list.h>
#include <hc_list_helpers.h>
#include "parser.h"  // for n_errors count; this should really be refactored
#include "AST.h"

using std::string;
using HaverfordCS::list;

// This file has the constructors and destructors;
//   all the generateHERA methods are together in generateHERA.cc

ExprNode::~ExprNode()  // nothing to do, but this ensures all subclasses will have virtual destructors, which C++ likes
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
IntLiteralNode::IntLiteralNode(int value) : v(value)
{
}



ComparisonNode::ComparisonNode(string op, ExprNode *lhs, ExprNode *rhs) :
	o(op),
	left(lhs),
	right(rhs)
{
}

ArithmeticNode::ArithmeticNode(string op, list<ExprNode *>operands) :
	o(op),
	subexps(operands)
{
}


VarUseNode::VarUseNode(string name) : n(name)
{
}


CallNode::CallNode(string funcName, HaverfordCS::list<ExprNode *>arguments) :
	n(funcName),
	argList(arguments)
{
}
