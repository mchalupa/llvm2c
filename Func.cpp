#include "Func.h"

#include <llvm/IR/Module.h>
#include <llvm/IR/Metadata.h>
#include <llvm/Support/raw_ostream.h>

#include "Type.h"

#include <utility>
#include <cstdint>
#include <string>
#include <fstream>

Func::Func(llvm::Function* func, Program* program) {
    this->program = program;
    function = func;
    varCount = 0;
    blockCount = 0;

    if (func->getReturnType()->isArrayTy()) {
        unsigned int size = func->getReturnType()->getArrayNumElements();
        returnType = std::move(Type::getType(func->getReturnType(), true, size));
    } else {
        returnType = std::move(Type::getType(func->getReturnType()));
    }

    parseFunction();
}

std::string Func::getBlockName(const llvm::BasicBlock* block) {
    auto iter = blockMap.find(block);
    if (iter == blockMap.end()) {
        std::string blockName = "block";
        blockName += std::to_string(blockCount);
        blockMap[block] = std::make_unique<Block>(blockName, block, this);
        blockCount++;

        return blockName;
    }

    return iter->second->blockName;
}

Expr* Func::getExpr(const llvm::Value* val) {
    auto iter = exprMap.find(val);

    if (iter != exprMap.end()) {
        return exprMap.find(val)->second.get();
    }

    return nullptr;
}

void Func::createExpr(const llvm::Value* val, std::unique_ptr<Expr> expr) {
    exprMap[val] = std::move(expr);
}

void Func::createExpr(const llvm::Instruction* ins, std::unique_ptr<Expr> expr) {
    createExpr(static_cast<const llvm::Value*>(ins), std::move(expr));
}

std::string Func::getVarName() {
    std::string varName = "var";
    varName += std::to_string(varCount);
    varCount++;

    return varName;
}

void Func::parseFunction() {
    for (const llvm::Value& arg : function->args()) {
        std::string varName = "var";
        varName += std::to_string(varCount);
        exprMap[&arg] = std::make_unique<Value>(varName, std::move(Type::getType(arg.getType())));
        varCount++;
    }

    for (const auto& block : *function) {
        getBlockName(&block);
        blockMap[&block]->parseLLVMBlock();
    }
}

void Func::print() const {
    returnType->print();
    llvm::outs() <<  " " << function->getName().str() << "(";
    bool first = true;

    for (const llvm::Value& arg : function->args()) {
        if (first) {
            exprMap.find(&arg)->second->print();
        } else {
            llvm::outs() << ", ";
            exprMap.find(&arg)->second->print();
        }
        first = false;
    }

    llvm::outs() << ") {\n";

    first = true;
    for (const auto& block : *function) {
        if (!first) {
            llvm::outs() << blockMap.find(&block)->second->blockName;
            llvm::outs() << ":\n";
        }
        blockMap.find(&block)->second->print();
        first = false;
    }

    llvm::outs() << "}\n\n";
}

void Func::saveFile(std::ofstream& file) const {
    file << function->getName().str() << "(";
    bool first = true;

    for (const llvm::Value& arg : function->args()) {
        if (first) {
            file << exprMap.find(&arg)->second->toString();
        } else {
            file << ", ";
            file << exprMap.find(&arg)->second->toString();
        }
        first = false;
    }

    file << ") {\n";

    first = true;
    for (const auto& block : *function) {
        if (!first) {
            file << blockMap.find(&block)->second->blockName;
            file << ":\n";
        }
        blockMap.find(&block)->second->saveFile(file);
        first = false;
    }

    file << "}\n\n";
}

Struct* Func::getStruct(const std::string& name) const {
    return program->getStruct(name);
}
