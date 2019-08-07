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
#include <hexutils.h>
#include <nlohmann/json.hpp>
#include "lib/parser_impl.h"

using ::testing::TestWithParam;
using ::testing::Values;
using json = nlohmann::json;

TEST(Protobuf, RawVarint) {
    uint8_t buffer[] = {0x08, 0x96, 0x01};

    parser_context_t ctx;
    parser_init(&ctx, buffer, sizeof(buffer));
    uint64_t v = readRawVarint(&ctx);
    EXPECT_EQ(ctx.lastError, parser_ok);
    EXPECT_EQ(ctx.offset, 0);
    EXPECT_EQ(ctx.lastConsumed, 1);
    EXPECT_EQ(v, 8);

    v = readRawVarint(&ctx);
    EXPECT_EQ(ctx.lastError, parser_ok);
    EXPECT_EQ(ctx.offset, 0);
    EXPECT_EQ(ctx.lastConsumed, 3);
    EXPECT_EQ(v, 150);
}

TEST(Protobuf, FieldVarint) {
    uint8_t buffer[] = {0x08, 0x96, 0x01};

    parser_context_t ctx;
    parser_init(&ctx, buffer, sizeof(buffer));
    uint64_t v = readVarint(&ctx);
    EXPECT_EQ(ctx.lastError, parser_ok);
    EXPECT_EQ(ctx.offset, 3);
    EXPECT_EQ(v, 150);
}

TEST(Protobuf, String) {
    uint8_t buffer[] = {0x12, 0x07, 0x74, 0x65, 0x73, 0x74, 0x69, 0x6e, 0x67};
    parser_context_t ctx;
    parser_init(&ctx, buffer, sizeof(buffer));

    uint64_t stringLen = 0;
    auto s = readString(&ctx, &stringLen);
    EXPECT_EQ(ctx.lastError, parser_ok);
    EXPECT_EQ(ctx.offset, 9);
    EXPECT_EQ(stringLen, 7);
    EXPECT_EQ(*s, 0x74);
    EXPECT_EQ(*(s + 1), 0x65);
}

TEST(Protobuf, Nested) {
    uint8_t buffer[] = {0x1a, 0x03, 0x08, 0x96, 0x01};
    parser_context_t ctx;
    parser_init(&ctx, buffer, sizeof(buffer));

    uint64_t stringLen = 0;
    auto s = readString(&ctx, &stringLen);
    EXPECT_EQ(ctx.lastError, parser_ok);
    EXPECT_EQ(ctx.offset, 5);
    EXPECT_EQ(stringLen, 3);
}

TEST(Protobuf, Enumeration) {
    char pbtx[] = "0a020801121473f16e71d0878f6ad26531e174452aec9161e8d41a"
                  "14000000000000000000000000000000000000000022061a0443415348";
    uint8_t buffer[200];
    uint16_t bufferSize = parseHexString(pbtx, buffer);
    EXPECT_EQ(bufferSize, 56);

    parser_context_t ctx;
    parser_init(&ctx, buffer, bufferSize);

    std::cout << std::endl;
    while (ctx.offset < ctx.bufferSize) {
        uint64_t v = readRawVarint(&ctx);
        EXPECT_EQ(ctx.lastError, parser_ok);
        std::cout << (int) FIELD_NUM(v) << " [" << (int) WIRE_TYPE(v) << "]: ";
        std::cout << (int) ctx.offset  << std::endl;

        switch(WIRE_TYPE(v)) {
            case WIRE_TYPE_VARINT: {
                uint64_t v = readVarint(&ctx);
                ASSERT_EQ(ctx.lastError, parser_ok);
            }
                break;
            case WIRE_TYPE_LEN: {
                uint64_t stringLen = 0;
                auto s = readString(&ctx, &stringLen);
                ASSERT_EQ(ctx.lastError, parser_ok);
            }
                break;
        }
    }
}
