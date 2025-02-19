// Copyright 2022 ByteDance Ltd. and/or its affiliates.
/*
 * Acknowledgement: This file originates from incubator-tvm
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

/*!
 * \file attrs.cc
 */
#include <matxscript/ir/attrs.h>

#include <matxscript/ir/attr_functor.h>
#include <matxscript/ir/printer/doc.h>
#include <matxscript/ir/printer/ir_docsifier.h>
#include <matxscript/ir/printer/ir_frame.h>
#include <matxscript/ir/printer/utils.h>
#include <matxscript/runtime/registry.h>

namespace matxscript {
namespace ir {

using namespace ::matxscript::runtime;
using namespace ::matxscript::ir::printer;

void DictAttrsNode::VisitAttrs(AttrVisitor* v) {
  v->Visit("__dict__", &dict);
}

void DictAttrsNode::VisitNonDefaultAttrs(AttrVisitor* v) {
  v->Visit("__dict__", &dict);
}

Array<AttrFieldInfo> DictAttrsNode::ListFieldInfo() const {
  return {};
}

DictAttrs::DictAttrs(Map<StringRef, ObjectRef> dict) {
  ObjectPtr<DictAttrsNode> n = make_object<DictAttrsNode>();
  n->dict = std::move(dict);
  data_ = std::move(n);
}

MATXSCRIPT_STATIC_IR_FUNCTOR(IRDocsifier, vtable)
    .set_dispatch<DictAttrs>("", [](DictAttrs attrs, ObjectPath p, IRDocsifier d) -> Doc {
      return d->AsDoc(attrs->dict, p->Attr("dict"));
    });

MATXSCRIPT_REGISTER_NODE_TYPE(DictAttrsNode);

MATXSCRIPT_REGISTER_NODE_TYPE(AttrFieldInfoNode);

MATXSCRIPT_REGISTER_GLOBAL("ir.DictAttrsGetDict").set_body_typed([](DictAttrs attrs) {
  return attrs->dict;
});

MATXSCRIPT_REGISTER_GLOBAL("ir.AttrsListFieldInfo").set_body_typed([](Attrs attrs) {
  return attrs->ListFieldInfo();
});

}  // namespace ir
}  // namespace matxscript
