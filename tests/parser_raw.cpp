#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-err58-cpp"
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
#include "hexutils.h"
#include <parser.h>
#include <parser_txdef.h>

using ::testing::TestWithParam;
using ::testing::Values;

TEST(Protobuf, RawVarint) {
    uint8_t buffer[] = {0x08, 0x96, 0x01};

    parser_context_t ctx;
    parser_error_t err;
    parser_init(&ctx, buffer, sizeof(buffer));
    uint64_t v;

    err = _readRawVarint(&ctx, &v);
    EXPECT_EQ(err, parser_ok);
    EXPECT_EQ(ctx.offset, 0);
    EXPECT_EQ(ctx.lastConsumed, 1);
    EXPECT_EQ(v, 8);

    err = _readRawVarint(&ctx, &v);
    EXPECT_EQ(err, parser_ok);
    EXPECT_EQ(ctx.offset, 0);
    EXPECT_EQ(ctx.lastConsumed, 3);
    EXPECT_EQ(v, 150);
}

TEST(Protobuf, FieldVarint_big) {
    uint8_t buffer[] = {0x08, 0xFF, 0xFF, 0xFF, 0xFF, 0x01};

    parser_context_t ctx;
    parser_error_t err;

    parser_init(&ctx, buffer, sizeof(buffer));
    uint64_t v;

    err = _readVarint(&ctx, &v);
    EXPECT_EQ(err, parser_ok);
    EXPECT_EQ(ctx.offset, 6);
    EXPECT_EQ(v, 536870911);
}

TEST(Protobuf, FieldVarint_TooBig) {
    uint8_t buffer[] = {0x08,
                        0xFF, 0xFF, 0xFF, 0xFF,
                        0xFF, 0xFF, 0xFF, 0xFF,
                        0xFF, 0xFF, 0xFF, 0xFF, 0x01};

    parser_context_t ctx;
    parser_error_t err;

    parser_init(&ctx, buffer, sizeof(buffer));
    uint64_t v;

    err = _readVarint(&ctx, &v);
    EXPECT_EQ(err, parser_value_out_of_range);
}

TEST(Protobuf, FieldVarint) {
    uint8_t buffer[] = {0x08, 0x96, 0x01};

    parser_context_t ctx;
    parser_error_t err;

    parser_init(&ctx, buffer, sizeof(buffer));
    uint64_t v;

    err = _readVarint(&ctx, &v);
    EXPECT_EQ(err, parser_ok);
    EXPECT_EQ(ctx.offset, 3);
    EXPECT_EQ(v, 150);
}

TEST(Protobuf, ReadUInt32Field) {
    uint8_t buffer[] = {0x08, 0x96, 0x01};

    parser_context_t ctx;
    parser_error_t err;

    parser_init(&ctx, buffer, sizeof(buffer));
    uint32_t v;

    err = _readUInt32(&ctx, &v);
    EXPECT_EQ(err, parser_ok);
    EXPECT_EQ(ctx.offset, 3);
    EXPECT_EQ(v, 150);
}

TEST(Protobuf, ReadUInt32Field_OutOfRange) {
    uint8_t buffer[] = {0x08, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x01};

    parser_context_t ctx;
    parser_error_t err;

    parser_init(&ctx, buffer, sizeof(buffer));
    uint32_t v;

    err = _readUInt32(&ctx, &v);
    EXPECT_EQ(err, parser_value_out_of_range);
}

TEST(Protobuf, String) {
    uint8_t buffer[] = {0x12, 0x07, 0x74, 0x65, 0x73, 0x74, 0x69, 0x6e, 0x67};
    parser_context_t ctx;
    parser_error_t err;

    parser_init(&ctx, buffer, sizeof(buffer));

    uint16_t stringLen = 0;
    const uint8_t *s;

    err = _readArray(&ctx, &s, &stringLen);
    EXPECT_EQ(err, parser_ok);
    EXPECT_EQ(ctx.offset, 9);
    EXPECT_EQ(stringLen, 7);
    EXPECT_EQ(*s, 0x74);
    EXPECT_EQ(*(s + 1), 0x65);
}

TEST(Protobuf, Nested) {
    uint8_t buffer[] = {0x1a, 0x03, 0x08, 0x96, 0x01};
    parser_context_t ctx;
    parser_error_t err;

    parser_init(&ctx, buffer, sizeof(buffer));

    uint16_t stringLen = 0;
    const uint8_t *s;

    err = _readArray(&ctx, &s, &stringLen);
    EXPECT_EQ(err, parser_ok);
    EXPECT_EQ(ctx.offset, 5);
    EXPECT_EQ(stringLen, 3);
}

TEST(Protobuf, Enumeration) {
    char pbtx[] = "0a020801121473f16e71d0878f6ad26531e174452aec9161e8d41a14000000000000000000000000000000000000000022061a0443415348";
    uint8_t buffer[200];
    uint16_t bufferSize = parseHexString(buffer, sizeof(buffer), pbtx);
    EXPECT_EQ(bufferSize, 56);

    parser_context_t ctx;
    parser_error_t err;
    parser_init(&ctx, buffer, bufferSize);

    uint64_t v;

    while (ctx.offset < ctx.bufferLen) {
        err = _readRawVarint(&ctx, &v);
        EXPECT_EQ(err, parser_ok);

        switch (WIRE_TYPE(v)) {
            case WIRE_TYPE_VARINT: {
                err = _readVarint(&ctx, &v);
                ASSERT_EQ(err, parser_ok);
            }
                break;
            case WIRE_TYPE_LEN: {
                uint16_t stringLen = 0;
                const uint8_t *s;

                err = _readArray(&ctx, &s, &stringLen);
                ASSERT_EQ(err, parser_ok);
            }
                break;
        }
    }
}

TEST(Protobuf, Header) {
    char pbtx[] = "00cafe00"                        // 4
                  "0b"                              // 1
                  "696f762d6c6f76656e6574"          // 11
                  "0000000000000000"                // 8
                  ///
                  "9a0338"                         // array 3 + 56 payload
                  "0a020801121473f16e71d0878f6ad26531e17445"
                  "2aec9161e8d41a14000000000000000000000000000000"
                  "000000000022061a0443415348";

    uint8_t buffer[200];
    uint16_t bufferSize = parseHexString(buffer, sizeof(buffer), pbtx);
    EXPECT_EQ(bufferSize, 83);

    parser_context_t ctx;
    parser_init(&ctx, buffer, bufferSize);

    parser_error_t err = parser_readRoot(&ctx);
    ASSERT_EQ(err, parser_ok) << parser_getErrorDescription(err);

    // Check results
    char chainID[bufferSize];
    ASSERT_EQ(parser_tx_obj.chainIDLen, 11);

    parser_arrayToString(chainID, bufferSize,
                         parser_tx_obj.chainID, parser_tx_obj.chainIDLen,
                         0, nullptr);

    ASSERT_STREQ(chainID, "iov-lovenet");

    ASSERT_EQ(parser_tx_obj.nonce, 0);
    ASSERT_EQ(ctx.offset, bufferSize);

    ASSERT_EQ(parser_tx_obj.feesLen, 0);
    ASSERT_EQ((uint64_t) parser_tx_obj.feesPtr, 0);

    ASSERT_EQ(parser_tx_obj.sendmsgLen, 56);
    ASSERT_EQ((ptrdiff_t) (parser_tx_obj.sendmsgPtr - buffer), 27);
}

TEST(Message, ChainID) {

    bool_t res;
    std::string chainIdPrefix = APP_MAINNET_CHAINID_PREFIX;

    std::string chainid_t1 = chainIdPrefix;
    std::string chainid_t2 = chainIdPrefix + "-3";
    std::string chainid_t3 = "some-net123" + chainIdPrefix;
    std::string chainid_t4 = "some-net123";
    std::string chainid_t5 = chainIdPrefix.substr(0, chainIdPrefix.size()-1);

    res = parser_IsMainnet(reinterpret_cast<const uint8_t *>(chainid_t1.c_str()), chainid_t1.length());
    ASSERT_EQ(res, bool_true);
    res = parser_IsMainnet(reinterpret_cast<const uint8_t *>(chainid_t2.c_str()), chainid_t2.length());
    ASSERT_EQ(res, bool_true);
    res = parser_IsMainnet(reinterpret_cast<const uint8_t *>(chainid_t3.c_str()), chainid_t3.length());
    ASSERT_EQ(res, bool_false);
    res = parser_IsMainnet(reinterpret_cast<const uint8_t *>(chainid_t4.c_str()), chainid_t4.length());
    ASSERT_EQ(res, bool_false);
    res = parser_IsMainnet(reinterpret_cast<const uint8_t *>(chainid_t5.c_str()), chainid_t5.length());
    ASSERT_EQ(res, bool_false);
}

TEST(Protobuf, SendMsg) {
    char pbtx[] = "0a020801"
                  "121473f16e71d0878f6ad26531e174452aec9161e8d4"
                  "1a14000000000000000000000000000000000000000022061a0443415348";

    uint8_t buffer[200];
    uint16_t bufferSize = parseHexString(buffer, sizeof(buffer), pbtx);
    EXPECT_EQ(bufferSize, 56);
    parser_context_t ctx {buffer, bufferSize, 0, 0};

    parser_sendmsg_t sendmsg;
    parser_sendmsgInit(&sendmsg);
    parser_error_t err = parser_readPB_SendMsg(&ctx, &sendmsg);
    ASSERT_EQ(err, parser_ok) << parser_getErrorDescription(err);

    ASSERT_EQ(sendmsg.metadataLen, 2);
    ASSERT_EQ(sendmsg.sourceLen, 20);
    ASSERT_EQ(sendmsg.destinationLen, 20);
    ASSERT_EQ(sendmsg.amountLen, 6);
    ASSERT_EQ(sendmsg.memoLen, 0);
    ASSERT_EQ(sendmsg.refLen, 0);

    char tmp[bufferSize];

    ASSERT_EQ(sendmsg.metadata.schema, 1);

    err = parser_getAddress((const uint8_t *) "x", 1,
                            tmp, bufferSize, sendmsg.sourcePtr, sendmsg.sourceLen);
    ASSERT_EQ(err, parser_ok) << parser_getErrorDescription(err);
    ASSERT_STREQ(tmp, "tiov1w0ckuuwss78k45n9x8shg3f2ajgkr6x5xdlje2");

    err = parser_getAddress((const uint8_t *) "x", 1,
                            tmp, bufferSize, sendmsg.destinationPtr, sendmsg.destinationLen);
    ASSERT_EQ(err, parser_ok) << parser_getErrorDescription(err);
    ASSERT_STREQ(tmp, "tiov1qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqzx8n0d");

    ASSERT_EQ(sendmsg.amount.whole, 0);
    ASSERT_EQ(sendmsg.amount.fractional, 0);
    ASSERT_EQ(sendmsg.amount.tickerLen, 4);

    parser_arrayToString(tmp, bufferSize,
                         sendmsg.amount.tickerPtr, sendmsg.amount.tickerLen,
                         0, nullptr);
    ASSERT_EQ(err, parser_ok) << parser_getErrorDescription(err);
    ASSERT_STREQ(tmp, "CASH");
}

TEST(Tx, Init) {
    parser_tx_t tx;

    parser_txInit(&tx);

    EXPECT_EQ(tx.version, nullptr);
    EXPECT_EQ(tx.chainIDLen, 0);
    EXPECT_EQ(tx.chainID, nullptr);
    EXPECT_EQ(tx.nonce, 0);

    EXPECT_EQ(tx.seen.fees, false);
    EXPECT_EQ(tx.seen.tx_message, false);

    //////

    EXPECT_EQ(tx.feesPtr, nullptr);
    EXPECT_EQ(tx.feesLen, 0);
    EXPECT_EQ(tx.fees.seen.payer, false);
    EXPECT_EQ(tx.fees.seen.coin, false);
    EXPECT_EQ(tx.fees.payerPtr, nullptr);
    EXPECT_EQ(tx.fees.payerLen, 0);
    EXPECT_EQ(tx.fees.coinPtr, nullptr);
    EXPECT_EQ(tx.fees.coinLen, 0);

    EXPECT_EQ(tx.fees.coin.seen.whole, false);
    EXPECT_EQ(tx.fees.coin.seen.fractional, false);
    EXPECT_EQ(tx.fees.coin.seen.ticker, false);
    EXPECT_EQ(tx.fees.coin.whole, 0);
    EXPECT_EQ(tx.fees.coin.fractional, 0);
    EXPECT_EQ(tx.fees.coin.tickerPtr, nullptr);
    EXPECT_EQ(tx.fees.coin.tickerLen, 0);

    //////

    EXPECT_EQ(tx.multisigPtr, nullptr);
    EXPECT_EQ(tx.multisigLen, 0);
    EXPECT_EQ(tx.multisig.count, 0);

    //////

    EXPECT_EQ(tx.sendmsgPtr, nullptr);
    EXPECT_EQ(tx.sendmsgLen, 0);

    EXPECT_EQ(tx.sendmsg.seen.metadata, false);
    EXPECT_EQ(tx.sendmsg.seen.source, false);
    EXPECT_EQ(tx.sendmsg.seen.destination, false);
    EXPECT_EQ(tx.sendmsg.seen.amount, false);
    EXPECT_EQ(tx.sendmsg.seen.memo, false);
    EXPECT_EQ(tx.sendmsg.seen.ref, false);

    EXPECT_EQ(tx.sendmsg.metadataPtr, nullptr);
    EXPECT_EQ(tx.sendmsg.metadataLen, 0);
    EXPECT_EQ(tx.sendmsg.metadata.seen.schema, false);
    EXPECT_EQ(tx.sendmsg.metadata.schema, 0);

    EXPECT_EQ(tx.sendmsg.sourcePtr, nullptr);
    EXPECT_EQ(tx.sendmsg.sourceLen, 0);

    EXPECT_EQ(tx.sendmsg.destinationPtr, nullptr);
    EXPECT_EQ(tx.sendmsg.destinationLen, 0);

    EXPECT_EQ(tx.sendmsg.amountPtr, nullptr);
    EXPECT_EQ(tx.sendmsg.amountLen, 0);
    EXPECT_EQ(tx.sendmsg.amount.seen.whole, false);
    EXPECT_EQ(tx.sendmsg.amount.seen.fractional, false);
    EXPECT_EQ(tx.sendmsg.amount.seen.ticker, false);
    EXPECT_EQ(tx.sendmsg.amount.whole, 0);
    EXPECT_EQ(tx.sendmsg.amount.fractional, 0);
    EXPECT_EQ(tx.sendmsg.amount.tickerPtr, nullptr);
    EXPECT_EQ(tx.sendmsg.amount.tickerLen, 0);

    EXPECT_EQ(tx.sendmsg.memoPtr, nullptr);
    EXPECT_EQ(tx.sendmsg.memoLen, 0);

    EXPECT_EQ(tx.sendmsg.refPtr, nullptr);
    EXPECT_EQ(tx.sendmsg.refLen, 0);

}

#pragma clang diagnostic pop
