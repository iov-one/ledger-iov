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

#include <gmock/gmock.h>
#include "utils/testcases.h"
#include "utils/common.h"

using ::testing::TestWithParam;
using ::testing::Values;


TEST(Utils, CanReadTestcaseCorrectly) {
    auto testcases = GetJsonTestCases("testvectors/sendtx_single.json");
    EXPECT_EQ(testcases.size(), 1);

    auto testcase = ReadTestCaseData(testcases[0].testcases, 0);

    EXPECT_EQ(testcase.bytes,
              "00cafe000b696f762d6d61696e6e657400000000000000000a231214bad055e2cbcffc633e7dc76dc1148d6e9a2debfd1a0b1080c2d72f1a04434153489a03450a0208011214bad055e2cbcffc633e7dc76dc1148d6e9a2debfd1a140000000000000000000000000000000000000000220808011a04434153482a09736f6d652074657874");

    EXPECT_EQ(testcase.nonce, 0);
    EXPECT_EQ(testcase.transaction.amount.quantity, "1000000000");
}

TEST(Utils, CanReadTestcaseMultisigCorrectly) {
    auto testcases = GetJsonTestCases("testvectors/sendtx_singleMultisig.json");
    EXPECT_EQ(testcases.size(), 1);

    auto testcase = ReadTestCaseData(testcases[0].testcases, 0);

    EXPECT_EQ(testcase.bytes,
              "00cafe000b696f762d6c6f76656e657400000000000000070a2312145ae2c58796b0ad48ffe7602eac3353488c859a2b1a0b1080c2d72f1a0443415348220800000000000000012208000000000000007b220800000000000001c722080000000000000b3d9a03c2010a0208011214abababab111222111222111222ccccccccdddddd1a140000000000000000000000000000000000000000220d08081080d293ad031a03494f562a8001412076657279206c6f6e67206d656d6f206c6f72656d20697073756d206c6f72656d20697073756d2e20412076657279206c6f6e67206d656d6f206c6f72656d20697073756d206c6f72656d20697073756d2e20412076657279206c6f6e67206d656d6f206c6f72656d20697073756d206c6f72656d20697073756d21213131");

    EXPECT_EQ(testcase.nonce, 7);
    EXPECT_EQ(testcase.transaction.amount.quantity, "8900000000");
}

//void checkJsonTx(json &j, uint64_t index) {
//    auto tx = j[index]["transaction"];
//    char tmpBuf[100];
//
//    parser_arrayToString(tmpBuf, 100,
//                         parser_tx_obj.chainID, parser_tx_obj.chainIDLen,
//                         0, nullptr);
//    EXPECT_EQ(tx["creator"]["chainId"], "string:" + std::string(tmpBuf));
//
//    parser_getAddress(parser_tx_obj.chainID, parser_tx_obj.chainIDLen, tmpBuf, 100,
//                      parser_tx_obj.sendmsg.sourcePtr, parser_tx_obj.sendmsg.sourceLen);
//    EXPECT_EQ(tx["sender"], "string:" + std::string(tmpBuf));
//
//    parser_getAddress(parser_tx_obj.chainID, parser_tx_obj.chainIDLen, tmpBuf, 100,
//                      parser_tx_obj.sendmsg.destinationPtr, parser_tx_obj.sendmsg.destinationLen);
//    EXPECT_EQ(tx["recipient"], "string:" + std::string(tmpBuf));
//
//    //// Check amount
//    parser_formatAmount(tmpBuf, 100, &parser_tx_obj.sendmsg.amount);
//    EXPECT_EQ(tx["amount"]["quantity"], "string:" + std::string(tmpBuf));
//
//    parser_arrayToString(tmpBuf, 100,
//                         parser_tx_obj.sendmsg.amount.tickerPtr, parser_tx_obj.sendmsg.amount.tickerLen,
//                         0, nullptr);
//    EXPECT_EQ(tx["amount"]["tokenTicker"], "string:" + std::string(tmpBuf));
//
//    //// Check fees
//    parser_formatAmount(tmpBuf, 100, &parser_tx_obj.fees.coin);
//    if (tx["fee"]["tokens"]["quantity"].is_null() == false) {
//        EXPECT_EQ(tx["fee"]["tokens"]["quantity"], "string:" + std::string(tmpBuf));
//    } else {
//        EXPECT_EQ("string:0", "string:" + std::string(tmpBuf));
//    }
//
//    parser_arrayToString(tmpBuf, 100,
//                         parser_tx_obj.fees.coin.tickerPtr, parser_tx_obj.fees.coin.tickerLen,
//                         0, nullptr);
//
//    if (tx["fee"]["tokens"]["tokenTicker"].is_null() == false) {
//        EXPECT_EQ(tx["fee"]["tokens"]["tokenTicker"], "string:" + std::string(tmpBuf));
//    } else {
//        EXPECT_EQ("string:", "string:" + std::string(tmpBuf));
//    }
//
//
//    EXPECT_EQ(j[index]["nonce"], parser_tx_obj.nonce);
//}
//
//TEST(TestCases, SingleJson) {
//    std::ifstream inFile("testvectors/sendtx_single.json");
//    ASSERT_TRUE(inFile.is_open()) << "Check that your working directory is pointing to the 'tests' directory";
//
//    json j;
//    inFile >> j;
//    uint8_t buffer[10000];
//    std::string s = j[0]["bytes"];
//    uint16_t bufferSize = parseHexString(s.c_str(), buffer);
//
//    parser_context_t ctx;
//    parser_error_t err = parser_parse(&ctx, buffer, bufferSize);
//    ASSERT_EQ(err, parser_ok) << parser_getErrorDescription(err);
//
//    checkJsonTx(j, 0);
//}
//
//class JsonTests : public ::testing::TestWithParam<int> {
//public:
//    static json j;
//
//    static void SetUpTestCase() {
//        std::ifstream inFile("testvectors/sendtx_tests.json");
//        ASSERT_TRUE(inFile.is_open()) << "Check that your working directory is pointing to the 'tests' directory";
//        inFile >> j;
//
//        std::cout << "Number of testcases: " << j.size() << std::endl;
//    }
//
//    struct PrintToStringParamName {
//        std::string operator()(const ::testing::TestParamInfo<int> &p) const {
//            std::stringstream ss;
//            ss << "TestCase" << p.param;
//            return ss.str();
//        }
//    };
//};
//
//json JsonTests::j;
//
//INSTANTIATE_TEST_CASE_P(JsonTestCases, JsonTests, ::testing::Range(0, 432));
//
//TEST_P(JsonTests, CheckParser) {
//    uint8_t buffer[200];
//    size_t i = GetParam();
//
//    std::string s = j[i]["bytes"];
//    uint16_t bufferSize = parseHexString(s.c_str(), buffer);
//
//    parser_context_t ctx;
//    parser_error_t err = parser_parse(&ctx, buffer, bufferSize);
//    ASSERT_EQ(err, parser_ok) << parser_getErrorDescription(err);
//
//    checkJsonTx(j, i);
//}
//
//
//class JsonMultisigTests : public ::testing::TestWithParam<int> {
//public:
//    static json j;
//
//    static void SetUpTestCase() {
//        std::ifstream inFile("testvectors/sendtx_multisig_tests.json");
//        ASSERT_TRUE(inFile.is_open()) << "Check that your working directory is pointing to the 'tests' directory";
//        inFile >> j;
//
//        std::cout << "Number of testcases: " << j.size() << std::endl;
//    }
//
//    struct PrintToStringParamName {
//        std::string operator()(const ::testing::TestParamInfo<int> &p) const {
//            std::stringstream ss;
//            ss << "TestCase" << p.param;
//            return ss.str();
//        }
//    };
//};
//
//json JsonMultisigTests::j;
//
//INSTANTIATE_TEST_CASE_P(JsonMultisigTests, JsonMultisigTests, ::testing::Range(0, 6));
//
//TEST_P(JsonMultisigTests, CheckParser) {
//    uint8_t buffer[10000];
//    size_t i = GetParam();
//
//    std::string s = j[i]["bytes"];
//    uint16_t bufferSize = parseHexString(s.c_str(), buffer);
//
//    parser_context_t ctx;
//    parser_error_t err = parser_parse(&ctx, buffer, bufferSize);
//    ASSERT_EQ(err, parser_ok) << parser_getErrorDescription(err);
//
//    checkJsonTx(j, i);
//}
