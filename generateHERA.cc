#include "AST.h"
#include "ContextInfo.h"
#include "hc_list_helpers.h"
#include "streams.h"

using std::string;

string IntLiteralNode::generateHERA(const ContextInfo &context) const
{
	return "SET(" + context.getReg() + ", " + std::to_string(v) + ")\n";
}

string ComparisonNode::generateHERA(const ContextInfo &context) const
{
	throw "comparison node code generation not implemented yet";
}

std::string HERA_op(const std::string &AST_op_name)
{
	if (AST_op_name == "+") {
		return "ADD";
	} else if (AST_op_name == "-") {
		return "SUB";
	} else if (AST_op_name == "*") {
		return "MUL";
	} else {
		throw "Internal compiler inconsistency/incompleteness: HERA_op not implemented for " + AST_op_name;
	}
}

string ArithmeticNode::generateHERA(const ContextInfo &context) const
{
	if (length(subexps) != 2) {
		throw "compiler incomplete/inconsistent: generateHERA not implemented for non-binary arithmetic";
	}
	ContextInfo lhsContext = context.evalThisFirst();
	ContextInfo rhsContext = context;  // just named for symmetry
	
	return (first(subexps)->generateHERA(lhsContext) +
		first(rest(subexps))->generateHERA(rhsContext) +
		HERA_op(o)+"("+context.getReg()+", "+lhsContext.getReg()+","+rhsContext.getReg()+")\n");
}

string VarUseNode::generateHERA(const ContextInfo &context) const
{
	throw "variableUse code generation not implemented yet";
	return "compiler ought not to require a return here, this can't be run";
}

string CallNode::generateHERA(const ContextInfo &context) const
{
	if (length(argList) != 0 || (n != "exit" && n != "getint")) {
		throw "compiler incomplete/inconsistent: generateHERA for calls only implented for getint and exit";
	}
	// NOTE that calls to exit and getint don't need parameters and don't perturb registers
	return ("MOVE(FP_alt, SP)\nCALL(FP_alt," + n + ")\n"+
		(context.getReg()=="R1"?"":"MOVE("+context.getReg()+", R1)"));
}

/*
int eval(Tree expression)
{
	trace << "Entering eval with tree whose root is " << expression.value() << std::endl;
	
	if (expression.isLeaf()) {  // adjust for your Tree class
		if (expression.value() == "exit") {
			trace << "Exiting" << std::endl;
			exit(0);
		}
		return stoi(expression.value()); // http://www.cplusplus.com/reference/string/stoi/
	} else if (expression.value() == "+") {
		return foldr<int, int>(std::plus<int>(),
				       0,
				       map<Tree, int>(eval, expression.children()));
	} else if (expression.value() == "*") {
		return foldr<int,int>(std::multiplies<int>(),  // well, I didn't choose the name
				      1,
				      map<Tree, int>(eval, expression.children()));
	} else {
		throw "Unrecognized operator: " + expression.value();
	}
}

*/
