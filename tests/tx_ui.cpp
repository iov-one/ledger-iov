/*******************************************************************************
*   (c) 2019 ZondaX GmbH
*
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS,
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
********************************************************************************/

#include "gmock/gmock.h"

#include <iostream>
#include <nlohmann/json.hpp>
#include <fstream>
#include <hexutils.h>
#include <lib/parser_impl.h>
#include <lib/parser.h>

using json = nlohmann::json;

void dumpUI() {
    uint8_t numFields = parser_getNumItems(nullptr);

#define KEY_LEN 60
#define VALUE_LEN 60
    char key[KEY_LEN];
    char value[VALUE_LEN];

    std::stringstream ss;

    for (int idx = 0; idx < numFields; idx++) {
        uint8_t pageCount;
        parser_error_t err = parser_getItem(nullptr, idx,
                                            key, KEY_LEN,
                                            value, VALUE_LEN,
                                            0, &pageCount);

        if (err != parser_ok) {
            ss << "ERR " << parser_getErrorDescription(err) << std::endl;
            break;
        }

        ss << idx << "| ";
        ss << key << " ";
        ss << value << " ";
        ss << std::endl;
    }

    std::cout << ss.str() << std::endl;
}

TEST(UI, SingleJson) {
    std::ifstream inFile("tests/testvectors/sendtx_single.json");
    ASSERT_TRUE(inFile.is_open()) << "Check that your working directory is pointing to the test directory";

    json j;
    inFile >> j;
    uint8_t buffer[200];
    std::string s = j[0]["bytes"];
    uint16_t bufferSize = parseHexString(s.c_str(), buffer);

    parser_context_t ctx;
    parser_error_t err = parser_parse(&ctx, buffer, bufferSize);
    ASSERT_EQ(err, parser_ok) << parser_getErrorDescription(err);

    std::cout << std::endl;
    dumpUI();
}
