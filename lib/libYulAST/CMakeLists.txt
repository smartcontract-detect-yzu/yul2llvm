add_library(libYulAST 
  YulASTBase.cpp
  YulAssignmentNode.cpp 
  YulBlockNode.cpp
  YulExpressionNode.cpp
  YulStatementNode.cpp
  YulIdentifierNode.cpp
  YulFunctionCallNode.cpp
  YulTypedIdentifierListNode.cpp
  YulIdentifierListNode.cpp
  YulFunctionArgListNode.cpp
  YulFunctionDefinitionNode.cpp
  YulFunctionRetListNode.cpp
  YulLeaveNode.cpp
  YulVariableDeclarationNode.cpp
  YulNodeBuilder.cpp
  YulLiteralNode.cpp
  YulNumberLiteralNode.cpp
  YulIfNode.cpp
  YulSwitchNode.cpp
  YulCaseNode.cpp
  YulDefaultNode.cpp
  YulBreakNode.cpp
  YulContinueNode.cpp
  YulForNode.cpp
  YulStringLiteralNode.cpp
  YulContractNode.cpp
  )

add_subdirectory(YulASTVisitor)

target_include_directories(libYulAST PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_link_libraries(libYulAST PUBLIC
  nlohmann_json::nlohmann_json
)
