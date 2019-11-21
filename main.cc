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
				string code = example1->generateHERA(ContextInfo());
				trace << code << endl;

			} catch (std::string message) {
				cerr << "code generation confirmation test threw exception: " << message << endl;
				return 2;
			}

			try {
				prompt << "> "; // like Racket :-)
				ParserResult AST = matchStartSymbolAndEOF();
//				trace << "Completed Parsing, got AST: " << AST.toCode() << endl;
				try {
					trace << "Now generating code: " << endl;
					cout << AST->generateHERA(ContextInfo()) << endl;
				} catch (std::string message) {
					cerr << "eval threw exception (typically an unhandled case): " << message << endl;
					return 4;
				}
			} catch (std::string message) {
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

