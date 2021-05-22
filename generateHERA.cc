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
    return "\nCBON()\n" + presumedRoot->generateHERA(ContextInfo());
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

    if (left->getType() != "CallNode" && right->getType() != "CallNode") {
        if ((left->getType() != "IntLiteralNode" && left->getType() != "VarUseNode" && left->getType() != "LetNode") ||
                (right->getType() != "IntLiteralNode" && right->getType() != "VarUseNode" && right->getType() != "LetNode")) {
            cerr << endl << "!Type error! cannot perform comparison operations on non-integers" << endl;
            exit(98);
        }
    }

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

    if (first(subexps)->getType() != "CallNode" && first(rest(subexps))->getType() != "CallNode") {
        if ((first(subexps)->getType() != "IntLiteralNode" && first(subexps)->getType() != "VarUseNode" && first(subexps)->getType() != "LetNode") ||
                (first(rest(subexps))->getType() != "IntLiteralNode" && first(rest(subexps))->getType() != "VarUseNode" && first(rest(subexps))
                ->getType() != "LetNode")) {
            cerr << endl << "!Type error! cannot perform arithmetic operations on non-integers" << endl;
            exit(99);
        }
    }

	ContextInfo rhsContext = context.evalThisAfter();
	ContextInfo lhsContext = context;  // just named for symmetry

    string leftHERA = first(subexps)->generateHERA(lhsContext);
    string rightHERA = first(rest(subexps))->generateHERA(rhsContext);

    return  leftHERA + rightHERA + HERA_op(o)+"("+context.getReg()+", "+lhsContext.getReg()+", "+rhsContext.getReg()+")\n";
}

string VarUseNode::generateHERA(const ContextInfo &context) const
{
	trace << "Entered VarUseNode::generateHERA for variable " + n << endl;

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

string IfNode::generateHERA(const ContextInfo &context) const
{
    trace << "Entered IfNode::generateHERA" << endl;

    if (expriftrue->getType() != "CallNode" && expriffalse->getType() != "CallNode") {
        if (expriftrue->getType() != expriffalse->getType()) {
            cerr << endl << "!Type error! \"then\" and \"else\" statements must be of the same type" << endl;
            exit(45);
        }
    }

    ContextInfo conditionContext = context;
    ContextInfo expriftrueContext = context.evalThisAfter();
    ContextInfo expriffalseContext = context.evalThisAfter().evalThisAfter();

    ContextInfo labelContext1 = ContextInfo();
    ContextInfo labelContext2 = ContextInfo();

    string conditionHERA = condition->generateHERA(conditionContext);
    string expriftrueHERA = expriftrue->generateHERA(expriftrueContext);
    string expriffalseHERA = expriffalse->generateHERA(expriffalseContext);

    return  conditionHERA + expriftrueHERA + expriffalseHERA +
            "FLAGS(" + conditionContext.getReg() + ")\n" +
            "BZ(" + labelContext1.getLabel() + ")\n" +
            "MOVE(" + context.getReg() + ", " + expriftrueContext.getReg() + ")\n" +
            "BR(" + labelContext2.getLabel() + ")\n" +
            "LABEL(" + labelContext1.getLabel() + ")\n" +
            "MOVE(" + context.getReg() + ", " + expriffalseContext.getReg() + ")\n" +
            "LABEL(" + labelContext2.getLabel() + ")\n";
}

string LetNode::generateHERA(const ContextInfo &context) const
{
    trace << "Entered LetNode::generateHERA" << endl;
    ContextInfo declarationsContext = context;
    ContextInfo expressionsContext = context;

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
    return  declarationsHelper(declarationsHERA, declarations, context);
}

string DeclarationsNode::declarationsHelper(string declarationsHERA, HaverfordCS::list<ExprNode *> declarations, ContextInfo context) const {
    if (empty(declarations)) {
        return declarationsHERA;
    }
    return declarationsHelper(declarationsHERA += first(declarations)->generateHERA(context), rest(declarations),
                              context);
}

string DeclarationNode::generateHERA(const ContextInfo &context) const
{
    auto literal = expr;
    string value;
    if (declType == "IntLiteralNode") {
        *literal = intLiteral;
        value = to_string(intLiteral.getValue());
    }
    else if (declType == "BoolLiteralNode") {
        *literal = boolLiteral;
        value = boolLiteral.getValue();
    }
    else if (declType == "definedVar") {
        *literal = definedVar;
        value = definedVar.getValue();
    }

    trace << "Entered DeclarationNode::generateHERA for declaration " + variable.getValue() + ((declType == "ExprNode")? " = expression" : " = " +
    value) << endl;

    if (declType == "definedVar") {
        value = to_string(declarationDict.lookup(value));
    }

    FPoffset += 1;

    declarationDict.add(variable.getValue(), FPoffset);

    return ((declType == "ExprNode")? literal->generateHERA(context) :
           (declType == "definedVar")? "LOAD(" + context.getReg() + ", " + value + ", FP)\n" :
           "SET(" + context.getReg() + ", " + value + ")\n") +
           "STORE(" + context.getReg() + ", " + std::to_string(FPoffset) + ", FP)\n";
}




