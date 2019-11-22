/*
 * "Compiler" project for HaverRacket
 *  Translate a subset of Racket into HERA
 *
 * To run this project and run the program tests/01-multiply.hrk, use the command:
 *   Debug/Compiler-C++-withgitandfiles < tests/01-multiply.hrk | tee Compiler.out
 * and then run the HERA program with the command
 *   HERA-C-Run Compiler.out
 */

#include <iostream>
#include <cstdlib>
#include "scannerDemo.h"
#include "parser.h"
#include "AST.h"
#include "ContextInfo.h"
#include "hc_list_helpers.h" // ez_list

using std::cout;
using std::cerr;
using std::endl;
using std::string;

#include "streams.h"
// uncomment one of the following definitions to enable tracing/debugging output
std::ostream &trace  = std::cerr;
// #include <fstream>  /* needed for ofstream below */
// std::ofstream _trace("/dev/null");	// output to "null", which discards all data
// std::ostream &trace = _trace;

// uncomment one of the following definitions to control the prompt
std::ostream &prompt  = std::cerr;
// #include <fstream>  /* needed for ofstream below */
// std::ofstream _prompt("/dev/null");	// output to "null", which discards all data
// std::ostream &prompt = _prompt;


int main(int numberOfCommandLineArguments, char *theCommandLineArguments[])
{
	try {
		bool testScannerInstead = false;
		if (numberOfCommandLineArguments == 2 && theCommandLineArguments[1] == string("testScannerInstead")) {
			trace << "Testing Scanner instead." << endl;
			testScannerInstead = true;
		}

		if (testScannerInstead) {
			cout << "Demonstrating lexical scanner. Enter tokens followed by <EOF>." << endl;
			scannerDemo();
		} else {
			try {
				ParserResult example1 = new ArithmeticNode("*", HaverfordCS::ez_list<ExprNode *>(new IntLiteralNode(6), new IntLiteralNode(7)));
				trace << "confirming codegen basic functionality on test tree, should get 6*7:" << endl;
				string code = generateFullHERA(example1);
				trace << code << endl;

			} catch (const char *message) {
				cerr << "code generation confirmation test threw exception: " << message << endl;
				return 2;
			}

			try {
				prompt << "> "; // like Racket :-)
				ParserResult AST = matchStartSymbolAndEOF();
//				trace << "Completed Parsing, got AST: " << AST.toCode() << endl;
				try {
					trace << "Now generating code: " << endl;
					cout << generateFullHERA(AST) << endl;
#if FREE_AST_VIA_DESTRUCTORS
					delete AST;
#endif
				} catch (const char *message) {
					cerr << "eval threw exception (typically an unhandled case): " << message << endl;
					return 4;
				}
			} catch (const char *message) {
				cerr << "that's odd, parser threw exception: " << message << endl;
				return 3;
			}
		}

		return 0;  // indicate successful translation if no exit with non-0 before this
	} catch (...) {
		cerr << "Yikes! Uncaught exception" << endl;
		return 66;
	}
}
