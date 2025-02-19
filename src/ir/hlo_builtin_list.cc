// Copyright 2022 ByteDance Ltd. and/or its affiliates.
/*
 * Acknowledgement: The structure of the expressions is inspired by TVM.
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
#include <matxscript/ir/hlo_builtin.h>
#include "./hlo_builtin_macros.h"

namespace matxscript {
namespace ir {
namespace builtin {

/******************************************************************************
 * List builtin methods
 *****************************************************************************/

MATXSCRIPT_IR_DEFINE_HLO_METHOD(list, __len__, size)
    .set_num_inputs(1)
    .add_argument("self", "List", "");

MATXSCRIPT_IR_DEFINE_HLO_METHOD(ft_list, __len__, size)
    .set_num_inputs(1)
    .add_argument("self", "FTList", "");

MATXSCRIPT_IR_DEFINE_HLO_METHOD(list, __contains__, contains)
    .set_num_inputs(2)
    .add_argument("self", "List", "")
    .add_argument("item", "<template>", "");

MATXSCRIPT_IR_DEFINE_HLO_METHOD(ft_list, __contains__, contains)
    .set_num_inputs(2)
    .add_argument("self", "FTList", "")
    .add_argument("item", "<template>", "");

MATXSCRIPT_IR_DEFINE_HLO_METHOD(list, __getitem__, get_item)
    .set_num_inputs(2)
    .add_argument("self", "List", "")
    .add_argument("i", "int", "");

MATXSCRIPT_IR_DEFINE_HLO_METHOD(ft_list, __getitem__, get_item)
    .set_num_inputs(2)
    .add_argument("self", "FTList", "")
    .add_argument("i", "int", "");

MATXSCRIPT_IR_DEFINE_HLO_METHOD(list, __setitem__, set_item)
    .set_num_inputs(3)
    .add_argument("self", "List", "")
    .add_argument("i", "int", "")
    .add_argument("item", "Any", "");

MATXSCRIPT_IR_DEFINE_HLO_METHOD(ft_list, __setitem__, set_item)
    .set_num_inputs(3)
    .add_argument("self", "FTList", "")
    .add_argument("i", "int", "")
    .add_argument("item", "<template>", "");

MATXSCRIPT_IR_DEFINE_HLO_METHOD(list, __getslice__, get_slice)
    .set_num_inputs(3)
    .set_num_inputs_max(4)
    .add_argument("self", "List", "")
    .add_argument("b", "int", "")
    .add_argument("e", "int", "")
    .add_argument("step", "int", "");

MATXSCRIPT_IR_DEFINE_HLO_METHOD(ft_list, __getslice__, get_slice)
    .set_num_inputs(3)
    .set_num_inputs_max(4)
    .add_argument("self", "FTList", "")
    .add_argument("b", "int", "")
    .add_argument("e", "int", "")
    .add_argument("step", "int", "");

MATXSCRIPT_IR_DEFINE_HLO_METHOD(list, append, append)
    .set_num_inputs(2)
    .add_argument("self", "List", "")
    .add_argument("item", "Any", "");

MATXSCRIPT_IR_DEFINE_HLO_METHOD(ft_list, append, append)
    .set_num_inputs(2)
    .add_argument("self", "FTList", "")
    .add_argument("item", "<template>", "");

MATXSCRIPT_IR_DEFINE_HLO_METHOD(list, extend, extend)
    .set_num_inputs(2)
    .add_argument("self", "List", "")
    .add_argument("items", "list|iter|Any|any_view", "");

MATXSCRIPT_IR_DEFINE_HLO_METHOD(ft_list, extend, extend)
    .set_num_inputs(2)
    .add_argument("self", "FTList", "")
    .add_argument("items", "FTList|Any|any_view", "");

MATXSCRIPT_IR_DEFINE_HLO_METHOD(list, repeat, repeat)
    .set_num_inputs(2)
    .add_argument("self", "List", "")
    .add_argument("times", "int", "");

MATXSCRIPT_IR_DEFINE_HLO_METHOD(ft_list, repeat, repeat)
    .set_num_inputs(2)
    .add_argument("self", "FTList", "")
    .add_argument("times", "int", "");

MATXSCRIPT_IR_DEFINE_HLO_MODULE_FUNC(list, fused_repeat_one)
    .set_num_inputs(2)
    .add_argument("value", "any_view|Any", "")
    .add_argument("times", "int", "");

MATXSCRIPT_IR_DEFINE_HLO_MODULE_FUNC(ft_list, fused_repeat_one)
    .set_num_inputs(2)
    .add_argument("value", "<template>", "")
    .add_argument("times", "int", "");

MATXSCRIPT_IR_DEFINE_HLO_MODULE_FUNC(list, fused_repeat_many)
    .set_num_inputs(2)
    .add_argument("value", "List", "")
    .add_argument("times", "int", "");

MATXSCRIPT_IR_DEFINE_HLO_MODULE_FUNC(ft_list, fused_repeat_many)
    .set_num_inputs(2)
    .add_argument("value", "<template>", "")
    .add_argument("times", "int", "");

MATXSCRIPT_IR_DEFINE_HLO_METHOD(list, reserve, reserve)
    .set_num_inputs(2)
    .add_argument("self", "List", "")
    .add_argument("new_size", "int", "");

MATXSCRIPT_IR_DEFINE_HLO_METHOD(ft_list, reserve, reserve)
    .set_num_inputs(2)
    .add_argument("self", "FTList", "")
    .add_argument("new_size", "int", "");

MATXSCRIPT_IR_DEFINE_HLO_METHOD(list, index, index)
    .set_num_inputs(2)
    .set_num_inputs_max(4)
    .add_argument("self", "List", "")
    .add_argument("x", "Any", "")
    .add_argument("start", "int", "")
    .add_argument("end", "int", "");

MATXSCRIPT_IR_DEFINE_HLO_METHOD(ft_list, index, index)
    .set_num_inputs(2)
    .set_num_inputs_max(4)
    .add_argument("self", "FTList", "")
    .add_argument("x", "<template>", "")
    .add_argument("start", "int", "")
    .add_argument("end", "int", "");

MATXSCRIPT_IR_DEFINE_HLO_METHOD(list, capacity, capacity)
    .set_num_inputs(1)
    .add_argument("self", "List", "");

MATXSCRIPT_IR_DEFINE_HLO_METHOD(ft_list, capacity, capacity)
    .set_num_inputs(1)
    .add_argument("self", "FTList", "");

MATXSCRIPT_IR_DEFINE_HLO_METHOD(list, pop, pop)
    .set_num_inputs(1)
    .set_num_inputs_max(2)
    .add_argument("self", "List", "")
    .add_argument("index", "int", "");

MATXSCRIPT_IR_DEFINE_HLO_METHOD(ft_list, pop, pop)
    .set_num_inputs(1)
    .set_num_inputs_max(2)
    .add_argument("self", "FTList", "")
    .add_argument("index", "int", "");

MATXSCRIPT_IR_DEFINE_HLO_METHOD(list, insert, insert)
    .set_num_inputs(3)
    .set_num_inputs_max(3)
    .add_argument("self", "List", "")
    .add_argument("index", "int", "")
    .add_argument("item", "Any|any_view", "");

MATXSCRIPT_IR_DEFINE_HLO_METHOD(ft_list, insert, insert)
    .set_num_inputs(3)
    .set_num_inputs_max(3)
    .add_argument("self", "List|FTList", "")
    .add_argument("index", "int", "")
    .add_argument("item", "<template>", "");

MATXSCRIPT_IR_DEFINE_HLO_METHOD(list, remove, remove)
    .set_num_inputs(2)
    .add_argument("self", "List", "")
    .add_argument("item", "Any|any_view", "");

MATXSCRIPT_IR_DEFINE_HLO_METHOD(ft_list, remove, remove)
    .set_num_inputs(2)
    .add_argument("self", "List|FTList", "")
    .add_argument("item", "<template>", "");

MATXSCRIPT_IR_DEFINE_HLO_METHOD(list, clear, clear)
    .set_num_inputs(1)
    .add_argument("self", "List", "");

MATXSCRIPT_IR_DEFINE_HLO_METHOD(ft_list, clear, clear)
    .set_num_inputs(1)
    .add_argument("self", "FTList", "");

MATXSCRIPT_IR_DEFINE_HLO_METHOD(list, reverse, reverse)
    .set_num_inputs(1)
    .add_argument("self", "List", "");

MATXSCRIPT_IR_DEFINE_HLO_METHOD(ft_list, reverse, reverse)
    .set_num_inputs(1)
    .add_argument("self", "FTList", "");

MATXSCRIPT_IR_DEFINE_HLO_METHOD(list, count, count)
    .set_num_inputs(2)
    .add_argument("self", "List", "")
    .add_argument("item", "<template>", "");

MATXSCRIPT_IR_DEFINE_HLO_METHOD(ft_list, count, count)
    .set_num_inputs(2)
    .add_argument("self", "FTList", "")
    .add_argument("item", "<template>", "");

MATXSCRIPT_IR_DEFINE_HLO_METHOD(list, sort_no_key, sort)
    .set_num_inputs(1)
    .set_num_inputs_max(2)
    .add_argument("self", "List", "")
    .add_argument("reverse", "bool", "");

MATXSCRIPT_IR_DEFINE_HLO_METHOD(list, sort, sort)
    .set_num_inputs(1)
    .set_num_inputs_max(3)
    .add_argument("self", "List", "")
    .add_argument("key", "any_view|Any", "")
    .add_argument("reverse", "bool", "");

MATXSCRIPT_IR_DEFINE_HLO_METHOD(ft_list, sort_no_key, sort)
    .set_num_inputs(1)
    .set_num_inputs_max(2)
    .add_argument("self", "FTList", "")
    .add_argument("reverse", "bool", "");

MATXSCRIPT_IR_DEFINE_HLO_METHOD(ft_list, sort, sort)
    .set_num_inputs(1)
    .set_num_inputs_max(3)
    .add_argument("self", "FTList", "")
    .add_argument("key", "any_view|Any", "")
    .add_argument("reverse", "bool", "");

/******************************************************************************
 * List custom functions
 *****************************************************************************/

MATXSCRIPT_IR_DEFINE_HLO_MODULE_FUNC(list, module_sort)
    .set_num_inputs(1)
    .add_argument("args", "*args", "");

MATXSCRIPT_IR_DEFINE_HLO_MODULE_FUNC(list, module_nth_element)
    .set_num_inputs(1)
    .add_argument("args", "*args", "");

MATXSCRIPT_IR_DEFINE_HLO_MODULE_FUNC(list, module_heapify)
    .set_num_inputs(1)
    .add_argument("args", "*args", "");

MATXSCRIPT_IR_DEFINE_HLO_MODULE_FUNC(list, module_heap_replace)
    .set_num_inputs(1)
    .add_argument("args", "*args", "");

MATXSCRIPT_IR_DEFINE_HLO_MODULE_FUNC(list, module_heap_pushpop)
    .set_num_inputs(1)
    .add_argument("args", "*args", "");

}  // namespace builtin
}  // namespace ir
}  // namespace matxscript
