#include <hc_list.h>
#include <hc_list_helpers.h>
#include "parser.h"  // for n_errors count; this should really be refactored
#include "AST.h"
#include "streams.h"

using std::string;
using std::endl;
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


// C++ Usage notes:
//   When an object is destroyed, either because it is on the stack, e.g. as a variable,
//     or because it is on the free-store heap, the _destructor(s)_ for the object,
//     including those for the superclass(es) and data fields, are called before
//     the memory for the object is released for potential re-use.
//   Note that _pointers_ do not have destructor methods, so while e.g. when a "+" node
//     goes away, the "string o" data field in the ArithmeticNode will automatically run,
//     but nothing happens (by default) for the trees _pointed_to_ by "subexps".
//     (ArithmeticNode's destructor also runs, of course, and so does ExprNode's.)

#if FREE_AST_VIA_DESTRUCTORS

// Memory management strategy:
//   In a _Tree_, we know that any node will have at most one parent.
//   Since the parser returns a pointer to the _root_,
//     and no methods give access to any node's child pointers,
//     we know that, when the root goes away, all the sub-trees can go away too.
//   So, when the main program "delete"s the root,
//     that will trigger the root's destructor; if that delete's any tree children,
//     the whole thing will then be destroyed recursively.

// Note that no destructor is needed for "IntLiteralNode".
//   It has no subtrees, and data fields go away automatically when the node is deleted

// In contrast, a ComparisonNode has subtree pointers "left" and "right".
//   When we delete a ComparisonNode, those _pointers_ go away,
//   but the trees on the free-store heap will last until we delete them.
//   Since, as noted, we know there are no other pointers to them,
//   we delete them when the ComparisonNode goes away:

ComparisonNode::~ComparisonNode()
{
	trace << "Deleting comparison node " << o << endl;
	delete left;
	delete right;
}

void deleteAllSubtrees(list<ExprNode *>subtrees)
{
	if (!empty(subtrees)) {
		delete head(subtrees);
		deleteAllSubtrees(rest(subtrees));
	}
}

ArithmeticNode::~ArithmeticNode()
{
	trace << "Deleting arithmetic node " << o << endl;
	
	deleteAllSubtrees(subexps);
}

CallNode::~CallNode()
{
	trace << "Deleting call node " << n << endl;
	
	deleteAllSubtrees(argList);
}

IntLiteralNode::~IntLiteralNode()
{
	trace << "Deleting integer literal node " << v << endl;
	// nothing needs to be done here,
	//  except for tracing we could have omitted this.
}

#endif
