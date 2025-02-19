// Copyright 2022 ByteDance Ltd. and/or its affiliates.
/*
 * Acknowledgement: This file originates from TVM.
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
#include <matxscript/runtime/logging.h>
#include <matxscript/runtime/py_commons/pystrtod.h>
#include <matxscript/runtime/registry.h>
#include <matxscript/runtime/str_escape.h>

#include <matxscript/ir/printer/doc.h>

#include <algorithm>
#include <cmath>

#include "./base_doc_printer.h"

namespace matxscript {
namespace ir {
namespace printer {

/*!
 * \brief Operator precedence
 *
 * This is based on
 * https://docs.python.org/3/reference/expressions.html#operator-precedence
 */
enum class ExprPrecedence : int32_t {
  /*! \brief Unknown precedence */
  kUnkown = 0,
  /*! \brief Lambda Expression */
  kLambda = 1,
  /*! \brief Conditional Expression */
  kIfThenElse = 2,
  /*! \brief Boolean OR */
  kBooleanOr = 3,
  /*! \brief Boolean AND */
  kBooleanAnd = 4,
  /*! \brief Boolean NOT */
  kBooleanNot = 5,
  /*! \brief in and not in */
  kIn = 6,
  /*! \brief Comparisons */
  kComparison = 7,
  /*! \brief Bitwise OR */
  kBitwiseOr = 8,
  /*! \brief Bitwise XOR */
  kBitwiseXor = 9,
  /*! \brief Bitwise AND */
  kBitwiseAnd = 10,
  /*! \brief Shift Operators */
  kShift = 11,
  /*! \brief Addition and subtraction */
  kAdd = 12,
  /*! \brief Multiplication, division, floor division, remainder */
  kMult = 13,
  /*! \brief Positive negative and bitwise NOT */
  kUnary = 14,
  /*! \brief Exponentiation */
  kExp = 15,
  /*! \brief Index access, attribute access, call and atom expression */
  kIdentity = 16,
};

ExprPrecedence GetExprPrecedence(const ExprDoc& doc) {
  // Key is the value of OperationDocNode::Kind
  static const std::vector<ExprPrecedence> op_kind_precedence = []() {
    using OpKind = OperationDocNode::Kind;
    std::map<OpKind, ExprPrecedence> raw_table = {
        {OpKind::kUSub, ExprPrecedence::kUnary},
        {OpKind::kInvert, ExprPrecedence::kUnary},
        {OpKind::kNot, ExprPrecedence::kBooleanNot},
        {OpKind::kIn, ExprPrecedence::kIn},
        {OpKind::kNotIn, ExprPrecedence::kIn},
        {OpKind::kAdd, ExprPrecedence::kAdd},
        {OpKind::kSub, ExprPrecedence::kAdd},
        {OpKind::kMult, ExprPrecedence::kMult},
        {OpKind::kDiv, ExprPrecedence::kMult},
        {OpKind::kFloorDiv, ExprPrecedence::kMult},
        {OpKind::kMod, ExprPrecedence::kMult},
        {OpKind::kPow, ExprPrecedence::kExp},
        {OpKind::kLShift, ExprPrecedence::kShift},
        {OpKind::kRShift, ExprPrecedence::kShift},
        {OpKind::kBitAnd, ExprPrecedence::kBitwiseAnd},
        {OpKind::kBitOr, ExprPrecedence::kBitwiseOr},
        {OpKind::kBitXor, ExprPrecedence::kBitwiseXor},
        {OpKind::kLt, ExprPrecedence::kComparison},
        {OpKind::kLtE, ExprPrecedence::kComparison},
        {OpKind::kEq, ExprPrecedence::kComparison},
        {OpKind::kNotEq, ExprPrecedence::kComparison},
        {OpKind::kGt, ExprPrecedence::kComparison},
        {OpKind::kGtE, ExprPrecedence::kComparison},
        {OpKind::kAnd, ExprPrecedence::kBooleanAnd},
        {OpKind::kOr, ExprPrecedence::kBooleanOr},
        {OpKind::kIfThenElse, ExprPrecedence::kIfThenElse},
    };
    int n = static_cast<int>(OpKind::kSpecialEnd);
    std::vector<ExprPrecedence> table(n + 1, ExprPrecedence::kUnkown);
    for (const auto& kv : raw_table) {
      table[static_cast<int>(kv.first)] = kv.second;
    }
    return table;
  }();

  // Key is the type index of Doc
  static const std::unordered_map<uint32_t, ExprPrecedence> doc_type_precedence = {
      {LiteralDocNode::RuntimeTypeIndex(), ExprPrecedence::kIdentity},
      {IdDocNode::RuntimeTypeIndex(), ExprPrecedence::kIdentity},
      {AttrAccessDocNode::RuntimeTypeIndex(), ExprPrecedence::kIdentity},
      {IndexDocNode::RuntimeTypeIndex(), ExprPrecedence::kIdentity},
      {CallDocNode::RuntimeTypeIndex(), ExprPrecedence::kIdentity},
      {LambdaDocNode::RuntimeTypeIndex(), ExprPrecedence::kLambda},
      {TupleDocNode::RuntimeTypeIndex(), ExprPrecedence::kIdentity},
      {ListDocNode::RuntimeTypeIndex(), ExprPrecedence::kIdentity},
      {DictDocNode::RuntimeTypeIndex(), ExprPrecedence::kIdentity},
      {SetDocNode::RuntimeTypeIndex(), ExprPrecedence::kIdentity},
  };

  if (const auto* op_doc = doc.as<OperationDocNode>()) {
    size_t kind = static_cast<int>(op_doc->kind);
    MXCHECK_LT(kind, op_kind_precedence.size()) << "ValueError: Invalid operation: " << kind;
    ExprPrecedence precedence = op_kind_precedence[kind];
    MXCHECK(precedence != ExprPrecedence::kUnkown)
        << "Precedence for operator " << static_cast<int>(op_doc->kind) << " is unknown";
    return precedence;
  }
  auto it = doc_type_precedence.find(doc->type_index());
  if (it != doc_type_precedence.end()) {
    return it->second;
  }
  MXCHECK(false) << "Precedence for doc type " << doc->GetTypeKey() << " is unknown";
  throw;
}

class PythonDocPrinter : public DocPrinter {
 public:
  explicit PythonDocPrinter(const PrinterConfig& options) : DocPrinter(options) {
  }

 protected:
  using DocPrinter::PrintDoc;

  void PrintTypedDoc(const LiteralDoc& doc) final;
  void PrintTypedDoc(const IdDoc& doc) final;
  void PrintTypedDoc(const AttrAccessDoc& doc) final;
  void PrintTypedDoc(const IndexDoc& doc) final;
  void PrintTypedDoc(const OperationDoc& doc) final;
  void PrintTypedDoc(const CallDoc& doc) final;
  void PrintTypedDoc(const LambdaDoc& doc) final;
  void PrintTypedDoc(const ListDoc& doc) final;
  void PrintTypedDoc(const DictDoc& doc) final;
  void PrintTypedDoc(const SetDoc& doc) final;
  void PrintTypedDoc(const TupleDoc& doc) final;
  void PrintTypedDoc(const ComprehensionDoc& doc) final;
  void PrintTypedDoc(const ListCompDoc& doc) final;
  void PrintTypedDoc(const SetCompDoc& doc) final;
  void PrintTypedDoc(const DictCompDoc& doc) final;
  void PrintTypedDoc(const SliceDoc& doc) final;
  void PrintTypedDoc(const YieldDoc& doc) final;
  void PrintTypedDoc(const StmtBlockDoc& doc) final;
  void PrintTypedDoc(const AssignDoc& doc) final;
  void PrintTypedDoc(const IfDoc& doc) final;
  void PrintTypedDoc(const WhileDoc& doc) final;
  void PrintTypedDoc(const ForDoc& doc) final;
  void PrintTypedDoc(const ContinueDoc& doc) final;
  void PrintTypedDoc(const BreakDoc& doc) final;
  void PrintTypedDoc(const ExceptionHandlerDoc& doc) final;
  void PrintTypedDoc(const TryExceptDoc& doc) final;
  void PrintTypedDoc(const RaiseDoc& doc) final;
  void PrintTypedDoc(const ExprStmtDoc& doc) final;
  void PrintTypedDoc(const AssertDoc& doc) final;
  void PrintTypedDoc(const ReturnDoc& doc) final;
  void PrintTypedDoc(const ScopeDoc& doc) final;
  void PrintTypedDoc(const FunctionDoc& doc) final;
  void PrintTypedDoc(const ClassDoc& doc) final;
  void PrintTypedDoc(const CommentDoc& doc) final;
  void PrintTypedDoc(const DocStringDoc& doc) final;
  void PrintTypedDoc(const ModuleDoc& doc) final;

 private:
  void NewLineWithoutIndent() {
    size_t start_pos = output_.tellp();
    output_ << "\n";
    size_t end_pos = output_.tellp();
    underlines_exempted_.push_back({start_pos, end_pos});
  }

  template <typename DocType>
  void PrintJoinedDocs(const Array<DocType>& docs, const std::string& separator) {
    bool is_first = true;
    for (auto& doc : docs) {
      if (is_first) {
        is_first = false;
      } else {
        output_ << separator;
      }
      PrintDoc(doc);
    }
  }

  void PrintIndentedBlock(const Array<StmtDoc>& docs) {
    IncreaseIndent();
    for (const StmtDoc& d : docs) {
      NewLine();
      PrintDoc(d);
    }
    if (docs.empty()) {
      NewLine();
      output_ << "pass";
    }
    DecreaseIndent();
  }

  void PrintDecorators(const Array<ExprDoc>& decorators) {
    for (const ExprDoc& decorator : decorators) {
      output_ << "@";
      PrintDoc(decorator);
      NewLine();
    }
  }

  /*!
   * \brief Print expression and add parenthesis if needed.
   */
  void PrintChildExpr(const ExprDoc& doc,
                      ExprPrecedence parent_precedence,
                      bool parenthesis_for_same_precedence = false) {
    ExprPrecedence doc_precedence = GetExprPrecedence(doc);
    if (doc_precedence < parent_precedence ||
        (parenthesis_for_same_precedence && doc_precedence == parent_precedence)) {
      output_ << "(";
      PrintDoc(doc);
      output_ << ")";
    } else {
      PrintDoc(doc);
    }
  }

  /*!
   * \brief Print expression and add parenthesis if doc has lower precedence than parent.
   */
  void PrintChildExpr(const ExprDoc& doc,
                      const ExprDoc& parent,
                      bool parenthesis_for_same_precedence = false) {
    ExprPrecedence parent_precedence = GetExprPrecedence(parent);
    return PrintChildExpr(doc, parent_precedence, parenthesis_for_same_precedence);
  }

  /*!
   * \brief Print expression and add parenthesis if doc doesn't have higher precedence than parent.
   *
   * This function should be used to print an child expression that needs to be wrapped
   * by parenthesis even if it has the same precedence as its parent, e.g., the `b` in `a + b`
   * and the `b` and `c` in `a if b else c`.
   */
  void PrintChildExprConservatively(const ExprDoc& doc, const ExprDoc& parent) {
    PrintChildExpr(doc, parent, /*parenthesis_for_same_precedence=*/true);
  }

  void MaybePrintCommentInline(const StmtDoc& stmt) {
    if (stmt->comment.defined()) {
      StringRef comment = stmt->comment.value();
      bool has_newline = std::find(comment.begin(), comment.end(), '\n') != comment.end();
      MXCHECK(!has_newline) << "ValueError: the comment string of " << stmt->GetTypeKey()
                            << " cannot have newline.";
      size_t start_pos = output_.tellp();
      output_ << "  # " << comment;
      size_t end_pos = output_.tellp();
      underlines_exempted_.push_back({start_pos, end_pos});
    }
  }

  void MaybePrintCommenMultiLines(const StmtDoc& stmt, bool new_line = false) {
    if (stmt->comment.defined()) {
      auto comment_lines = runtime::StringHelper::Split(stmt->comment.value().view(), "\n");
      bool first_line = true;
      size_t start_pos = output_.tellp();
      for (const auto& line_box : comment_lines) {
        auto line = line_box.As<runtime::string_view>();
        if (first_line) {
          output_ << "# " << line;
          first_line = false;
        } else {
          NewLine() << "# " << line;
        }
      }
      size_t end_pos = output_.tellp();
      underlines_exempted_.push_back({start_pos, end_pos});
      if (new_line) {
        NewLine();
      }
    }
  }

  void PrintDocString(const StringRef& comment) {
    size_t start_pos = output_.tellp();
    output_ << "\"\"\"";
    auto comment_lines = runtime::StringHelper::Split(comment.view(), "\n");
    for (const auto& line_box : comment_lines) {
      auto line = line_box.As<runtime::string_view>();
      if (line.empty()) {
        // No indentation on empty line
        output_ << "\n";
      } else {
        NewLine() << line;
      }
    }

    NewLine() << "\"\"\"";
    size_t end_pos = output_.tellp();
    underlines_exempted_.push_back({start_pos, end_pos});
  }

  void PrintBlockComment(const StringRef& comment) {
    IncreaseIndent();
    NewLine();
    PrintDocString(comment);
    DecreaseIndent();
  }
};

void PythonDocPrinter::PrintTypedDoc(const LiteralDoc& doc) {
  const ObjectRef& value = doc->value;
  if (!value.defined()) {
    output_ << "None";
  } else if (const auto* int_imm = value.as<IntImmNode>()) {
    if (int_imm->dtype.is_bool()) {
      output_ << (int_imm->value ? "True" : "False");
    } else {
      output_ << int_imm->value;
    }
  } else if (const auto* float_imm = value.as<FloatImmNode>()) {
    output_ << runtime::py_builtins::PyOS_double_to_string(
        float_imm->value, 'r', 0, runtime::py_builtins::Py_DTSF_ADD_DOT_0, NULL);
  } else if (const auto* string_obj = value.as<StringNode>()) {
    output_ << "\"" << runtime::BytesEscape(string_obj->data_container) << "\"";
  } else {
    MXLOG(FATAL) << "TypeError: Unsupported literal value type: " << value->GetTypeKey();
  }
}

void PythonDocPrinter::PrintTypedDoc(const IdDoc& doc) {
  output_ << doc->name;
}

void PythonDocPrinter::PrintTypedDoc(const AttrAccessDoc& doc) {
  PrintChildExpr(doc->value, doc);
  output_ << "." << doc->name;
}

void PythonDocPrinter::PrintTypedDoc(const IndexDoc& doc) {
  PrintChildExpr(doc->value, doc);
  if (doc->indices.size() == 0) {
    output_ << "[()]";
  } else {
    output_ << "[";
    PrintJoinedDocs(doc->indices, ", ");
    output_ << "]";
  }
}

const std::string OperatorToString(OperationDocNode::Kind operation_kind) {
  static const std::vector<std::string> op_kind2str = []() {
    using OpKind = OperationDocNode::Kind;
    std::map<OpKind, std::string> raw_table = {
        {OpKind::kUSub, "-"},        //
        {OpKind::kInvert, "~"},      //
        {OpKind::kNot, "not "},      //
        {OpKind::kAdd, "+"},         //
        {OpKind::kSub, "-"},         //
        {OpKind::kMult, "*"},        //
        {OpKind::kDiv, "/"},         //
        {OpKind::kFloorDiv, "//"},   //
        {OpKind::kMod, "%"},         //
        {OpKind::kPow, "**"},        //
        {OpKind::kLShift, "<<"},     //
        {OpKind::kRShift, ">>"},     //
        {OpKind::kBitAnd, "&"},      //
        {OpKind::kBitOr, "|"},       //
        {OpKind::kBitXor, "^"},      //
        {OpKind::kLt, "<"},          //
        {OpKind::kLtE, "<="},        //
        {OpKind::kEq, "=="},         //
        {OpKind::kNotEq, "!="},      //
        {OpKind::kGt, ">"},          //
        {OpKind::kGtE, ">="},        //
        {OpKind::kAnd, "and"},       //
        {OpKind::kOr, "or"},         //
        {OpKind::kIn, "in"},         //
        {OpKind::kNotIn, "not in"},  //
    };

    std::vector<std::string> table;
    table.resize(static_cast<int>(OperationDocNode::Kind::kSpecialEnd) + 1);

    for (const auto& kv : raw_table) {
      table[static_cast<int>(kv.first)] = kv.second;
    }

    return table;
  }();

  auto op_index = static_cast<int>(operation_kind);
  MXCHECK_LT(op_index, op_kind2str.size());
  const std::string str = op_kind2str[op_index];
  MXCHECK(!str.empty()) << "OperationDocNode::Kind " << static_cast<int>(operation_kind)
                        << " cannot be converted to operator token in Python directly.";
  return str;
}

void PythonDocPrinter::PrintTypedDoc(const OperationDoc& doc) {
  using OpKind = OperationDocNode::Kind;
  if (doc->kind < OpKind::kUnaryEnd) {
    // Unary Operators
    MXCHECK_EQ(doc->operands.size(), 1);
    output_ << OperatorToString(doc->kind);
    PrintChildExpr(doc->operands[0], doc);
  } else if (doc->kind == OpKind::kPow) {
    // Power operator is different than other binary operators
    // It's right-associative and binds less tightly than unary operator on its right.
    // https://docs.python.org/3/reference/expressions.html#the-power-operator
    // https://docs.python.org/3/reference/expressions.html#operator-precedence
    MXCHECK_EQ(doc->operands.size(), 2);
    PrintChildExprConservatively(doc->operands[0], doc);
    output_ << " ** ";
    PrintChildExpr(doc->operands[1], ExprPrecedence::kUnary);
  } else if (doc->kind < OpKind::kBinaryEnd) {
    // Binary Operator
    MXCHECK_EQ(doc->operands.size(), 2);
    PrintChildExpr(doc->operands[0], doc);
    output_ << " " << OperatorToString(doc->kind) << " ";
    PrintChildExprConservatively(doc->operands[1], doc);
  } else if (doc->kind == OpKind::kIfThenElse) {
    MXCHECK_EQ(doc->operands.size(), 3)
        << "ValueError: IfThenElse requires 3 operands, but got " << doc->operands.size();
    PrintChildExpr(doc->operands[1], doc);
    output_ << " if ";
    PrintChildExprConservatively(doc->operands[0], doc);
    output_ << " else ";
    PrintChildExprConservatively(doc->operands[2], doc);
  } else {
    MXLOG(FATAL) << "Unknown OperationDocNode::Kind " << static_cast<int>(doc->kind);
    throw;
  }
}

void PythonDocPrinter::PrintTypedDoc(const CallDoc& doc) {
  PrintChildExpr(doc->callee, doc);

  output_ << "(";

  // Print positional args
  bool is_first = true;
  for (const ExprDoc& arg : doc->args) {
    if (is_first) {
      is_first = false;
    } else {
      output_ << ", ";
    }
    PrintDoc(arg);
  }

  // Print keyword args
  MXCHECK_EQ(doc->kwargs_keys.size(), doc->kwargs_values.size())
      << "CallDoc should have equal number of elements in kwargs_keys and kwargs_values.";
  for (size_t i = 0; i < doc->kwargs_keys.size(); i++) {
    if (is_first) {
      is_first = false;
    } else {
      output_ << ", ";
    }
    StringRef keyword = doc->kwargs_keys[i];
    output_ << keyword;
    output_ << "=";
    PrintDoc(doc->kwargs_values[i]);
  }

  output_ << ")";
}

void PythonDocPrinter::PrintTypedDoc(const LambdaDoc& doc) {
  output_ << "lambda ";
  PrintJoinedDocs(doc->args, ", ");
  output_ << ": ";
  PrintChildExpr(doc->body, doc);
}

void PythonDocPrinter::PrintTypedDoc(const ListDoc& doc) {
  output_ << "[";
  PrintJoinedDocs(doc->elements, ", ");
  output_ << "]";
}

void PythonDocPrinter::PrintTypedDoc(const SetDoc& doc) {
  output_ << "{";
  PrintJoinedDocs(doc->elements, ", ");
  output_ << "}";
}

void PythonDocPrinter::PrintTypedDoc(const TupleDoc& doc) {
  output_ << "(";
  if (doc->elements.size() == 1) {
    PrintDoc(doc->elements[0]);
    output_ << ",";
  } else {
    PrintJoinedDocs(doc->elements, ", ");
  }
  output_ << ")";
}

void PythonDocPrinter::PrintTypedDoc(const DictDoc& doc) {
  MXCHECK_EQ(doc->keys.size(), doc->values.size())
      << "DictDoc should have equal number of elements in keys and values.";
  output_ << "{";
  size_t idx = 0;
  for (const ExprDoc& key : doc->keys) {
    if (idx > 0) {
      output_ << ", ";
    }
    PrintDoc(key);
    output_ << ": ";
    PrintDoc(doc->values[idx]);
    idx++;
  }
  output_ << "}";
}

void PythonDocPrinter::PrintTypedDoc(const ComprehensionDoc& doc) {
  output_ << "for ";
  PrintDoc(doc->target);
  output_ << " in ";
  PrintDoc(doc->iter);
  if (doc->ifs.defined()) {
    auto ifs = doc->ifs.value();
    if (ifs.size()) {
      output_ << " if ";
      PrintJoinedDocs(doc->ifs.value(), " ");
    }
  }
}

void PythonDocPrinter::PrintTypedDoc(const ListCompDoc& doc) {
  output_ << "[";
  PrintDoc(doc->elt);
  output_ << " ";
  PrintJoinedDocs(doc->generators, " ");
  output_ << "]";
}

void PythonDocPrinter::PrintTypedDoc(const SetCompDoc& doc) {
  output_ << "{";
  PrintDoc(doc->elt);
  output_ << " ";
  PrintJoinedDocs(doc->generators, " ");
  output_ << "}";
}

void PythonDocPrinter::PrintTypedDoc(const DictCompDoc& doc) {
  output_ << "{";
  PrintDoc(doc->key);
  output_ << ": ";
  PrintDoc(doc->value);
  output_ << " ";
  PrintJoinedDocs(doc->generators, " ");
  output_ << "}";
}

void PythonDocPrinter::PrintTypedDoc(const SliceDoc& doc) {
  if (doc->start != nullptr) {
    PrintDoc(doc->start.value());
  }
  output_ << ":";
  if (doc->stop != nullptr) {
    PrintDoc(doc->stop.value());
  }
  if (doc->step != nullptr) {
    output_ << ":";
    PrintDoc(doc->step.value());
  }
}

void PythonDocPrinter::PrintTypedDoc(const YieldDoc& doc) {
  output_ << "yield";
  if (doc->value.defined()) {
    output_ << " ";
    PrintDoc(doc->value.value());
  }
}

void PythonDocPrinter::PrintTypedDoc(const StmtBlockDoc& doc) {
  for (const StmtDoc& stmt : doc->stmts) {
    PrintDoc(stmt);
    if (stmt != doc->stmts.back()) {
      NewLine();
    }
  }
}

void PythonDocPrinter::PrintTypedDoc(const AssignDoc& doc) {
  if (const auto* tuple_doc = doc->lhs.as<TupleDocNode>()) {
    PrintJoinedDocs(tuple_doc->elements, ", ");
  } else {
    PrintDoc(doc->lhs);
  }

  if (doc->annotation) {
    output_ << ": ";
    PrintDoc(doc->annotation.value());
  }
  if (doc->rhs) {
    output_ << " = ";
    if (const auto* tuple_doc = doc->rhs.as<TupleDocNode>()) {
      PrintJoinedDocs(tuple_doc->elements, ", ");
    } else {
      PrintDoc(doc->rhs.value());
    }
  }
  MaybePrintCommentInline(doc);
}

void PythonDocPrinter::PrintTypedDoc(const IfDoc& doc) {
  MaybePrintCommenMultiLines(doc, true);
  output_ << "if ";
  PrintDoc(doc->predicate);
  output_ << ":";

  PrintIndentedBlock(doc->then_branch);

  if (!doc->else_branch.empty()) {
    NewLine();
    output_ << "else:";
    PrintIndentedBlock(doc->else_branch);
  }
}

void PythonDocPrinter::PrintTypedDoc(const WhileDoc& doc) {
  MaybePrintCommenMultiLines(doc, true);
  output_ << "while ";
  PrintDoc(doc->predicate);
  output_ << ":";

  PrintIndentedBlock(doc->body);
}

void PythonDocPrinter::PrintTypedDoc(const ForDoc& doc) {
  MaybePrintCommenMultiLines(doc, true);
  output_ << "for ";
  if (const auto* tuple = doc->lhs.as<TupleDocNode>()) {
    if (tuple->elements.size() == 1) {
      PrintDoc(tuple->elements[0]);
      output_ << ",";
    } else {
      PrintJoinedDocs(tuple->elements, ", ");
    }
  } else {
    PrintDoc(doc->lhs);
  }
  output_ << " in ";
  PrintDoc(doc->rhs);
  output_ << ":";

  PrintIndentedBlock(doc->body);
}

void PythonDocPrinter::PrintTypedDoc(const ContinueDoc& doc) {
  MaybePrintCommenMultiLines(doc, true);
  output_ << "continue";
}

void PythonDocPrinter::PrintTypedDoc(const BreakDoc& doc) {
  MaybePrintCommenMultiLines(doc, true);
  output_ << "break";
}

void PythonDocPrinter::PrintTypedDoc(const ExceptionHandlerDoc& doc) {
  MaybePrintCommenMultiLines(doc, true);
  output_ << "except";
  if (doc->type.defined()) {
    output_ << " ";
    PrintDoc(doc->type.value());
    if (doc->name.defined()) {
      output_ << " as ";
      PrintDoc(doc->name.value());
    }
  }
  output_ << ":";
  PrintIndentedBlock(doc->body);
}

void PythonDocPrinter::PrintTypedDoc(const TryExceptDoc& doc) {
  MaybePrintCommenMultiLines(doc, true);
  output_ << "try:";
  PrintIndentedBlock(doc->body);

  if (doc->handlers.defined()) {
    auto handlers = doc->handlers.value();
    for (auto handler : handlers) {
      NewLine();
      PrintTypedDoc(handler);
    }
  }

  if (doc->orelse.defined()) {
    NewLine();
    auto orelse = doc->orelse.value();
    output_ << "orelse:";
    PrintIndentedBlock(orelse);
  }

  if (doc->finalbody.defined()) {
    NewLine();
    auto finalbody = doc->finalbody.value();
    output_ << "finally:";
    PrintIndentedBlock(finalbody);
  }
}

void PythonDocPrinter::PrintTypedDoc(const RaiseDoc& doc) {
  MaybePrintCommenMultiLines(doc, true);
  output_ << "raise";
  if (doc->exc.defined()) {
    output_ << " ";
    PrintDoc(doc->exc.value());
  }
  if (doc->cause.defined()) {
    output_ << " from ";
    PrintDoc(doc->cause.value());
  }
}

void PythonDocPrinter::PrintTypedDoc(const ScopeDoc& doc) {
  MaybePrintCommenMultiLines(doc, true);
  output_ << "with ";
  PrintDoc(doc->rhs);
  if (doc->lhs != nullptr) {
    output_ << " as ";
    PrintDoc(doc->lhs.value());
  }
  output_ << ":";

  PrintIndentedBlock(doc->body);
}

void PythonDocPrinter::PrintTypedDoc(const ExprStmtDoc& doc) {
  PrintDoc(doc->expr);
  MaybePrintCommentInline(doc);
}

void PythonDocPrinter::PrintTypedDoc(const AssertDoc& doc) {
  output_ << "assert ";
  PrintDoc(doc->test);
  if (doc->msg.defined()) {
    output_ << ", ";
    PrintDoc(doc->msg.value());
  }
  MaybePrintCommentInline(doc);
}

void PythonDocPrinter::PrintTypedDoc(const ReturnDoc& doc) {
  output_ << "return ";
  PrintDoc(doc->value);
  MaybePrintCommentInline(doc);
}

void PythonDocPrinter::PrintTypedDoc(const FunctionDoc& doc) {
  for (const AssignDoc& arg_doc : doc->args) {
    MXCHECK(arg_doc->comment == nullptr) << "Function arg cannot have comment attached to them.";
  }

  PrintDecorators(doc->decorators);

  output_ << "def ";
  PrintDoc(doc->name);

  output_ << "(";
  PrintJoinedDocs(doc->args, ", ");
  output_ << ")";

  if (doc->return_type.defined()) {
    output_ << " -> ";
    PrintDoc(doc->return_type.value());
  }

  output_ << ":";

  if (doc->comment.defined()) {
    PrintBlockComment(doc->comment.value());
  }
  PrintIndentedBlock(doc->body);
  NewLineWithoutIndent();
}

void PythonDocPrinter::PrintTypedDoc(const ClassDoc& doc) {
  PrintDecorators(doc->decorators);

  output_ << "class ";
  PrintDoc(doc->name);
  if (doc->base.defined()) {
    output_ << "(";
    PrintDoc(doc->base);
    output_ << ")";
  }
  output_ << ":";

  if (doc->comment.defined()) {
    PrintBlockComment(doc->comment.value());
  }
  PrintIndentedBlock(doc->body);
}

void PythonDocPrinter::PrintTypedDoc(const CommentDoc& doc) {
  if (doc->comment.defined()) {
    MaybePrintCommenMultiLines(doc, false);
  }
}

void PythonDocPrinter::PrintTypedDoc(const DocStringDoc& doc) {
  if (doc->comment.defined() && !doc->comment.value().empty()) {
    PrintDocString(doc->comment.value());
  }
}

void PythonDocPrinter::PrintTypedDoc(const ModuleDoc& doc) {
  // TODO: introduce ImportStmtDoc and ImportFromStmtDoc
  output_ << "import matx";
  if (doc->comment.defined()) {
    PrintBlockComment(doc->comment.value());
  }
  for (const StmtDoc& d : doc->body) {
    NewLine();
    NewLine();
    PrintDoc(d);
  }
  NewLine();
}

StringRef DocToPythonScript(Doc doc, const PrinterConfig& cfg) {
  if (cfg->num_context_lines < 0) {
    cfg->num_context_lines = std::numeric_limits<int32_t>::max();
  }
  PythonDocPrinter printer(cfg);
  printer.Append(doc, cfg);
  StringRef result_ref = printer.GetString();
  auto result = result_ref.view();
  int last_space = result.size();
  while (last_space > 0 && std::isspace(result[last_space - 1])) {
    last_space--;
  }
  return StringRef(result.substr(0, last_space));
}

MATXSCRIPT_REGISTER_GLOBAL("ir.printer.DocToPythonScript").set_body_typed(DocToPythonScript);

}  // namespace printer
}  // namespace ir
}  // namespace matxscript
