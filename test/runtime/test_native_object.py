# Copyright 2022 ByteDance Ltd. and/or its affiliates.
#
# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.
import unittest
import matx


class TestRuntimeNativeObject(unittest.TestCase):

    def setUp(self) -> None:
        pass

    def test_native_object(self):
        echo_stub = matx.make_native_object("EchoServiceExample", )
        req = matx.make_native_object("MySimpleNativeDataExample")
        rsp = matx.make_native_object("MySimpleNativeDataExample")
        ret = echo_stub.echo(req, rsp)
        self.assertEqual(rsp.get_content(), b"[Response] hello")
        return ret


if __name__ == "__main__":
    import logging

    logging.basicConfig(level=logging.INFO)
    unittest.main()
