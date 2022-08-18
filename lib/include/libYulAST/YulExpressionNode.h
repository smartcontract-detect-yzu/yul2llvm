#pragma once
#include <libYulAST/YulStatementNode.h>
#include <nlohmann/json.hpp>

namespace yulast {
class YulExpressionNode : public YulStatementNode {
protected:
public:
  YUL_AST_EXPRESSION_NODE_TYPE expressionType;
  virtual std::string to_string() override;
  YulExpressionNode(const json *rawAST, YUL_AST_EXPRESSION_NODE_TYPE exprType);
  virtual llvm::Value *codegen(llvm::Function *F) override;
  virtual ~YulExpressionNode(){};
};
}; // namespace yulast