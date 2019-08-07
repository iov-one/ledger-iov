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

using ::testing::TestWithParam;
using ::testing::Values;
using json = nlohmann::json;

// Testcases: https://gist.github.com/webmaster128/5304649b21dc080cadd9e6484d07b1d2

TEST(TestCases, Json) {
    std::ifstream inFile("testvectors/sendtx_single.json");
    json j;
    inFile >> j;
    std::cout << std::endl;
    std::cout << std::setw(4) << j << std::endl;

    EXPECT_EQ(j.size(), 1);
    EXPECT_EQ(j[0].size(), 3);

    EXPECT_EQ(j[0]["bytes"].size(), 1);
    EXPECT_EQ(j[0]["nonce"].size(), 1);
    EXPECT_EQ(j[0]["transaction"].size(), 5);

    EXPECT_EQ(j[0]["bytes"].get<std::string>(),
        std::string("00cafe000b696f762d6c6f76656e657400000000000000009a03380a02080"
                    "1121473f16e71d0878f6ad26531e174452aec9161e8d41a1400000000000000000"
                    "0000000000000000000000022061a0443415348"));
}
