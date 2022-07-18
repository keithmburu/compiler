#ifndef AST_H_
#define AST_H_

#include <string>
#include <hc_list.h>
#include "ContextInfo.h"
#include "Dictionary.h"


/*
 *  This file defines the heirarchy of different kinds of AST nodes.
 *  Currently we have:
 *    ExprNode (an "interface" class for expressions, with the following concrete subclasses ("implementers" of the interface):
 *	IntLiteralNode(int value)
 *	ComparisonNode(std::string op, ExprNode *lhs, ExprNode *rhs)
 *	ArithmeticNode(std::string op, list<ExprNode *>)
 *	VarUseNode(std::string name)
 *	CallNode(std::string name, HaverfordCS::list<ExprNode *>arguments)
 *
 *  The "generateHERA" methods are usually called by calling generateFullHERA on the root,
 *      which puts "CBON" at the start.
 */

#if ! defined FREE_AST_VIA_DESTRUCTORS
#define FREE_AST_VIA_DESTRUCTORS true
#endif

// C++ Usage Note:
// The empty class definition of ContextInfo below lets us declare "const ContextInfo &" parameters
//   without having to define the whole class here, which is fine because, at this point,
//   we don't care much about what's in it ... we just need to know that there's some way
//   to pass information down to the subtrees as we evaluate them.


// Define the information will we need to pass down the tree as we generate code, see ContextInfo.h
class ContextInfo;

extern Dictionary declarationDict;

// C++ Usage Note:
//
// The word "virtual" is needed in C++ when inheritance is use,
//   to explicitly tell the compiler that we might override a function in a subclass
// Java has the correct default of allowing this, and the keyword "final" prevents it.
//   Many C++ compilers fail to complain if you forget the word "virtual" and override a method anyway :-(
//
// Putting "= 0" at the end of a virtual function declaration means
//   that the method _must_ be overridden in subclasses
//
// Classes with virtual functions and (in subclasses) data with destructors need to have
//  virtual destructors, to help the C++ compiler be sure the right data destructors are called


// Class ExprNode is an interface that defines the methods that all
//   expression nodes in our AST will provide
// C++ does not have Java's "interface" keyword, so instead
//   we just create a superclass with no data and all functions virtual

class ExprNode {
public:
	ExprNode();           // just used for memory allocation labs, to print trace information
	virtual ~ExprNode();  // used for actual memory allocation, sometimes, and for tracing

    virtual std::string generateHERA(const ContextInfo &info) const = 0;
    virtual std::string getType() const = 0;

    ExprNode* operator==(ExprNode *other) {
        this->type = other->type;
        return this;
    }
private:
    std::string type;
};

std::string generateFullHERA(ExprNode *presumedRoot);



// Now, the various specific kinds of expressions we might have:

class IntLiteralNode : public ExprNode {
	public:
		IntLiteralNode(int value);
		~IntLiteralNode();  // just used for trace output

        int getValue() const { return v; }
        std::string getType() const { return type; }

		std::string generateHERA(const ContextInfo &info) const;
	private:
		int v;  // the value
        const std::string type = "IntLiteralNode";
};

class BoolLiteralNode : public ExprNode {
public:
    BoolLiteralNode(std::string value);
    ~BoolLiteralNode();  // just used for trace output

    int getValue() const { return v; }
    std::string getType() const { return type; }

    std::string generateHERA(const ContextInfo &info) const;
private:
    int v;  // the value
    const std::string type = "BoolLiteralNode";
};

class ComparisonNode : public ExprNode {  // <= etc., _inherently_binary_ in HaverRacket
public:
    ComparisonNode(std::string op, ExprNode *lhs, ExprNode *rhs);
#if FREE_AST_VIA_DESTRUCTORS
    ~ComparisonNode();
#endif
    std::string getType() const { return type; }

    std::string generateHERA(const ContextInfo &info) const;
private:
    std::string o;
    ExprNode *left;
    ExprNode *right;
    const std::string type = "ComparisonNode";
};

class ArithmeticNode : public ExprNode {  // +, *, -, etc.
	public:
		ArithmeticNode(std::string op, HaverfordCS::list<ExprNode *>);
#if FREE_AST_VIA_DESTRUCTORS
		~ArithmeticNode();
#endif
        std::string getType() const { return type; }

        std::string generateHERA(const ContextInfo &info) const;
	private:
		std::string o;
		HaverfordCS::list<ExprNode *>subexps;
        const std::string type = "ArithmeticNode";
};

/*
   This represents the use of an identifier as an expression, e.g. the x in (* x 6).
   Note that identifiers also exist in variable declarations, e.g. (let ((x 7)) ... )
     but the x there doesn't count as an _expression_ since we're not asking for its value.
 */
class VarUseNode : public ExprNode {
	public:
		VarUseNode(std::string name);
		~VarUseNode();

        std::string getValue() const { return n; }
        std::string getType() const { return type; }

        std::string generateHERA(const ContextInfo &info) const;
	private:
		std::string n;
        const std::string type = "VarUseNode";
};


class CallNode : public ExprNode {
	public:
		CallNode(std::string funcName, HaverfordCS::list<ExprNode *>arguments);
#if FREE_AST_VIA_DESTRUCTORS
		~CallNode();
#endif
        std::string getType() const { return type; }

        std::string generateHERA(const ContextInfo &info) const;
	private:
		std::string n;  // the name
		HaverfordCS::list<ExprNode *>argList;
        const std::string type = "CallNode";
};

class IfNode : public ExprNode {
public:
    IfNode(ExprNode *condition, ExprNode *expriftrue, ExprNode *expriffalse);
#if FREE_AST_VIA_DESTRUCTORS
    ~IfNode();
#endif
    std::string getType() const { return type; }

    std::string generateHERA(const ContextInfo &info) const;
private:
    ExprNode *condition;
    ExprNode *expriftrue;
    ExprNode *expriffalse;
    const std::string type = "IfNode";
};

class DeclarationsNode : public ExprNode {
public:
    DeclarationsNode(HaverfordCS::list<ExprNode *> declarations);
    HaverfordCS::list<ExprNode *> getDeclarations();
    std::string declarationsHelper(std::string declarationsHERA, HaverfordCS::list<ExprNode *> declarations, ContextInfo context) const;
#if FREE_AST_VIA_DESTRUCTORS
    ~DeclarationsNode();
#endif
    std::string getType() const { return type; }

    std::string generateHERA(const ContextInfo &info) const;
private:
    HaverfordCS::list<ExprNode *> declarations;
    const std::string type = "DeclarationsNode";
};

class DeclarationNode : public ExprNode {
public:
    DeclarationNode(VarUseNode variable, IntLiteralNode intLiteral, string declType);
    DeclarationNode(VarUseNode variable, BoolLiteralNode boolLiteral, string declType);
    DeclarationNode(VarUseNode variable, VarUseNode definedVar, string declType);
    DeclarationNode(VarUseNode variable, ExprNode *expr, string declType);
#if FREE_AST_VIA_DESTRUCTORS
    ~DeclarationNode();
#endif
    std::string getType() const { return type; }

    std::string generateHERA(const ContextInfo &info) const;
private:
    VarUseNode variable;
    VarUseNode definedVar = VarUseNode("defaultVar");
    std::string declType;
    IntLiteralNode intLiteral = IntLiteralNode(0);
    BoolLiteralNode boolLiteral = BoolLiteralNode("#f");
    ExprNode *expr = new IntLiteralNode(0);
    const std::string type = "DeclarationNode";
};

class LetNode : public ExprNode {
public:
    LetNode(ExprNode *declarations, HaverfordCS::list<ExprNode *> expressions);
#if FREE_AST_VIA_DESTRUCTORS
    ~LetNode();
#endif
    std::string getType() const { return type; }

    std::string generateHERA(const ContextInfo &info) const;
    std::string expressionsHelper(std::string expressionsHERA, HaverfordCS::list<ExprNode *> expressions, ContextInfo context) const;
    private:
    ExprNode *declarations;
    HaverfordCS::list<ExprNode *> expressions;
    const std::string type = "LetNode";
};

#endif /*AST_H_*/
