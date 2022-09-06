#pragma once
#include <libYulAST/YulASTBase.h>
#include <libYulAST/YulBlockNode.h>
#include <libYulAST/YulFunctionArgListNode.h>
#include <libYulAST/YulFunctionRetListNode.h>
#include <libYulAST/YulIdentifierNode.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Function.h>
#include <nlohmann/json.hpp>

namespace yulast {
class YulFunctionDefinitionNode : public YulStatementNode {
protected:
  std::unique_ptr<YulIdentifierNode> functionName;
  std::unique_ptr<YulFunctionArgListNode> args;
  std::unique_ptr<YulFunctionRetListNode> rets;
  std::unique_ptr<YulBlockNode> body;
  void createPrototype();
  void createVarsForArgsAndRets();
  llvm::FunctionType *FT = NULL;
  llvm::Function *F = NULL;
  std::string str = "";
  virtual void parseRawAST(const json *rawAst) override;
  llvm::Type* getReturnType();

public:
  void dumpToStdout() const;
  virtual llvm::Value *codegen(llvm::Function *F) override;
  virtual std::string to_string() override;
  YulFunctionDefinitionNode(const json *rawAST);
  llvm::Function *getLLVMFunction();
  std::string getName();
  void dump(llvm::raw_ostream &os = llvm::errs()) const;
};
}; // namespace yulast
