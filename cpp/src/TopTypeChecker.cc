// Copyright 2013 <Michael Thorpe>
//
//  type_checker.cpp
//  ponyC
//
//  Created by Michael Thorpe on 18/12/2012.
//
//

#include "TopTypeChecker.h"

#include <assert.h>
#include <iostream>
#include <set>

#include "Loader.h"
#include "Primitives.h"

#define debug(x)    (std::cout << x << std::endl)

static std::string extractName(AST* const ast) {
    if (ast->t->id == TokenType::TK_TYPEID) {
        return ast->t->string;
    }

    std::string res;

    for (auto children : ast->children) {
        if (children == nullptr)
            continue;
        res = extractName(children);
        if (res.compare("") != 0) {
            return res;
        }
    }

    return "";
}

static void extractMixins(AST* const ast, std::vector<std::string> &mixins) {
    assert(ast->t->id == TokenType::TK_IS);
    
    if (ast != nullptr) {
        AST* curr = ast->children.at(0);
        while (curr != nullptr) {
            // Heh, hacky
            mixins.push_back(curr->children.at(0)->t->string);
            curr = curr->sibling;
        }
    }
}

Type* TopTypeChecker::newType(AST* const ast, Kind k, std::set<ClassContents> contents) {
    assert(ast->t->id == TokenType::TK_OBJECT
           || ast->t->id == TokenType::TK_TRAIT
           || ast->t->id == TokenType::TK_ACTOR
           || ast->t->id == TokenType::TK_DECLARE);
    
    auto mixins = std::vector<std::string>();
    auto name = extractName(ast->children.at(0));

    extractMixins(ast->children.at(2), mixins);
    return new Type(name, k, ast, mixins, contents);
}

Mode TopTypeChecker::getMode(AST* const ast) {
    // since we treat emptiness as readonly:
    
    if (ast == nullptr) return Mode::READONLY;
    else {
        switch (ast->t->id) {
            case TokenType::TK_BANG:
                return Mode::IMMUTABLE;
            case TokenType::TK_UNIQ:
                return Mode::UNIQUE;
            case TokenType::TK_MUT:
                return Mode::MUTABLE;
            case TokenType::TK_MODE:
                return Mode::READONLY;
                
            default:
                this->tc->errorList.push_back(Error(ast->t->fileName, ast->t->line, ast->t->linePos, "Mode is broken"));
                return Mode::READONLY;
        }
    }
}

static std::string getType(AST* const ast) {
    if (ast == nullptr) {
        debug("null pointer passed to getType");
        return "";
    }

    switch (ast->children.at(0)->t->id) {
        case TokenType::TK_PARTIAL:
            break;
        case TokenType::TK_TYPEID:
            debug("The type is: " + ast->children.at(0)->t->string + " !");
            return ast->children.at(0)->t->string;
        case TokenType::TK_LAMBDA:
            break;
        default:
            break;
    }
    debug("here");
    return "";
}

void TopTypeChecker::getTypeList(AST* const ast, std::vector<std::string> &types) {
    AST* current = ast;

    while (current != nullptr) {
        assert(current->t->id == TokenType::TK_OFTYPE);
        types.push_back(getType(current->children.at(0)));
        current = current->sibling;
    }
}

void TopTypeChecker::getArgsList(AST* const ast, std::vector<Parameter> &types) {
    AST* current = ast;

    while (current != nullptr) {
        assert(current->t->id == TokenType::TK_ARGS);

        AST* a = current->children.at(0);

        if (a != nullptr) {
            auto type = std::vector<std::string>();
            getTypeList(a->children.at(1), type);
            types.push_back(Parameter(a->children.at(0)->t->string, type));
        }

        current = current->sibling;
    }
}

ClassContents* TopTypeChecker::newVarContent(AST* const ast) {
    auto type = std::vector<std::string>();
    getTypeList(ast, type);
    Field* v = new Field(ast->children.at(0)->t->string, type, ast);
    return v;
}

ClassContents* TopTypeChecker::newFunctionContent(AST* const ast) {
    auto inputs = std::vector<Parameter>();
    auto outputs = std::vector<Parameter>();

    getArgsList(ast->children.at(3), inputs);
    getArgsList(ast->children.at(4), outputs);
    
    return new Function(getMode(ast->children.at(0)), inputs, outputs, ast->children.at(1)->t->string, ast);
}

std::set<ClassContents> TopTypeChecker::collectFunctions(AST* const ast) {
    auto contents = std::set<ClassContents>();

    AST* node = ast;

    while (node != nullptr) {
        switch (node->t->id) {
            case TokenType::TK_VAR:
                debug("var declaration");
                contents.insert(*newVarContent(node));
                break;
            case TokenType::TK_DELEGATE:
                debug("delegate");
                contents.insert(Delegate(node->children.at(0)->t->string,node));
                break;
            case TokenType::TK_NEW:
                debug("constructor");
                contents.insert(Constructor(node->children.at(1)->t->string,node));
                break;
            case TokenType::TK_AMBIENT:
                debug("ambient");
                contents.insert(Ambient(node->children.at(0)->t->string,node));
                break;
            case TokenType::TK_FUNCTION:
                debug("function");
                contents.insert(*newFunctionContent(node));
                break;
            case TokenType::TK_MESSAGE:
                debug("message");
                contents.insert(Message(node->children.at(0)->t->string,node));
                break;
            default:
                break;
        }

        node = node->sibling;
    }

    return contents;
}

void TopTypeChecker::recurseSingleTopAST(AST* const ast, std::set<Type> &typeList,
                                      std::set<CompilationUnit> &imports) {
    if (ast == nullptr)
        return;

    switch (ast->t->id) {
        case TokenType::TK_OBJECT:
            typeList.insert(*newType(ast,
                                    Kind::TYPE_OBJECT,
                                    collectFunctions(ast->children.at(3)->children.at(0))));
            break;
        case TokenType::TK_TRAIT:
            typeList.insert(*newType(ast,
                                    Kind::TYPE_TRAIT,
                                    collectFunctions(ast->children.at(3)->children.at(0))));
            break;
        case TokenType::TK_ACTOR:
            typeList.insert(*newType(ast,
                                    Kind::TYPE_ACTOR,
                                    collectFunctions(ast->children.at(3)->children.at(0))));
            break;
        case TokenType::TK_DECLARE:
            // Are declarations types? (yes - mappings from one type to another)
            typeList.insert(Type(extractName(ast), Kind::TYPE_DECLARE, ast));
            break;
        case TokenType::TK_USE:
        {
            std::string importName = ast->children.at(1)->t->string;
            auto package = Loader::Load(this->tc->unit.directoryName, importName);
            package->buildUnit();

            // For now don't both with the type-id

            imports.insert(*package);
            break;
        }
        case TokenType::TK_MODULE:
            break;
        default:
            // Not a top level declaration
            return;
    }

    for (auto children : ast->children) {
        recurseSingleTopAST(children, typeList, imports);
    }

    recurseSingleTopAST(ast->sibling, typeList, imports);
}

void TopTypeChecker::checkNameClashes() {
    for (auto ast : this->tc->fullASTList) {
        for (auto type : ast->topLevelDecls) {
            auto name = type.name;

            if (tc->typeNames.find(name) == tc->typeNames.end()) {
                tc->typeNames.insert(name);
            } else {
                this->tc->errorList.push_back(
                                Error(type.ast->t->fileName,
                                      type.ast->t->line,
                                      type.ast->t->linePos,
                                    "Name clash " +
                                      name +
                                      " found multiple times " +
                                      "in the current module"));
            }
        }
    }
}

void TopTypeChecker::topLevelTypes() {

    // All compilation units
    for (auto ast : this->tc->astList) {
        std::cout << "Typechecking file: " << ast->t->fileName << std::endl;

        // Collections holding topLevel types
        // and the imports.
        auto topLevel = std::set<Type>();
        auto imports = std::set<CompilationUnit>();

        recurseSingleTopAST(ast, topLevel, imports);

        auto fullAST = new FullAST(ast, imports, topLevel);

        tc->fullASTList.insert(fullAST);
    }

    this->checkNameClashes();
}

void TopTypeChecker::typeCheck() {
    this->topLevelTypes();

    if (this->tc->errorList.size() > 0) {
        std::cout << "Errors detected in top level types" << std::endl;
    }

    for (auto error : this->tc->errorList) {
        std::cout << "Error at " << error.prog_name << "\t"
            << error.line << ":" << error.line_pos << "\t"
            << error.message << std::endl;
    }
}