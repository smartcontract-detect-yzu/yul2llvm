#include <libYulAST/YulASTVisitor/CodegenVisitor.h>
#include <libYulAST/YulASTVisitor/IntrinsicHelper.h>

bool YulIntrinsicHelper::isFunctionCallIntrinsic(llvm::StringRef calleeName) {
  if (calleeName == "checked_add_t_uint256") {
    return true;
  } else if (calleeName == "mstore") {
    return true;
  } else if (calleeName == "add") {
    return true;
  } else if (calleeName == "sub") {
    return true;
  } else if (calleeName == "shl") {
    return true;
  } else if (calleeName == "allocate_unbounded") {
    return true;
  }
  return false;
}

bool YulIntrinsicHelper::skipDefinition(llvm::StringRef calleeName) {
  if (calleeName.startswith("abi_encode_")) {
    return true;
  } else if (calleeName.startswith("abi_decode_tuple_")) {
    return true;
  } else if (calleeName.startswith("finalize_allocation")) {
    return true;
  } else
    return false;
}

llvm::Value *
YulIntrinsicHelper::handleIntrinsicFunctionCall(YulFunctionCallNode &node) {
  std::string calleeName = node.getCalleeName();
  if (!calleeName.compare("checked_add_t_uint256")) {
    return handleAddFunctionCall(node);
  } else if (!calleeName.compare("mstore")) {
    return handleMStoreFunctionCall(node);
  } else if (!calleeName.compare("add")) {
    return handleAddFunctionCall(node);
  } else if (!calleeName.compare("sub")) {
    return handleSubFunctionCall(node);
  } else if (!calleeName.compare("shl")) {
    return handleShl(node);
  } else if (calleeName == "allocate_unbounded") {
    return handleAllocateUnbounded(node);
  }
  return nullptr;
}

llvm::Value *
YulIntrinsicHelper::handleAllocateUnbounded(YulFunctionCallNode &node) {
  return visitor.CreateEntryBlockAlloca(
      visitor.currentFunction, "alloc_unbounded",
      llvm::Type::getIntNTy(visitor.getContext(), 256));
}
llvm::Value *YulIntrinsicHelper::handleShl(YulFunctionCallNode &node) {
  assert(node.getArgs().size() == 2 &&
         "Incorrect number of args in shl instruction handler");
  auto &builder = visitor.getBuilder();
  llvm::Value *v1, *v2;
  v1 = visitor.visit(*node.getArgs()[0]);
  v2 = visitor.visit(*node.getArgs()[1]);
  return builder.CreateShl(v1, v2);
}

/**
 * @brief Contract: The caller confirms that only one of the value is pointer
 * and other is a primitive type.
 *
 * @param v1
 * @param v2
 * @return llvm::Value*
 */
llvm::Value *YulIntrinsicHelper::handlePointerAdd(llvm::Value *v1,
                                                  llvm::Value *v2) {
  llvm::Value *ptr, *primitive;
  assert((v1->getType()->isPointerTy() ^ v2->getType()->isPointerTy()) &&
         "Both values are pointer in handlePointerArithmetic");
  ptr = v1->getType()->isPointerTy() ? v1 : v2;
  primitive = v1->getType()->isPointerTy() ? v2 : v1;
  assert(primitive->getType()->isIntegerTy() &&
         "primitive is not integer in pointer addition");
  llvm::SmallVector<llvm::Value *> index = {primitive};
  llvm::Type *bytePtrType = llvm::Type::getInt8PtrTy(visitor.getContext());
  llvm::ArrayType *arrayfied = llvm::ArrayType::get(bytePtrType, 0);
  ptr = visitor.getBuilder().CreatePointerCast(ptr, arrayfied->getPointerTo(),
                                               "ptr1");
  ptr->print(llvm::outs(), false);
  llvm::outs() << "\n";
  arrayfied->print(llvm::outs(), false);

  return visitor.getBuilder().CreateGEP(arrayfied, ptr, index, "asfd");
}
/**
 * @brief Here the contract is that atleast one of the argument is
 * pointer type. This function handles two cases seperately
 * 1. If only one is pointer
 * 2. If both are pointers
 *
 * @param v1
 * @param v2
 * @return llvm::Value*
 */
llvm::Value *YulIntrinsicHelper::handlePointerSub(llvm::Value *v1,
                                                  llvm::Value *v2) {
  llvm::Value *ptr, *primitive;
  // if only one is prmitive
  if (v1->getType()->isPointerTy() ^ v2->getType()->isPointerTy()) {
    ptr = v1->getType()->isPointerTy() ? v1 : v2;
    primitive = v1->getType()->isPointerTy() ? v2 : v1;
    assert(primitive->getType()->isIntegerTy() &&
           "Non integer sub from prmitive");
    llvm::Value *negated = visitor.getBuilder().CreateMul(
        primitive,
        llvm::ConstantInt::get(
            primitive->getType(),
            llvm::APInt(-1, primitive->getType()->getIntegerBitWidth())));
    return handlePointerAdd(ptr, negated);
  } else {
    assert(v1->getType() == v2->getType() &&
           "pointer diff between different pointers");
    llvm::Type *destType = llvm::Type::getIntNTy(
        visitor.getContext(),
        visitor.getModule().getDataLayout().getPointerSize());
    llvm::Value *castedV1 = visitor.getBuilder().CreateCast(
        llvm::Instruction::CastOps::PtrToInt, v1, destType, "v1");
    llvm::Value *castedV2 = visitor.getBuilder().CreateCast(
        llvm::Instruction::CastOps::PtrToInt, v2, destType, "v2");
    return visitor.getBuilder().CreateSub(castedV1, castedV2);
  }
}

llvm::Value *
YulIntrinsicHelper::handleAddFunctionCall(YulFunctionCallNode &node) {
  llvm::IRBuilder<> &Builder = visitor.getBuilder();
  llvm::Value *v1, *v2;
  v1 = visitor.visit(*node.getArgs()[0]);
  v2 = visitor.visit(*node.getArgs()[1]);
  // Only one is pointer other is a scalar type
  if (v1->getType()->isPointerTy() ^ v2->getType()->isPointerTy()) {
    return handlePointerAdd(v1, v2);
  } // both values are same type, make sure they are not both pointer type
  else if (v1->getType()->isPointerTy() && v2->getType()->isPointerTy()) {
    assert(false && "pointer arithmetic with both pointers");
  } // both scalars
  return Builder.CreateAdd(v1, v2);
}

llvm::Value *
YulIntrinsicHelper::handleSubFunctionCall(YulFunctionCallNode &node) {
  llvm::IRBuilder<> &Builder = visitor.getBuilder();
  llvm::Value *v1, *v2;
  v1 = visitor.visit(*node.getArgs()[0]);
  v2 = visitor.visit(*node.getArgs()[1]);
  if (v1->getType()->isPointerTy() || v2->getType()->isPointerTy()) {
    return handlePointerSub(v1, v2);
  }
  return Builder.CreateSub(v1, v2);
}

llvm::Value *
YulIntrinsicHelper::handleMStoreFunctionCall(YulFunctionCallNode &node) {
  assert(node.getArgs().size() == 2 &&
         "Incorrect number of arguments to mstore");
  llvm::Value *val, *ptr;
  ptr = visitor.visit(*(node.getArgs()[0]));
  val = visitor.visit(*(node.getArgs()[1]));
  /**
   *  @todo: This pointer cast may not be correct check again
   */
  if (!ptr->getType()->isPointerTy())
    ptr = visitor.getBuilder().CreateIntToPtr(ptr,
                                              val->getType()->getPointerTo());
  visitor.getBuilder().CreateStore(val, ptr);
  return nullptr;
}