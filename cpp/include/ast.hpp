#ifndef ponyC_ast_hpp
#define ponyC_ast_hpp

#define AST_SLOTS 7

#include <string>
#include <map>

typedef enum {
    // primitives
    TK_STRING   = 0,
    TK_INT      = 1,
    TK_FLOAT    = 2,
    TK_ID       = 3,
    TK_TYPEID   = 4,
    
    // symbols
    TK_LBRACE   = 5,
    TK_RBRACE   = 6,
    TK_LPAREN   = 7,
    TK_RPAREN   = 8,
    TK_LBRACKET = 9,
    TK_RBRACKET = 10,
    TK_COMMA    = 11,
    TK_RESULTS  = 12,
    
    TK_CALL     = 13,
    TK_PACKAGE  = 14,
    TK_OFTYPE   = 15,
    TK_PARTIAL  = 16,
    TK_ASSIGN   = 17,
    TK_BANG     = 18,
    
    TK_PLUS     = 19,
    TK_MINUS    = 20,
    TK_MULTIPLY = 21,
    TK_DIVIDE   = 22,
    TK_MOD      = 23,
    
    TK_LSHIFT   = 24,
    TK_RSHIFT   = 25,
    
    TK_LT       = 26,
    TK_LE       = 27,
    TK_GE       = 28,
    TK_GT       = 29,
    
    TK_EQ       = 30,
    TK_NOTEQ    = 31,
    TK_STEQ     = 32,
    TK_NSTEQ    = 33,
    
    TK_OR       = 34,
    TK_AND      = 35,
    TK_XOR      = 36,
    
    TK_UNIQ     = 37,
    TK_MUT      = 38,
    TK_MODE     = 39,
    
    // keywords
    TK_USE      = 40,
    TK_DECLARE  = 41,
    TK_TYPE     = 42,
    TK_LAMBDA   = 43,
    TK_TRAIT    = 44,
    TK_OBJECT   = 45,
    TK_ACTOR    = 46,
    TK_IS       = 47,
    TK_VAR      = 48,
    TK_DELEGATE = 49,
    TK_NEW      = 50,
    TK_AMBIENT  = 51,
    TK_FUNCTION = 52,
    TK_MESSAGE  = 53,
    TK_THROWS   = 54,
    TK_THROW    = 55,
    TK_RETURN   = 56,
    TK_BREAK    = 57,
    TK_CONTINUE = 58,
    TK_IF       = 59,
    TK_ELSE     = 60,
    TK_FOR      = 61,
    TK_IN       = 62,
    TK_WHILE    = 63,
    TK_DO       = 64,
    TK_MATCH    = 65,
    TK_CASE     = 66,
    TK_AS       = 67,
    TK_CATCH    = 68,
    TK_ALWAYS   = 69,
    TK_THIS     = 70,
    TK_TRUE     = 71,
    TK_FALSE    = 72,
    
    // abstract
    TK_MODULE   = 73,
    TK_DECLAREMAP=74,
    TK_MAP      = 75,
    TK_TYPEBODY = 76,
    TK_TYPECLASS= 77,
    TK_FORMALARGS=78,
    TK_FIELD    = 79,
    TK_ARG      = 80,
    TK_ARGS     = 81,
    TK_BLOCK    = 82,
    TK_CASEVAR  = 83,
    TK_LIST     = 84,
    
    TK_SCOLON   = 85,
    TK_PACKAGEDEC = 86,
    
    TK_EOF      = 87
    
} tok_type;

typedef enum {
    TYPE_TRAIT,
    TYPE_ACTOR,
    TYPE_OBJECT,
    TYPE_PRIMITIVE,
    TYPE_FUNCTION,
    TYPE_DECLARE
} Kind;

typedef struct Token {
    
    tok_type id;
    unsigned int line;
    unsigned int line_pos;
    std::string* fileName;
    
    union {
        std::string* string;
        double flt;
        unsigned int integer;
    };
    
} Token;

typedef struct Type Type;

typedef struct AST {
    Token* t;
    struct AST* sibling;
    std::vector<AST*>* children;
    std::map<std::string, Type*>* types;
} AST;

typedef struct Type {
    std::string name;
    std::string type;
    
    Kind kind;
    
    AST* ast;
    std::vector<std::string>* mixins;
} Type;



#endif
