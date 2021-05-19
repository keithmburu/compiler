#include "AST.h"
#include "ContextInfo.h"
#include "hc_list_helpers.h"
#include "streams.h"

using std::string;
using std::endl;

Dictionary declarationDict = Dictionary();
int FPoffset = -1;

std::string generateFullHERA(ExprNode *presumedRoot)
{
    return "CBON()\n" + presumedRoot->generateHERA(ContextInfo());
}

string IntLiteralNode::generateHERA(const ContextInfo &context) const
{
	trace << "Entered IntLiteralNode::generateHERA for integer " + std::to_string(v) << endl;
	return "SET(" + context.getReg() + ", " + std::to_string(v) + ")\n";
}

string BoolLiteralNode::generateHERA(const ContextInfo &context) const
{
    trace << "Entered BoolLiteralNode::generateHERA for boolean " + std::to_string(v) << endl;
    return "SET(" + context.getReg() + ", " + std::to_string(v) + ")\n";
}

std::string HERA_op(const std::string &AST_op_name)
{
	if (AST_op_name == "+") {
		return "ADD";
	} else if (AST_op_name == "-") {
		return "SUB";
	} else if (AST_op_name == "*") {
		return "MUL";
    } else if (AST_op_name == "=") {
        return "CMP";
    } else if (AST_op_name == ">=") {
        return "CMP";
    } else if (AST_op_name == "<=") {
        return "CMP";
	} else {
		throw "Internal compiler inconsistency/incompleteness: HERA_op not implemented for " + AST_op_name;
	}
}

string ComparisonNode::generateHERA(const ContextInfo &context) const
{
    trace << "Entered ComparisonNode::generateHERA for comparison " + o << endl;

    // see arithmetic node for more about the "context" stuff:
    //	trace << "need to compare the result of left-hand-side:\n" << left->generateHERA(context) << endl;
    //	trace << "                        with right-hand-side:\n" << left->generateHERA(context.evalThisAfter()) << endl;

    ContextInfo rhsContext = context.evalThisAfter();
    ContextInfo lhsContext = context;  // just named for symmetry

    ContextInfo labelContext1 = ContextInfo();
    ContextInfo endLabelContext = ContextInfo();


    return left->generateHERA(lhsContext) +
           right->generateHERA(rhsContext) +
           HERA_op(o)+"("+lhsContext.getReg()+", "+rhsContext.getReg()+")\n" +
           "BZ(" + context.getLabel() + ")\n" +
            ((o == "=")? "SET(" + context.getReg() + ", " + "0)\n" :
             (o == "<=" || o == ">=")? "BS(" + labelContext1.getLabel() + ")\n" : "") +
            ((o == ">=")? "SET(" + context.getReg() + ", " + "1)\n" :
             (o == "<=")? "SET(" + context.getReg() + ", " + "0)\n" : "") +
            "BR(" + endLabelContext.getLabel() + ")\n" +
            "LABEL(" + context.getLabel() + ")\n" +
            "SET(" + context.getReg() + ", " + "1)\n" +
            "BR(" + endLabelContext.getLabel() + ")\n" +
            ((o == "<=" || o == ">=")? ("LABEL(" + labelContext1.getLabel() + ")\n" +
            ((o == "<=")? "SET(" + context.getReg() + ", " + "1)\n" :
             "SET(" + context.getReg() + ", " + "0)\n")) : "") +
             "LABEL(" + endLabelContext.getLabel() + ")\n";
}

string ArithmeticNode::generateHERA(const ContextInfo &context) const
{
	trace << "Entered ArithmeticNode::generateHERA for operator " + o << endl;
	if (length(subexps) != 2) {
		throw "compiler incomplete/inconsistent: generateHERA not implemented for non-binary arithmetic";
	}
	ContextInfo rhsContext = context.evalThisAfter();
	ContextInfo lhsContext = context;  // just named for symmetry

    return (first(subexps)->generateHERA(lhsContext) +
		first(rest(subexps))->generateHERA(rhsContext) +
		HERA_op(o)+"("+context.getReg()+", "+lhsContext.getReg()+", "+rhsContext.getReg()+")\n");
}

string VarUseNode::generateHERA(const ContextInfo &context) const
{
	trace << "Entered VarUseNode::generateHERA for variable " + n << endl;
    cout << (empty(declarationDict.getList())? "dict is still empty" : "YAY dict has something") << endl;

    return "LOAD(" + context.getReg() + ", " + to_string(declarationDict.lookup(n)) + ", FP)\n";
}

string CallNode::generateHERA(const ContextInfo &context) const
{
	trace << "Entered CallNode::generateHERA for call to " + n << endl;
	
	if (length(argList) != 0 || (n != "exit" && n != "getint")) {
		throw "compiler incomplete/inconsistent: generateHERA for calls only implented for getint and exit";
	}
	// NOTE that calls to exit and getint don't need parameters and don't perturb registers
	return ("MOVE(FP_alt, SP)\nCALL(FP_alt," + n + ")\n"+
		(context.getReg()=="R1"?"":"MOVE("+context.getReg()+", R1)\n"));
}

string ConditionalNode::generateHERA(const ContextInfo &context) const
{
    trace << "Entered ConditionalNode::generateHERA" << endl;
    ContextInfo conditionContext = context;
    ContextInfo expriftrueContext = context.evalThisAfter();
    ContextInfo expriffalseContext = context.evalThisAfter().evalThisAfter();

    ContextInfo labelContext = ContextInfo();

    return  condition->generateHERA(conditionContext) + expriftrue->generateHERA(expriftrueContext) +
            expriffalse->generateHERA(expriffalseContext) +
            "FLAGS(" + conditionContext.getReg() + ")\n" +
            "BZ(" + context.getLabel() + ")\n" +
            "MOVE(" + context.getReg() + ", " + expriftrueContext.getReg() + ")\n" +
            "BR(" + labelContext.getLabel() + ")\n" +
            "LABEL(" + context.getLabel() + ")\n" +
            "MOVE(" + context.getReg() + ", " + expriffalseContext.getReg() + ")\n" +
            "LABEL(" + labelContext.getLabel() + ")\n";
}

string LetNode::generateHERA(const ContextInfo &context) const
{
    trace << "Entered LetNode::generateHERA" << endl;
    ContextInfo declarationsContext = context;
    ContextInfo expressionsContext = context.evalThisAfter();

    string expressionsHERA = "";
    string declarationsHERA = declarations->generateHERA(declarationsContext);
    expressionsHERA = expressionsHelper(expressionsHERA, expressions, expressionsContext);
    return declarationsHERA + expressionsHERA;
}

string LetNode::expressionsHelper(string expressionsHERA, HaverfordCS::list<ExprNode *> expressions, ContextInfo context) const {
    if (empty(expressions)) {
        return expressionsHERA;
    }
    return expressionsHelper(expressionsHERA += first(expressions)->generateHERA(context), rest(expressions), context.evalThisAfter());
}

string DeclarationsNode::generateHERA(const ContextInfo &context) const
{
    trace << "Entered DeclarationsNode::generateHERA" << endl;
    ContextInfo declarationsContext = context;

    ContextInfo labelContext = ContextInfo();

    string declarationsHERA = "";
    return  declarationsHelper(declarationsHERA, declarations, context, FPoffset);
}

string DeclarationsNode::declarationsHelper(string declarationsHERA, HaverfordCS::list<ExprNode *> declarations, ContextInfo context,
                                            int FPoffset) const {
    if (empty(declarations)) {
        return declarationsHERA;
    }
    return declarationsHelper(declarationsHERA += first(declarations)->generateHERA(context), rest(declarations),
                              context, FPoffset);
}

string DeclarationNode::generateHERA(const ContextInfo &context) const
{
    trace << "Entered DeclarationNode::generateHERA for declaration " + variable.getValue() + " = " + to_string(literal.getValue()) << endl;

    FPoffset += 1;
    declarationDict.add(variable.getValue(), FPoffset);

    return "SET(" + context.getReg() + ", " + std::to_string(literal.getValue()) + ")\n" +
           "STORE(" + context.getReg() + ", " + std::to_string(FPoffset) + ", FP)\n";
}

