NodeType{
	ScriptNode,				//list of statements and a variable context
	NoteNode, 				//two expressions
	PatternNode,			//list of expressions representing sub-beats
	LoopNode,				//list of nodes and iteration count
	LetNode,				//variable ID and expression for its value

	//expressions			
	AdditiveExprNode,		//left/right and (+ or - ) operator
	MultiplicativeExprNode,	//left/right and (* or / ) operator
	PrimaryExprNode,		//basic values/variables
	RandomNode				//random range (min, max)
}


//return a series of scriptnodes. the evaluator can run through the list linearly(and recursively sometimes) to generate songs. If there is a parsing error, it will return an empty list and populate the error message with the problem. 
scriptnode* parse _script(Token list tokens , string& errorMsg) 
{
	scriptnode list script= {}
	map variables<id, value> = {}

	while (lookahead isn't empty)
		bool successful = parse_statement(tokens, script, variables, errorMsg)
		if (not successful) return {}
	end

	return script
}

//every innner parse function arguments sim. 
		
bool parse_statement(tokens, script, variables, errorMsg)
{
	//the "successful" variable stores whether or not parsing the current statement succeeded. if it didn't, issue an error.

	bool successful = false;
	if (lookahead.type = INT || ID || RAND || PARSTART) successful = parse_note()
	else if (lookahead = PATTERN) successful = parse_pattern()
	else if (lookahead = LET) successful = parse_let()
	else if (lookahead = BARSTART) successful = parse_loop()
	else {
		*errorMsg = "Error: invalid syntax"
		successful = false
	}

	if(successful) {
		//I don't want to enforce requiring a comma as the very last character of the song, so:
		//check lookahead_many(1). If it exists, then we should have a comma as our direct lookahead.
		
		if(lookahead_many(1) is not empty && !match_token(script, COMMA)){
			*errorMsg = "Error: missing comma between statements"
			return false
		} else {
			return true;
		}
	} else {
		//the individual parse functions are responsible for setting the error message when they fail
		return false
	}
}


parse_pattern(tokens, script, variables, errorMsg)
{

	if(!match_token(PATTERN)) {
		*errorMsg = "Error: expected "pattern""
		return false
	}
	if(!match_token(PARSTART)) {
		*errorMsg = "Error: expected \"(\" after \"pattern\""
		return false
	}
	
	vector <AdditivveExprNode*> elements = {}

	//parse additive expressions until we reach the end of the closed parenthesis
	while(lookahead != nothing && lookahead != PAREND) {
		Option<AdditiveExprNode> expr = parse_additive_expr()
		if(expr is empty) return false;
		elements.push_back(expr)
	}

	//check that there is at least one element
	if(elements.empty()) {
		*errorMsg = "Error: empty pattern block"
		return false
	}

	//match the closed paren
	if(!match_token(tokens, PAREND)){
		*errorMsg = "Error: pattern block missing closed parenthesis ')'."
		return false 
	}

	//add the pattern node to the script
	script.push_back(new patternNode(elements))
	return true
}	


parse_let(tokens, script, variables, errorMsg)
{
	if(!match_token(LET)){
		*errorMsg = "Error: expected "let"
		return false
	}

	//extract the name of the var for later
	varToken = lookahead
	
	if(!match_token(ID)){
		*errorMsg = "Error: expected variable name"
		return false
	}

	if(!match_token(EQUALS)){
		*errorMsg = "Error: expected "="
		return false
	}

	Option<AdditiveExprNode> expr = parse_additive_expr()
	if(expr is empty) return false;
	elements.push_back(expr)
}

parse_loop()
{
	//parse loop start bracket '['
	if (!match_token(tokens, BARSTART)) {
        *errorMsg = "Error: expected '[' at start of loop"
        return false
    }

	//parse loop body statements
    scriptnode list loopBody = {}
    while (lookahead(tokens) != nothing && lookahead(tokens) != BAREND) {
		bool success = parse_statement()
        if (!success) return false
	}
	
	//parse iteration count expression
	AdditiveExprNode* iterations = parse_additive_expr()
    if (iterations == nullptr) {
        *errorMsg = "Error: expected loop iteration count after ']'"
        return false
    }

	//add loop to script
	script.push_back(new LoopNode(loopBody, iterations))
	return true
}


bool parse_note()
{
	//parse frequency epxression
	AdditiveExprNode* freqExpr = parse_additive_expr()
	if(!freqExpr) {
		*errorMsg = "Error: invalid freq in note"
		return false
	}

	//parse duration
	AdditiveExprNode* durationExpr = parse_additive_expr()
	if(!durationExpr){
		*errorMsg = "Error: invalid duration in note"
		return false
	}

	script.push_back(new NoteNode(freqExpr, durationExpr))
	return true
}


AdditiveExprNode* parse_additive_expr()
{
	AdditiveExprNode* left = parse_multiplicative_expr()
	AdditiveExprNode* right = nullptr
	if(!left) return nullptr

	opType = lookahead.type
	if(opType = ADD || opType = SUB){
		match_token(nextTokType)
		additiveExprNode* right = parse_additive_expr()

		if(!right){
			//we have an additive symbol without anything after it. Bad!
			*errorMsg = "Error: expected stuff after the " + optype.toString
			return nullptr
		}
		left = new AdditiveExprNode(left, right opType)
	}

	return left
}


MultiplicativeExprNode* parse_multiplicative_expr()
{
	PrimaryExprNode* left = parse_primary_expr()
	if(!left) return nullptr

	opType = lookahead.type
	if (opType = MUL || opType = DIV){
		match_token(nextTokType)
		PrimaryExprNode* right = parse_primary_expr()

		if(!right){
			//we have an additive symbol without anything after it. Bad!
			*errorMsg = "Error: expected stuff after the " + optype.toString
			return nullptr
		}
		left = new MultiplicativeExprNode(left, right, opType)
	}

	return left
}


PrimaryExprNode* parse_primary_expr()
{
	Token tok = lookahead
	if(!tok) {
		errorMsg = "Error: unexpected end of input"
		return nullptr
	}

	if(match_token(INT)){
		return new IntNode(tok.value)
	} 
	else if(match_token(ID)){
		return new VarReferenceNode(tok.varName)
	}
	else if(match_token(RAND){
		if (!match_token(tokens, PARSTART)) {
			errorMsg = "Expected '(' after rand";
			return nullptr;
		}

		 AdditiveExprNode* min = parse_additive_expr(tokens, errorMsg);
		if (!min){
			*errorMsg = "Error: rand min missing"
			return nullptr
		} 


		AdditiveExprNode* max = parse_additive_expr(tokens, errorMsg);
		if (!max) {
			*errorMsg = "Error: rand max missing"
			delete min
			return nullptr
		}

		if (!match_token(tokens, PAREND)) {
			errorMsg = "Error: Expected ')' to close rand";
			return nullptr;
    	}
		return new RandomNode(min, max)
	}
	else if(match_token(PARSTART)) {
		AdditiveExprNode* expr = parse_additive_expr(tokens, errorMsg);
		if(!expr) return nullptr;

		if (!match_token(tokens, PAREND)) {
            *errorMsg = "Error: Missing closing parenthesis";
            return nullptr;
        }

		return new GroupedExprNode(expr)
	} else {
		*errorMsg = "Error: unexpected token: " + tox.lexeme
		return nullptr
	}
}
