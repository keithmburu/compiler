enum kindOfToken {
	END_OF_INPUT = 0,   // defined by flex system
	LPAREN,	RPAREN,
	PLUS,	MINUS,	TIMES,
	INT, IDENTIFIER
};

void scannerError(); /// define in whatever uses the regexp-based scanner

extern int tokenCount;  // not static because the .l file needs it too
