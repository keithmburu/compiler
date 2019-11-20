#ifndef AST_H_
#define AST_H_

#include <string>
#include <hc_list.h>


class contextInfo; // what information will we need to pass down the tree as we generate code?


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


// Class Expr_Node is an interface that defines the methods that all
//   expression nodes in our AST will provide
// C++ does not have Java's "interface" keyword, so instead
//   we just create a superclass with no data and all functions virtual

class Expr_Node {
public:
	virtual ~Expr_Node();

	virtual std::string generateHERA(const contextInfo &info) = 0;
	// virtual typeInfo type() = 0;  // type info not required in 2019
};

// Now, the various specific kinds of expressions we might have:

class Int_Literal_Node : public Expr_Node {
	public:
		Int_Literal_Node(int value);

		std::string generateHERA(const contextInfo &info);
	private:
		int v;  // the value
};


class Comparison_Node : public Expr_Node {  // <= etc., _inherently_binary_ in HaverRacket
	public:
		Op_Node(std::string op, Expr_Node *lhs, Expr_Node *rhs);
		~Op_Node();
		std::string generateHERA(const contextInfo &info);
	private:
		std::string o;
		Expr_Node *left;
		Expr_Node *right;
};

class Arithmetic_Node : public Expr_Node {  // +, *, -, etc.
	public:
		Op_Node(std::string op, HaverfordCS::list<Expr_Node *>);
		~Op_Node();
		std::string generateHERA(const contextInfo &info);
	private:
		std::string o;
		HaverfordCS::list<Expr_Node *>subexps;
};


/*
   This represents the use of an identifier as an expression, e.g. the x in (* x 6).
   Note that identifiers also exist in variable declarations, e.g. (let ((x 7)) ... )
     but the x there doesn't count as an _expression_ since we're not asking for its value.
 */

class Var_Use_Node : public Expr_Node {
	public:
		Var_Use_Node(std::string name);
		std::string   generateHERA(const contextInfo &info);
	private:
		std::string n;
};

#endif /*AST_H_*/
