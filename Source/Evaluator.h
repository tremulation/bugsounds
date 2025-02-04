/*
  ==============================================================================

    Evaluator.h
    Created: 30 Jan 2025 4:38:46pm
    Author:  Taro

  ==============================================================================
*/

#pragma once

#include "SongCodeCompiler.h"
#include <JuceHeader.h>
#include <vector>
#include <string>
#include <map>


struct StatementNode;
struct ExprNode;
struct ScriptNode;

using ExprPtr = juce::ReferenceCountedObjectPtr<ExprNode>;
using StatementPtr = juce::ReferenceCountedObjectPtr<StatementNode>;
using ScriptPtr = juce::ReferenceCountedObjectPtr<ScriptNode>;


/* 
Error reporting struct 
Should include everything needed to report errors in the UI. 
set inside the evaluator, and used in the ui thread
*/
struct ErrorInfo {
    std::string message;
    size_t errorStart;
    size_t errorEnd;
    std::string tokenText;
};


inline void setErrorInfo(ErrorInfo* errorInfo, const std::string& message,
    size_t start, size_t end, const std::string& text) {
    errorInfo->message = message;
    errorInfo->errorStart = start;
    errorInfo->errorEnd = end;
    errorInfo->tokenText = text;
}



/*
-------------------============ AST NODES ============-------------------
The tokens from the lexer are compiled into a tree of nodes of this structure.
The two overarching node types are statement nodes, and expression nodes.

Statement nodes will evaluate directly into a note, or a pattern (with the
exception of variable declarations). 

Expression nodes always get compiled into numbers. These numbers then form
parts of statements (the subbeats of patterns, or the freq/dur of notes, for
example.


*/



struct StatementNode : public juce::ReferenceCountedObject {
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StatementNode)
    StatementNode() = default;
    virtual ~StatementNode() = default;
};

struct ExprNode : public juce::ReferenceCountedObject{
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ExprNode)
    ExprNode() = default;
    virtual ~ExprNode() = default;
};

//scriptnode is the main one. keeps track of the context
struct ScriptNode : public juce::ReferenceCountedObject {
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ScriptNode)
    //using a vector of statements instead of a statement list for simplicity here
    std::vector<StatementPtr> statements;
    std::map<std::string, float> variables;
    ScriptNode() = default;
};


//notenode: freq and duration
struct NoteNode : StatementNode {
    ExprPtr frequency;
    ExprPtr duration;

    NoteNode(ExprPtr freq, ExprPtr dur) : frequency(freq), duration(dur) {};
};


//patternNode: list of sub beat expresions
struct PatternNode : StatementNode {
    std::vector<ExprPtr> subBeats;
    ~PatternNode() {}
};


//loopNode: loop body, and iteration count
struct LoopNode : StatementNode {
    std::vector<StatementPtr> body;
    ExprPtr iterations;

    LoopNode(std::vector<StatementPtr> body, ExprNode* iter) : body(std::move(body)), iterations(iter) {}
    ~LoopNode() { }
};


//letNode: variable declaration
struct LetNode : StatementNode {
    std::string id;
    ExprNode* value;

    LetNode(const std::string& id, ExprNode* val) : id(id), value(val) {}
    ~LetNode() { delete value; }
};



//additiveExprNode: left, right, operator
struct AdditiveExprNode : ExprNode {
    ExprNode* left;
    ExprNode* right;
    enum Op{Add, subtract} op;

    AdditiveExprNode(ExprNode* l, ExprNode* r, Op op) : left(l), right(r), op(op) {}
    ~AdditiveExprNode() {
        delete left;
        delete right;
    }
};


//multiplicativeExprNode: left, right, operator
struct MultiplicativeExprNode : ExprNode {
    ExprNode* left;
    ExprNode* right;
    enum Op { Multiply, Divide } op;

    MultiplicativeExprNode(ExprNode* l, ExprNode* r, Op op) : left(l), right(r), op(op) {}
    ~MultiplicativeExprNode() {
        delete left;
        delete right;
    }
};


//primaryExprNode: integer, var, or another expr grouped in parenthesis
struct PrimaryExprNode : ExprNode {
    enum Kind { Integer, Variable, Grouped };
    Kind kind;
    union {
        int integerValue;
        std::string variableName;
        ExprNode* groupedExpr;
    };

    PrimaryExprNode(int value) : kind(Integer), integerValue(value) {}
    PrimaryExprNode(const std::string& var) : kind(Variable), variableName(var) {}
    PrimaryExprNode(ExprNode* expr) : kind(Grouped), groupedExpr(expr) {}
    ~PrimaryExprNode() {
        if (kind == Variable) variableName.~basic_string();
        else if (kind == Grouped) delete groupedExpr;
    }
};


// RandomNode: min and max expressions
struct RandomNode : ExprNode {
    ExprNode* min;
    ExprNode* max;

    RandomNode(ExprNode* min, ExprNode* max) : min(min), max(max) {}
    ~RandomNode() {
        delete min;
        delete max;
    }
};

//implied nullptr for vars. If you don't provide a pointer to already-initialized variables, then they will be null
std::vector<SongElement> evaluateSongString(std::string& songcode,
                                            ErrorInfo* errorInfo,
                                            std::map<std::string, float>* vars = nullptr);