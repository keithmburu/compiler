#include <hc_list.h>
#include <hc_list_helpers.h>
#include "parser.h"  // for n_errors count; this should really be refactored
#include "streams.h"

using std::string;
using std::endl;
using HaverfordCS::list;


#include <fstream>  /* needed for ofstream below */
#if defined TRACE_EXPR_ALLOCATIONS
static std::ofstream  alloc_trace(TRACE_EXPR_ALLOCATIONS);
#else
// by default, allow option for control via environment variable, but it's not given, send to /dev/null (disappears)
static std::ofstream alloc_trace(getenv("HAVERRACKET_ALLOC_TRACE")?getenv("HAVERRACKET_ALLOC_TRACE"):"/dev/null");
// static std::ofstream &alloc_trace = trace;  // alternate easy option, just send to regular trace
#endif



// This file has the constructors and destructors;
//   all the generateHERA methods are together in generateHERA.cc


// C++ Usage Note:
// The v(value) is like having v=value, but initializes v rather than assigning to it
// i.e., writing "v = value" in the constructor _body_ is like writing
// i.e., writing "v = value" in the constructor _body_ is like writing
//     int i;
//     i = 12;
// whereas writing v(value) before the body is like writing
//     int i=12;  // i is created with "12" from the start
//
// Those steps, as the call(s) to any superclass(es)' constructors, are done before the body of the IntLiteralNode itself
IntLiteralNode::IntLiteralNode(int value) : v(value)
{
	// nothing else needs to be done here, since the stuff above defines "v" as "value"
	alloc_trace << "(class IntLiteralNode constructor called for node at memory " << this << " and value=" << value << endl;
}

BoolLiteralNode::BoolLiteralNode(string value) : v((value == "#t")? 1 : 0)
{
    // nothing else needs to be done here, since the stuff above defines "v" as "value"
    alloc_trace << "(class BoolLiteralNode constructor called for node at memory " << this << " and value=" << value << endl;
}

// (so, we should see this trace before the one above, for each int literal node)
ExprNode::ExprNode()
{
	alloc_trace << "[superclass ExprNode constructor  called for node at memory " << this << endl;
}

ComparisonNode::ComparisonNode(string op, ExprNode *lhs, ExprNode *rhs) :
	o(op),
	left(lhs),
	right(rhs)
{
	alloc_trace << "(class ComparisonNode constructor called for node at memory " << this << endl;
}

ArithmeticNode::ArithmeticNode(string op, list<ExprNode *>operands) :
	o(op),
	subexps(operands)
{
	alloc_trace << "(class ArithmeticNode constructor called for node at memory " << this << endl;
}


VarUseNode::VarUseNode(string name) : n(name)
{
	alloc_trace << "(class     VarUseNode constructor called for node at memory " << this << endl;
}

CallNode::CallNode(string funcName, HaverfordCS::list<ExprNode *>arguments) :
	n(funcName),
	argList(arguments)
{
	alloc_trace << "(class       CallNode constructor called for node at memory " << this << endl;
}

IfNode::IfNode(ExprNode *condition, ExprNode *expriftrue, ExprNode *expriffalse) :
    condition(condition),
    expriftrue(expriftrue),
    expriffalse(expriffalse)
{
    alloc_trace << "(class IfNode constructor called for node at memory " << this << endl;
}

LetNode::LetNode(ExprNode *declarations, HaverfordCS::list<ExprNode *> expressions) :
        declarations(declarations),
        expressions(expressions)
{
    alloc_trace << "(class LetNode constructor called for node at memory " << this << endl;
}

DeclarationsNode::DeclarationsNode(HaverfordCS::list<ExprNode *> declarations) :
        declarations(declarations)
{
    alloc_trace << "(class DeclarationsNode constructor called for node at memory " << this << endl;
}

DeclarationNode::DeclarationNode(VarUseNode variable, IntLiteralNode intLiteral, string declType) :
        variable(variable),
        intLiteral(intLiteral),
        declType(declType)
{
    alloc_trace << "(class DeclarationNode constructor called for node at memory " << this << endl;
}

DeclarationNode::DeclarationNode(VarUseNode variable, BoolLiteralNode boolLiteral, string declType) :
        variable(variable),
        boolLiteral(boolLiteral),
        declType(declType)
{
    alloc_trace << "(class DeclarationNode constructor called for node at memory " << this << endl;
}

DeclarationNode::DeclarationNode(VarUseNode variable, VarUseNode definedVar, string declType) :
        variable(variable),
        definedVar(definedVar),
        declType(declType)
{
    alloc_trace << "(class DeclarationNode constructor called for node at memory " << this << endl;
}

DeclarationNode::DeclarationNode(VarUseNode variable, ExprNode *expr, string declType) :
        variable(variable),
        expr(expr),
        declType(declType)
{
    alloc_trace << "(class DeclarationNode constructor called for node at memory " << this << endl;
}

// C++ Usage notes:
//   When an object is destroyed, either because it is on the stack, e.g. as a variable,
//     or because it is on the free-store heap, the _destructor(s)_ for the object,
//     including those for the superclass(es) and data fields, are called before
//     the memory for the object is released for potential re-use
//     (destructors are called for those fields as they are released,
//     and the superclass destructor is used after the class' own destructor is done).
//
//   Note that _pointers_ do not have destructor methods, so while e.g. when a "+" node
//     goes away, the "string o" data field in the ArithmeticNode will automatically run,
//     but nothing happens (by default) for the trees _pointed_to_ by "subexps".
//     (ArithmeticNode's destructor also runs, of course, and so _it_ may take care of subexps)

ExprNode::~ExprNode()  // nothing to do, but this ensures all subclasses will have virtual destructors, which C++ likes
{
	alloc_trace << " superclass ExprNode destructor   called for node at memory " << this << "]" << endl;
}

IntLiteralNode::~IntLiteralNode()
{
	alloc_trace << " class IntLiteralNode  destructor called for node at memory " << this << " that had value=" << v << ")" << endl;
	// nothing needs to be done here,
	//  except for tracing we could have omitted this.
}

BoolLiteralNode::~BoolLiteralNode()
{
    alloc_trace << " class BoolLiteralNode  destructor called for node at memory " << this << " that had value=" << v << ")" << endl;
    // nothing needs to be done here,
    //  except for tracing we could have omitted this.
}

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
	alloc_trace << " class ComparisonNode  destructor called for node at memory " << this << ")" << endl;

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
	alloc_trace << " class ArithmeticNode  destructor called for node at memory " << this << ")" << endl;
	
	deleteAllSubtrees(subexps);
}

VarUseNode::~VarUseNode()
{
	alloc_trace << " class     VarUseNode  destructor called for node at memory " << this << ")" << endl;
}

CallNode::~CallNode()
{
	alloc_trace << " class       CallNode  destructor called for node at memory " << this << ")" << endl;
	
	deleteAllSubtrees(argList);
}

IfNode::~IfNode()
{
    alloc_trace << " class     IfNode  destructor called for node at memory " << this << ")" << endl;
}

LetNode::~LetNode()
{
    alloc_trace << " class     LetNode  destructor called for node at memory " << this << ")" << endl;
}

DeclarationsNode::~DeclarationsNode()
{
    alloc_trace << " class     DeclarationsNode  destructor called for node at memory " << this << ")" << endl;
}

DeclarationNode::~DeclarationNode()
{
    alloc_trace << " class     DeclarationsNode  destructor called for node at memory " << this << ")" << endl;
}

#endif
