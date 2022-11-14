// Copyright 2022 ByteDance Ltd. and/or its affiliates.
/*
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
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <matxscript/pipeline/tx_session.h>

using namespace ::matxscript::runtime;

int main(int argc, char* argv[]) {
  // test case
  std::unordered_map<std::string, RTValue> feed_dict;
  feed_dict.emplace("texts", List{Unicode(U"hello"), Unicode(U"world"), Unicode(U"unknown")});
  std::vector<std::pair<std::string, RTValue>> result;
  const char* module_path = argv[1];
  const char* module_name = "model.spec.json";
  {
    auto sess = TXSession::Load(module_path, module_name);
    auto result = sess->Run(feed_dict);
    for (auto& r : result) {
      std::cout << "result: " << r.second << std::endl;
    }
  }
  return 0;
}
