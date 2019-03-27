#include "TypeHandler.h"

#include "llvm/IR/DerivedTypes.h"

#include "../Program.h"

std::unique_ptr<Type> TypeHandler::getType(const llvm::Type* type, bool voidType) {
    if (type->isArrayTy()) {
        return std::make_unique<ArrayType>(getType(type->getArrayElementType(), voidType), type->getArrayNumElements());
    }

    if (type->isVoidTy()) {
        return std::make_unique<VoidType>();
    }

    if (type->isIntegerTy()) {
        const auto intType = static_cast<const llvm::IntegerType*>(type);
        if (intType->getBitWidth() == 1) {
            return std::make_unique<IntType>(false);
        }

        if (intType->getBitWidth() <= 8) {
            if (voidType) {
                return std::make_unique<VoidType>();
            }
            return std::make_unique<CharType>(false);
        }

        if (intType->getBitWidth() <= 16) {
            return std::make_unique<ShortType>(false);
        }

        if (intType->getBitWidth() <= 32) {
            return std::make_unique<IntType>(false);
        }

        if (intType->getBitWidth() <= 64) {
            return std::make_unique<LongType>(false);
        }

        return std::make_unique<UInt128>();
    }

    if (type->isFloatTy()) {
        return std::make_unique<FloatType>();
    }

    if (type->isDoubleTy()) {
        return std::make_unique<DoubleType>();
    }

    if (type->isX86_FP80Ty()) {
        return std::make_unique<LongDoubleType>();
    }

    if (type->isPointerTy()) {
        const llvm::PointerType* PT = llvm::cast<const llvm::PointerType>(type);
        return std::make_unique<PointerType>(getType(PT->getPointerElementType(), voidType));
    }

    if (type->isStructTy()) {
        const llvm::StructType* structType = llvm::dyn_cast<const llvm::StructType>(type);

        if (!structType->hasName()) {
            if (program->unnamedStructs.find(structType) == program->unnamedStructs.end()) {
                program->createNewUnnamedStruct(structType);
            }
            createNewUnnamedStructType(structType, program->unnamedStructs[structType]->toString());
            return unnamedStructs[structType]->clone();
        }

        if (structType->getName().str().compare("struct.__va_list_tag") == 0) {
            return std::make_unique<StructType>("__va_list_tag");
        }

        if (structType->getName().str().substr(0, 6).compare("struct") == 0) {
            return std::make_unique<StructType>(structType->getName().str().erase(0, 7));
        } else {
            //union
            return std::make_unique<StructType>(structType->getName().str().erase(0, 6));
        }
    }

    if (type->isFunctionTy()) {
        const llvm::FunctionType* FT = llvm::cast<llvm::FunctionType>(type);
        auto functionType = std::make_unique<FunctionType>(getType(FT->getReturnType(), voidType));
        if (FT->getNumParams() == 0) {
            functionType->addParam(std::make_unique<VoidType>());
        } else {
            for (unsigned i = 0; i < FT->getNumParams(); i++) {
                functionType->addParam(getType(FT->getParamType(0), voidType));
            }
        }

        return functionType;
    }

    return nullptr;
}

std::unique_ptr<Type> TypeHandler::getBinaryType(const Type* left, const Type* right) {
    if (const auto LDT = dynamic_cast<const LongDoubleType*>(left)) {
        return std::make_unique<LongDoubleType>();
    }
    if (const auto LDT = dynamic_cast<const LongDoubleType*>(right)) {
        return std::make_unique<LongDoubleType>();
    }

    if (const auto DT = dynamic_cast<const DoubleType*>(left)) {
        return std::make_unique<DoubleType>();
    }
    if (const auto DT = dynamic_cast<const DoubleType*>(right)) {
        return std::make_unique<DoubleType>();
    }

    if (const auto FT = dynamic_cast<const FloatType*>(left)) {
        return std::make_unique<FloatType>();
    }
    if (const auto FT = dynamic_cast<const FloatType*>(right)) {
        return std::make_unique<FloatType>();
    }

    if (const auto UI = dynamic_cast<const UInt128*>(left)) {
        return std::make_unique<UInt128>();
    }
    if (const auto UI = dynamic_cast<const UInt128*>(right)) {
        return std::make_unique<UInt128>();
    }

    if (const auto LT = dynamic_cast<const LongType*>(left)) {
        return std::make_unique<LongType>(LT->unsignedType);
    }
    if (const auto LT = dynamic_cast<const LongType*>(right)) {
        return std::make_unique<LongType>(LT->unsignedType);
    }

    if (const auto IT = dynamic_cast<const IntType*>(left)) {
        return std::make_unique<IntType>(IT->unsignedType);
    }
    if (const auto IT = dynamic_cast<const IntType*>(right)) {
        return std::make_unique<IntType>(IT->unsignedType);
    }

    if (const auto ST = dynamic_cast<const ShortType*>(left)) {
        return std::make_unique<ShortType>(ST->unsignedType);
    }
    if (const auto ST = dynamic_cast<const ShortType*>(right)) {
        return std::make_unique<ShortType>(ST->unsignedType);
    }

    if (const auto CT = dynamic_cast<const CharType*>(left)) {
        return std::make_unique<CharType>(CT->unsignedType);
    }
    if (const auto CT = dynamic_cast<const CharType*>(right)) {
        return std::make_unique<CharType>(CT->unsignedType);
    }

    return nullptr;
}

void TypeHandler::createNewUnnamedStructType(const llvm::StructType* structPointer, const std::string& structString) {
    if (unnamedStructs.find(structPointer) == unnamedStructs.end()) {
        unnamedStructs[structPointer] = std::make_unique<UnnamedStructType>(structString);
    }
}