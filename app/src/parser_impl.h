/*******************************************************************************
*  (c) 2019 ZondaX GmbH
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
#pragma once

#include <zxtypes.h>
#include "parser_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include "parser_txdef.h"

extern parser_tx_t parser_tx_obj;

#define WIRE_TYPE_VARINT   0            // Zigzag is not supported
#define WIRE_TYPE_64BIT    1            // Not supported
#define WIRE_TYPE_LEN      2
#define WIRE_TYPE_32BIT    5            // Not supported

#define FIELD_NUM(x) ((x) >> 3u)
#define WIRE_TYPE(x) ((uint8_t)((x) & 0x7u))

parser_error_t parser_init(parser_context_t *ctx,
                           const uint8_t *buffer,
                           uint16_t bufferSize);

parser_error_t _readRawVarint(parser_context_t *ctx, uint64_t *value);

parser_error_t _readVarint(parser_context_t *ctx, uint64_t *value);

parser_error_t _readUInt32(parser_context_t *ctx, uint32_t *value);

parser_error_t _readUInt8(parser_context_t *ctx, uint8_t *value);

parser_error_t _readArray(parser_context_t *ctx, const uint8_t **arrayPtr, uint16_t *arrayLength);

parser_error_t parser_readPB_Metadata(parser_context_t *ctx, parser_metadata_t *metadata);

parser_error_t parser_readPB_Coin(parser_context_t *ctx, parser_coin_t *coin);

parser_error_t parser_readPB_Fees(parser_context_t *ctx, parser_fees_t *fees);

parser_error_t parser_readPB_SendMsg(parser_context_t *ctx, parser_sendmsg_t *sendmsg);

parser_error_t parser_readPB_VoteMsg(parser_context_t *ctx, parser_votemsg_t *votemsg);

parser_error_t parser_readPB_UpdateMultisigMsg(parser_context_t *ctx, parser_updatemultisigmsg_t *updatemultisigmsg);

parser_error_t parser_readPB_Participant(parser_context_t *ctx, parser_participant_t *participant);

parser_error_t parser_readPB_CreateProposalMsg(parser_context_t *ctx, parser_createproposalmsg_t *createProposal);

parser_error_t parser_readPB_UpdateElectorateMsg(parser_context_t *ctx, parser_updateelectorate_t *updateElectorate);

parser_error_t parser_readPB_Root(parser_context_t *ctx);

parser_error_t parser_readRoot(parser_context_t *ctx);

parser_error_t parser_Tx(parser_context_t *ctx);

bool_t parser_IsMainnet(const uint8_t *chainID, uint16_t chainIDLen);

const char *parser_getHRP(const uint8_t *chainID, uint16_t chainIDLen);

parser_error_t parser_getAddress(const uint8_t *chainID, uint16_t chainIDLen,
                                 char *addr, uint16_t addrLen,
                                 const uint8_t *ptr, uint16_t len);

parser_error_t parser_arrayToString(char *out, uint16_t outLen,
                                    const uint8_t *in, uint8_t inLen,
                                    uint8_t pageIdx, uint8_t *pageCount);

parser_error_t parser_formatAmount(char *out, uint16_t outLen, parser_coin_t *coin);
parser_error_t parser_formatAmountFriendly(char *out, uint16_t outLen, parser_coin_t *coin);

#ifdef __cplusplus
}
#endif
