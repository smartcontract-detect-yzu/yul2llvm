#include <libYulAST/YulASTVisitor/CodegenVisitor.h>
#include <libYulAST/YulASTVisitor/IntrinsicHelper.h>

YulIntrinsicHelper::YulIntrinsicHelper(LLVMCodegenVisitor &v) : visitor(v) {}

void printFunction(llvm::Function *enclosingFunction) {
  enclosingFunction->print(llvm::outs());
  llvm::outs() << "\n";
}

llvm::Function *
YulIntrinsicHelper::getOrCreateFunction(std::string name,
                                        llvm::FunctionType *FT) {
  llvm::Function *F = nullptr;
  F = visitor.getModule().getFunction(name);
  if (F)
    return F;
  else {
    F = llvm::Function::Create(FT,
                               llvm::Function::LinkageTypes::ExternalLinkage,
                               name, visitor.getModule());
    return F;
  }
}

llvm::Value *
YulIntrinsicHelper::getPointerToStorageVarByName(std::string name) {
  auto structFieldOrder = visitor.currentContract->getStructFieldOrder();
  auto typeMap = visitor.currentContract->getTypeMap();
  auto fieldIt =
      std::find(structFieldOrder.begin(), structFieldOrder.end(), name);
  assert(fieldIt != structFieldOrder.end());
  int structIndex = fieldIt - structFieldOrder.begin();
  llvm::SmallVector<llvm::Value *> indices;
  indices.push_back(
      llvm::ConstantInt::get(visitor.getContext(), llvm::APInt(32, 0, false)));
  indices.push_back(llvm::ConstantInt::get(
      visitor.getContext(), llvm::APInt(32, structIndex, false)));
  llvm::Value *ptr = visitor.getBuilder().CreateGEP(
      visitor.getSelfType(), (llvm::Value *)visitor.getSelf(), indices,
      "ptr_self_" + name);
  return ptr;
}

llvm::FunctionType *
YulIntrinsicHelper::getFunctionType(YulFunctionCallNode &node,
                                    llvm::SmallVector<llvm::Value *> &argsV) {
  auto funcArgTypes = getFunctionArgTypes(node.getCalleeName(), argsV);
  auto retType = getReturnType(node.getCalleeName());
  return llvm::FunctionType::get(retType, funcArgTypes, false);
}

llvm::Type *YulIntrinsicHelper::getReturnType(llvm::StringRef calleeName) {
  if (calleeName == "revert")
    return llvm::Type::getVoidTy(visitor.getContext());
  else if (calleeName == "__pyul_map_index")
    return llvm::Type::getIntNPtrTy(visitor.getContext(), 256);
  else if (calleeName == "pyul_storage_var_update")
    return llvm::Type::getVoidTy(visitor.getContext());
  else if (calleeName == "__pyul_storage_var_dynamic_load")
    return llvm::Type::getIntNTy(visitor.getContext(), 256);
  else if (calleeName.startswith("abi_encode_"))
    return llvm::Type::getIntNPtrTy(visitor.getContext(), 256);
  return llvm::Type::getIntNTy(visitor.getContext(), 256);
}

llvm::SmallVector<llvm::Type *> YulIntrinsicHelper::getFunctionArgTypes(
    std::string calleeName, llvm::SmallVector<llvm::Value *> &argsV) {
  llvm::SmallVector<llvm::Type *> funcArgTypes;
  if (calleeName == "__pyul_storage_var_load") {
    funcArgTypes.push_back(llvm::Type::getIntNPtrTy(visitor.getContext(), 256));
  } else if (calleeName == "__pyul_storage_var_update") {
    funcArgTypes.push_back(llvm::Type::getIntNPtrTy(visitor.getContext(), 256));
    funcArgTypes.push_back(llvm::Type::getIntNPtrTy(visitor.getContext(), 256));
  } else if (!calleeName.compare("__pyul_map_index")) {
    funcArgTypes.push_back(llvm::Type::getIntNPtrTy(visitor.getContext(), 256));
    funcArgTypes.push_back(llvm::Type::getIntNTy(visitor.getContext(), 256));
  } else if (!calleeName.compare("__pyul_storage_var_dynamic_load")) {
    funcArgTypes.push_back(llvm::Type::getIntNTy(visitor.getContext(), 256));
    funcArgTypes.push_back(llvm::Type::getIntNTy(visitor.getContext(), 256));
  } else {
    for (auto &arg : argsV) {
      funcArgTypes.push_back(arg->getType());
    }
  }
  return funcArgTypes;
}