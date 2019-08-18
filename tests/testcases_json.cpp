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
#include <lib/iov.h>

using ::testing::TestWithParam;
using ::testing::Values;
using json = nlohmann::json;

// Testcases: https://gist.github.com/webmaster128/5304649b21dc080cadd9e6484d07b1d2

TEST(TestCases, Json) {
    std::ifstream inFile("testvectors/sendtx_single.json");
    ASSERT_TRUE(inFile.is_open()) << "Check that your working directory is pointing to the test directory";

    json j;
    inFile >> j;
    std::cout << std::endl;
    std::cout << std::setw(4) << j << std::endl;

    EXPECT_EQ(j.size(), 1);
    EXPECT_EQ(j[0].size(), 3);

    EXPECT_EQ(j[0]["bytes"].size(), 1);
    EXPECT_EQ(j[0]["bytes"],
              "00cafe000b696f762d6c6f76656e657400000000000000009a03380a020801121473f16e71d0878f6ad26531e174452aec9161e8d41a14000000000000000000000000000000000000000022061a0443415348");

    EXPECT_EQ(j[0]["nonce"].size(), 1);
    EXPECT_EQ(j[0]["nonce"], 0);

    EXPECT_EQ(j[0]["transaction"].size(), 5);
    EXPECT_EQ(j[0]["transaction"]["amount"].size(), 3);
    EXPECT_EQ(j[0]["transaction"]["amount"]["quantity"], "string:0");
}

void checkJsonTx(json &j, uint64_t index) {
    auto tx = j[index]["transaction"];
    char tmpBuf[100];

    parser_arrayToString(tmpBuf, 100,
                         parser_tx_obj.chainID, parser_tx_obj.chainIDLen,
                         0, nullptr);
    EXPECT_EQ(tx["creator"]["chainId"], "string:" + std::string(tmpBuf));

    parser_getAddress(parser_tx_obj.chainID, parser_tx_obj.chainIDLen, tmpBuf, 100,
                      parser_tx_obj.sendmsg.sourcePtr, parser_tx_obj.sendmsg.sourceLen);
    EXPECT_EQ(tx["sender"], "string:" + std::string(tmpBuf));

    parser_getAddress(parser_tx_obj.chainID, parser_tx_obj.chainIDLen, tmpBuf, 100,
                      parser_tx_obj.sendmsg.destinationPtr, parser_tx_obj.sendmsg.destinationLen);
    EXPECT_EQ(tx["recipient"], "string:" + std::string(tmpBuf));

    //// Check amount
    parser_formatAmount(tmpBuf, 100, &parser_tx_obj.sendmsg.amount);
    EXPECT_EQ(tx["amount"]["quantity"], "string:" + std::string(tmpBuf));

    parser_arrayToString(tmpBuf, 100,
                         parser_tx_obj.sendmsg.amount.tickerPtr, parser_tx_obj.sendmsg.amount.tickerLen,
                         0, nullptr);
    EXPECT_EQ(tx["amount"]["tokenTicker"], "string:" + std::string(tmpBuf));

    //// Check fees
    parser_formatAmount(tmpBuf, 100, &parser_tx_obj.fees.coin);
    if (tx["fee"]["tokens"]["quantity"].is_null() == false) {
        EXPECT_EQ(tx["fee"]["tokens"]["quantity"], "string:" + std::string(tmpBuf));
    } else {
        EXPECT_EQ("string:0", "string:" + std::string(tmpBuf));
    }

    parser_arrayToString(tmpBuf, 100,
                         parser_tx_obj.fees.coin.tickerPtr, parser_tx_obj.fees.coin.tickerLen,
                         0, nullptr);

    if (tx["fee"]["tokens"]["tokenTicker"].is_null() == false) {
        EXPECT_EQ(tx["fee"]["tokens"]["tokenTicker"], "string:" + std::string(tmpBuf));
    } else {
        EXPECT_EQ("string:", "string:" + std::string(tmpBuf));
    }


    EXPECT_EQ(j[index]["nonce"], parser_tx_obj.nonce);
}

TEST(TestCases, SingleJson) {
    std::ifstream inFile("testvectors/sendtx_single.json");
    ASSERT_TRUE(inFile.is_open()) << "Check that your working directory is pointing to the test directory";

    json j;
    inFile >> j;
    uint8_t buffer[200];
    std::string s = j[0]["bytes"];
    uint16_t bufferSize = parseHexString(s.c_str(), buffer);

    parser_context_t ctx;
    parser_error_t err = parser_parse(&ctx, buffer, bufferSize);
    ASSERT_EQ(err, parser_ok) << parser_getErrorDescription(err);

    checkJsonTx(j, 0);
}

class JsonTests : public ::testing::TestWithParam<int> {
public:
    static json j;

    static void SetUpTestCase() {
        std::ifstream inFile("testvectors/sendtx_tests.json");
        ASSERT_TRUE(inFile.is_open()) << "Check that your working directory is pointing to the test directory";
        inFile >> j;

        std::cout << "Number of testcases: " << j.size() << std::endl;
    }

    struct PrintToStringParamName {
        std::string operator()(const ::testing::TestParamInfo<int> &p) const {
            std::stringstream ss;
            ss << "TestCase" << p.param;
            return ss.str();
        }
    };
};

json JsonTests::j;

INSTANTIATE_TEST_CASE_P

(JsonTestCases, JsonTests, ::testing::Range(0, 432));

TEST_P(JsonTests, CheckParser) {
    uint8_t buffer[200];
    size_t i = GetParam();

    std::string s = j[i]["bytes"];
    uint16_t bufferSize = parseHexString(s.c_str(), buffer);

    std::cout << std::setw(4) << j[i] << std::endl;

    parser_context_t ctx;
    parser_error_t err = parser_parse(&ctx, buffer, bufferSize);
    ASSERT_EQ(err, parser_ok) << parser_getErrorDescription(err);

    checkJsonTx(j, i);
}
