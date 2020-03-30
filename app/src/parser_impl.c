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

#include <zxmacros.h>
#include "parser_impl.h"
#include "parser_txdef.h"
#include <bech32.h>
#include "coin.h"

parser_tx_t parser_tx_obj;

#define CHECK_NOT_DUPLICATED(FIELD) if (FIELD) return parser_duplicated_field; else FIELD = 1;

#define WITH_CONTEXT(PTR, LEN, CALL) { \
    parser_context_t __tmpctx; \
    parser_error_t __err = parser_init_context(&__tmpctx, PTR, LEN); \
    if ( __err != parser_no_data) FAIL_ON_ERROR(CALL) }

#define READ_NONNEGATIVE_INT64(FIELD) FAIL_ON_ERROR(_readNonNegativeInt64(ctx, &FIELD))
#define READ_UINT32(FIELD) FAIL_ON_ERROR(_readUInt32(ctx, &FIELD))
#define READ_UINT8(FIELD) FAIL_ON_ERROR(_readUInt8(ctx, &FIELD))
#define READ_ARRAY(FIELD) FAIL_ON_ERROR(_readArray(ctx, &FIELD##Ptr, &FIELD##Len))

parser_error_t parser_init_context(parser_context_t *ctx, const uint8_t *buffer, uint16_t bufferSize) {
    ctx->offset = 0;
    ctx->lastConsumed = 0;

    if (bufferSize == 0 || buffer == NULL) {
        // Not available, use defaults
        ctx->buffer = NULL;
        ctx->bufferLen = 0;
        return parser_no_data;
    }

    ctx->buffer = buffer;
    ctx->bufferLen = bufferSize;

    return parser_ok;
}

parser_error_t parser_init(parser_context_t *ctx, const uint8_t *buffer, uint16_t bufferSize) {
    FAIL_ON_ERROR(parser_init_context(ctx, buffer, bufferSize))

    parser_txInit(&parser_tx_obj);

    return parser_ok;
}

const char *parser_getErrorDescription(parser_error_t err) {
    switch (err) {
        // General errors
        case parser_ok:
            return "No error";
        case parser_no_data:
            return "No more data";
        case parser_unexpected_buffer_end:
            return "Unexpected buffer end";
        case parser_unexpected_wire_type:
            return "Unexpected wire type";
        case parser_unexpected_version:
            return "Unexpected version";
        case parser_unexpected_characters:
            return "Unexpected characters";
        case parser_unexpected_field:
            return "Unexpected field";
        case parser_duplicated_field:
            return "Unexpected duplicated field";
        case parser_unexpected_chain:
            return "Unexpected chain";
        case parser_unexpected_field_length:
            return "Unexpected field length";
        default:
            return "Unrecognized error code";
    }
}

parser_error_t _readRawVarint(parser_context_t *ctx, uint64_t *value) {
    uint16_t offset = ctx->offset + ctx->lastConsumed;
    uint16_t consumed = 0;

    const uint8_t *p = ctx->buffer + offset;
    const uint8_t *end = ctx->buffer + ctx->bufferLen + 1;
    *value = 0;

    // Extract value
    uint16_t shift = 0;
    while (p < end && shift < 64) {
        const uint64_t tmp = ((*p) & 0x7Fu);

        if (shift == 63 && tmp > 1) {
            return parser_value_out_of_range;
        }

        *value += tmp << shift;

        consumed++;
        if (!(*p & 0x80u)) {
            ctx->lastConsumed += consumed;
            return parser_ok;
        }
        shift += 7;
        p++;
    }

    return parser_unexpected_buffer_end;
}

parser_error_t _readVarint(parser_context_t *ctx, uint64_t *value) {
    ctx->lastConsumed = 0;

    parser_error_t err = _readRawVarint(ctx, value);
    if (err != parser_ok) {
        ctx->lastConsumed = 0;
        return err;
    }

    if (WIRE_TYPE(*value) != WIRE_TYPE_VARINT) {
        ctx->lastConsumed = 0;
        return parser_unexpected_wire_type;
    }

    err = _readRawVarint(ctx, value);

    if (err == parser_ok) {
        ctx->offset += ctx->lastConsumed;
        ctx->lastConsumed = 0;
    }

    return err;
}

parser_error_t _readNonNegativeInt64(parser_context_t *ctx, int64_t *value) {
    ctx->lastConsumed = 0;

    uint64_t tmpValue;
    FAIL_ON_ERROR(_readVarint(ctx, &tmpValue))

    *value = *((int64_t *) &tmpValue);

    if (*value < 0) {
        return parser_value_out_of_range;
    }

    return parser_ok;
}

parser_error_t _readUInt32(parser_context_t *ctx, uint32_t *value) {
    ctx->lastConsumed = 0;

    uint64_t tmpValue;
    FAIL_ON_ERROR( _readVarint(ctx, &tmpValue))

    if (tmpValue >= (uint64_t) UINT32_MAX) {
        return parser_value_out_of_range;
    }

    *value = (uint32_t) tmpValue;
    return parser_ok;
}

parser_error_t _readUInt8(parser_context_t *ctx, uint8_t *value) {
    ctx->lastConsumed = 0;

    uint64_t tmpValue;
    FAIL_ON_ERROR( _readVarint(ctx, &tmpValue))

    if (tmpValue >= (uint64_t) UINT8_MAX) {
        return parser_value_out_of_range;
    }

    *value = (uint8_t) tmpValue;
    return parser_ok;
}

parser_error_t _readArray(parser_context_t *ctx, const uint8_t **arrayPtr, uint16_t *arrayLength) {
    ctx->lastConsumed = 0;

    // First retrieve array type and confirm
    uint64_t v;
    parser_error_t err = _readRawVarint(ctx, &v);
    if (err != parser_ok) {
        ctx->lastConsumed = 0;
        return 0;
    }
    if (WIRE_TYPE(v) != WIRE_TYPE_LEN) {
        ctx->lastConsumed = 0;
        return parser_unexpected_wire_type;
    }

    // Now get number of bytes
    uint64_t tmpValue;
    err = _readRawVarint(ctx, &tmpValue);
    if (tmpValue >= (uint64_t) UINT16_MAX) {
        err = parser_value_out_of_range;
    }
    if (err != parser_ok) {
        ctx->lastConsumed = 0;
        return err;
    }
    *arrayLength = tmpValue;

    // check that the returned buffer is not out of bounds
    if (ctx->offset + ctx->lastConsumed + *arrayLength > ctx->bufferLen) {
        ctx->lastConsumed = 0;
        return parser_unexpected_buffer_end;
    }

    // Set a pointer to the start of the first element
    *arrayPtr = ctx->buffer + ctx->offset + ctx->lastConsumed;
    ctx->lastConsumed += *arrayLength;

    ctx->offset += ctx->lastConsumed;
    ctx->lastConsumed = 0;
    return parser_ok;
}

parser_error_t _checkValidReadableChars(const uint8_t *p, uint16_t len) {
    for (uint16_t i = 0; i < len; i++) {
        uint8_t tmp = *(p + i);
        if (tmp < 33 || tmp > 127) {
            return parser_unexpected_characters;
        }
    }
    return parser_ok;
}

parser_error_t _checkUppercaseLetters(const uint8_t *p, uint16_t len) {
    for (uint16_t i = 0; i < len; i++) {
        uint8_t tmp = *(p + i);
        if (tmp < 'A' || tmp > 'Z') {
            return parser_unexpected_characters;
        }
    }
    return parser_ok;
}

parser_error_t _checkChainIDValid(const uint8_t *p, uint16_t len) {
    for (uint16_t i = 0; i < len; i++) {
        uint8_t tmp = *(p + i);

//        [a-zA-Z0-9_.-]
        const uint8_t valid = (tmp >= 'a' && tmp <= 'z') ||
                              (tmp >= 'A' && tmp <= 'Z') ||
                              (tmp >= '0' && tmp <= '9') ||
                              tmp == '_' || tmp == '.' || tmp == '-';

        if (!valid) {
            return parser_unexpected_characters;
        }
    }
    return parser_ok;
}

parser_error_t parser_readPB_Metadata(parser_context_t *ctx, parser_metadata_t *metadata) {

    uint64_t v;
    while (ctx->offset < ctx->bufferLen) {
        FAIL_ON_ERROR( _readRawVarint(ctx, &v))

        switch (FIELD_NUM(v)) {
            case PBIDX_METADATA_SCHEMA: {
                CHECK_NOT_DUPLICATED(metadata->seen.schema)
                READ_UINT32(metadata->schema)
                break;
            }
            default:
                // Unknown fields are rejected to avoid malleability
                return parser_unexpected_field;
        }
    }

    return parser_ok;
}

parser_error_t parser_readPB_Coin(parser_context_t *ctx, parser_coin_t *coin) {
    parser_error_t err;
    uint64_t v;

    while (ctx->offset < ctx->bufferLen) {
        FAIL_ON_ERROR( _readRawVarint(ctx, &v))

        switch (FIELD_NUM(v)) {
            case PBIDX_COIN_WHOLE: {
                CHECK_NOT_DUPLICATED(coin->seen.whole)
                READ_NONNEGATIVE_INT64(coin->whole)
                break;
            }
            case PBIDX_COIN_FRACTIONAL: {
                CHECK_NOT_DUPLICATED(coin->seen.fractional)
                READ_NONNEGATIVE_INT64(coin->fractional)
                break;
            }
            case PBIDX_COIN_TICKER: {
                CHECK_NOT_DUPLICATED(coin->seen.ticker)
                READ_ARRAY(coin->ticker)
                break;
            }
            default:
                // Unknown fields are rejected to avoid malleability
                return parser_unexpected_field;
        }
    }

    // Validation
    if (coin->whole < 0)
        return parser_value_out_of_range;
    if (coin->fractional < 0)
        return parser_value_out_of_range;
    if (coin->tickerLen < 3 || coin->tickerLen > 4)
        return parser_value_out_of_range;

    FAIL_ON_ERROR(_checkUppercaseLetters(coin->tickerPtr, coin->tickerLen))

    return parser_ok;
}

parser_error_t parser_readPB_Fees(parser_context_t *ctx, parser_fees_t *fees) {
    uint64_t v;
    while (ctx->offset < ctx->bufferLen) {
        FAIL_ON_ERROR( _readRawVarint(ctx, &v))

        switch (FIELD_NUM(v)) {
            case PBIDX_FEES_PAYER: {
                CHECK_NOT_DUPLICATED(fees->seen.payer)
                READ_ARRAY(fees->payer)
                break;
            }
            case PBIDX_FEES_COIN: {
                CHECK_NOT_DUPLICATED(fees->seen.coin)
                READ_ARRAY(fees->coin)
                break;
            }
            default:
                // Unknown fields are rejected to avoid malleability
                return parser_unexpected_field;
        }
    }

    WITH_CONTEXT(fees->coinPtr, fees->coinLen,
                 parser_readPB_Coin(&__tmpctx, &fees->coin))

    return parser_ok;
}

parser_error_t parser_readPB_Multisig(parser_context_t *ctx, parser_multisig_t *m) {
    union {
        uint64_t v;
        uint8_t bytes[8];
    } data;

    if (m->count >= PBIDX_MULTISIG_COUNT_MAX) {
        return parser_value_out_of_range;
    }

    const uint8_t *p;
    uint16_t pLen;
    FAIL_ON_ERROR( _readArray(ctx, &p, &pLen))

    if (pLen != 8) {
        return parser_unexpected_field_length;
    }

    MEMCPY(data.bytes, p, 8);
    m->values[m->count] = uint64_from_BEarray(data.bytes);
    m->count++;

    return parser_ok;
}

parser_error_t parser_readPB_SendMsg(parser_context_t *ctx, parser_sendmsg_t *sendmsg) {

    uint64_t v;
    while (ctx->offset < ctx->bufferLen) {
        FAIL_ON_ERROR( _readRawVarint(ctx, &v))

        switch (FIELD_NUM(v)) {
            case PBIDX_SENDMSG_METADATA: {
                CHECK_NOT_DUPLICATED(sendmsg->seen.metadata)
                READ_ARRAY(sendmsg->metadata)
                break;
            }
            case PBIDX_SENDMSG_SOURCE: {
                CHECK_NOT_DUPLICATED(sendmsg->seen.source)
                READ_ARRAY(sendmsg->source)
                break;
            }
            case PBIDX_SENDMSG_DESTINATION: {
                CHECK_NOT_DUPLICATED(sendmsg->seen.destination)
                READ_ARRAY(sendmsg->destination)
                break;
            }
            case PBIDX_SENDMSG_AMOUNT: {
                CHECK_NOT_DUPLICATED(sendmsg->seen.amount)
                READ_ARRAY(sendmsg->amount)
                break;
            }
            case PBIDX_SENDMSG_MEMO: {
                CHECK_NOT_DUPLICATED(sendmsg->seen.memo)
                READ_ARRAY(sendmsg->memo)
                break;
            }
                // NOTE: Disabling this field, it should not appear in any transaction
//            case PBIDX_SENDMSG_REF: {
//                CHECK_NOT_DUPLICATED(sendmsg->seen.ref);
//                READ_ARRAY(sendmsg->ref);
//                break;
//            }
            default:
                // Unknown fields are rejected to avoid malleability
                return parser_unexpected_field;
        }
    }

    WITH_CONTEXT(sendmsg->metadataPtr, sendmsg->metadataLen,
                 parser_readPB_Metadata(&__tmpctx, &sendmsg->metadata))

    WITH_CONTEXT(sendmsg->amountPtr, sendmsg->amountLen,
                 parser_readPB_Coin(&__tmpctx, &sendmsg->amount))

    return parser_ok;
}

parser_error_t parser_readPB_VoteMsg(parser_context_t *ctx, parser_votemsg_t *votemsg) {

    uint64_t v;
    while (ctx->offset < ctx->bufferLen) {
        FAIL_ON_ERROR( _readRawVarint(ctx, &v))

        switch (FIELD_NUM(v)) {
            case PBIDX_VOTEMSG_METADATA: {
                CHECK_NOT_DUPLICATED(votemsg->seen.metadata)
                READ_ARRAY(votemsg->metadata)
                break;
            }
            case PBIDX_VOTEMSG_PROPOSAL_ID: {
                CHECK_NOT_DUPLICATED(votemsg->seen.id)
                READ_ARRAY(votemsg->proposalId)
                break;
            }
            case PBIDX_VOTEMSG_VOTER: {
                CHECK_NOT_DUPLICATED(votemsg->seen.voter)
                READ_ARRAY(votemsg->voter)
                break;
            }
            case PBIDX_VOTEMSG_VOTE: {
                CHECK_NOT_DUPLICATED(votemsg->seen.voteOption)
                READ_UINT8(votemsg->voteOption)
                break;
            }
            default:
                // Unknown fields are rejected to avoid malleability
                return parser_unexpected_field;
        }
    }

    WITH_CONTEXT(votemsg->metadataPtr, votemsg->metadataLen,
                 parser_readPB_Metadata(&__tmpctx, &votemsg->metadata))

    return parser_ok;
}

parser_error_t parser_readPB_UpdateMultisigMsg(parser_context_t *ctx, parser_updatemultisigmsg_t *updatemultisigmsg) {

    uint64_t v;
    while (ctx->offset < ctx->bufferLen) {
        FAIL_ON_ERROR(_readRawVarint(ctx, &v))

        switch (FIELD_NUM(v)) {
            case PBIDX_UPDATEMSG_METADATA: {
                CHECK_NOT_DUPLICATED(updatemultisigmsg->seen.metadata)
                READ_ARRAY(updatemultisigmsg->metadata)
                break;
            }
            case PBIDX_UPDATEMSG_ID: {
                CHECK_NOT_DUPLICATED(updatemultisigmsg->seen.id)
                READ_ARRAY(updatemultisigmsg->contractId)
                break;
            }
            case PBIDX_UPDATEMSG_PARTICIPANTS: {
                if (updatemultisigmsg->participantsCount >= PBIDX_UPDATEMSG_PARTICIPANTS_MAX){
                    return parser_unexpected_number_items;
                }

                parser_context_t local_ctx;
                FAIL_ON_ERROR(_readArray(ctx, &local_ctx.buffer, &local_ctx.bufferLen))
                local_ctx.offset = 0;
                local_ctx.lastConsumed = 0;

                FAIL_ON_ERROR(parser_readPB_Participant(
                        &local_ctx,
                        &updatemultisigmsg->participant_array[updatemultisigmsg->participantsCount]))

                updatemultisigmsg->participantsCount++;
                break;
            }
            case PBIDX_UPDATEMSG_ACTIVATION_TH: {
                CHECK_NOT_DUPLICATED(updatemultisigmsg->seen.activation_th)
                READ_UINT32(updatemultisigmsg->activation_th)
                break;
            }
            case PBIDX_UPDATEMSG_ADMIN_TH: {
                CHECK_NOT_DUPLICATED(updatemultisigmsg->seen.admin_th)
                READ_UINT32(updatemultisigmsg->admin_th)
                break;
            }
            default:
                // Unknown fields are rejected to avoid malleability
                return parser_unexpected_field;
        }
    }

    WITH_CONTEXT(updatemultisigmsg->metadataPtr, updatemultisigmsg->metadataLen,
                 parser_readPB_Metadata(&__tmpctx, &updatemultisigmsg->metadata))

    return parser_ok;
}

parser_error_t parser_readPB_Participant(parser_context_t *ctx, parser_participant_t *participant) {

    uint64_t v;
    while (ctx->offset < ctx->bufferLen) {
        FAIL_ON_ERROR( _readRawVarint(ctx, &v))

        switch (FIELD_NUM(v)) {
            case PBIDX_PARTICIPANTMSG_SIGNATURE: {
                CHECK_NOT_DUPLICATED(participant->seen.signature)
                READ_ARRAY(participant->signature)
                break;
            }
            case PBIDX_PARTICIPANTMSG_WEIGHT: {
                CHECK_NOT_DUPLICATED(participant->seen.weight)
                READ_UINT32(participant->weight)
                break;
            }
            default:
                // Unknown fields are rejected to avoid malleability
                return parser_unexpected_field;
        }
    }

    return parser_ok;
}

parser_error_t parser_readPB_CreateProposalMsg(parser_context_t *ctx, parser_createproposalmsg_t *createProposal) {
    uint64_t v;
    while (ctx->offset < ctx->bufferLen) {
        FAIL_ON_ERROR( _readRawVarint(ctx, &v))

        switch (FIELD_NUM(v)) {
            case PBIDX_CREATEPROPOSALMSG_METADATA: {
                CHECK_NOT_DUPLICATED(createProposal->seen.metadata)
                READ_ARRAY(createProposal->metadata)
                break;
            }
            case PBIDX_CREATEPROPOSALMSG_TITLE: {
                CHECK_NOT_DUPLICATED(createProposal->seen.title)
                READ_ARRAY(createProposal->title)
                break;
            }
            case PBIDX_CREATEPROPOSALMSG_OPTION: {
                CHECK_NOT_DUPLICATED(createProposal->seen.raw_option)
                READ_ARRAY(createProposal->rawOption)
                break;
            }
            case PBIDX_CREATEPROPOSALMSG_DESCRIPTION: {
                CHECK_NOT_DUPLICATED(createProposal->seen.description)
                READ_ARRAY(createProposal->description)
                break;
            }
            case PBIDX_CREATEPROPOSALMSG_RULEID: {
                CHECK_NOT_DUPLICATED(createProposal->seen.election_rule_id)
                READ_ARRAY(createProposal->electionRuleId)
                break;
            }
            case PBIDX_CREATEPROPOSALMSG_STARTTIME: {
                CHECK_NOT_DUPLICATED(createProposal->seen.start_time)
                READ_NONNEGATIVE_INT64(createProposal->startTime)
                break;
            }
            case PBIDX_CREATEPROPOSALMSG_AUTHOR: {
                CHECK_NOT_DUPLICATED(createProposal->seen.author)
                READ_ARRAY(createProposal->author)
                break;
            }
            default:
                // Unknown fields are rejected to avoid malleability
                return parser_unexpected_field;
        }
    }

    WITH_CONTEXT(createProposal->metadataPtr, createProposal->metadataLen,
                 parser_readPB_Metadata(&__tmpctx, &createProposal->metadata))

    WITH_CONTEXT(createProposal->rawOptionPtr, createProposal->rawOptionLen,
                 _readArray(&__tmpctx, &createProposal->updateelectoratemsgPtr,
                                             &createProposal->updateelectoratemsgLen))

    WITH_CONTEXT(createProposal->updateelectoratemsgPtr, createProposal->updateelectoratemsgLen,
                 parser_readPB_UpdateElectorateMsg(&__tmpctx, &createProposal->updateelectoratemsg))

    return parser_ok;
}

parser_error_t parser_readPB_UpdateElectorateMsg(parser_context_t *ctx, parser_updateelectorate_t *updateElectorate) {
    uint64_t v;
    while (ctx->offset < ctx->bufferLen) {
        FAIL_ON_ERROR(_readRawVarint(ctx, &v))

        switch (FIELD_NUM(v)) {
            case PBIDX_UPDATEELECTORATEMSG_METADATA: {
                CHECK_NOT_DUPLICATED(updateElectorate->seen.metadata)
                READ_ARRAY(updateElectorate->metadata)
                break;
            }
            case PBIDX_UPDATEELECTORATEMSG_ELECTORATE_ID: {
                CHECK_NOT_DUPLICATED(updateElectorate->seen.electorate_id)
                READ_ARRAY(updateElectorate->electorateId)
                break;
            }
            case PBIDX_UPDATEELECTORATEMSG_ELECTOR: {
                if (updateElectorate->electorCount >= PBIDX_UPDATEELECTORATEMSG_ELECTOR_MAX){
                    return parser_unexpected_number_items;
                }

                parser_context_t local_ctx;
                FAIL_ON_ERROR(_readArray(ctx, &local_ctx.buffer, &local_ctx.bufferLen))
                local_ctx.offset = 0;
                local_ctx.lastConsumed = 0;

                parser_participant_t* p = &updateElectorate->elector_array[updateElectorate->electorCount];
                parser_ParticipantmsgInit(p);
                FAIL_ON_ERROR(parser_readPB_Participant(&local_ctx, p))

                updateElectorate->electorCount++;
                break;
            }
            default:
                // Unknown fields are rejected to avoid malleability
                return parser_unexpected_field;
        }
    }

    WITH_CONTEXT(updateElectorate->metadataPtr, updateElectorate->metadataLen,
                 parser_readPB_Metadata(&__tmpctx, &updateElectorate->metadata))

    return parser_ok;
}

parser_error_t parser_readPB_Root(parser_context_t *ctx) {
    parser_error_t err = parser_ok;
    uint64_t v;
    while (ctx->offset < ctx->bufferLen && err == parser_ok) {

        FAIL_ON_ERROR( _readRawVarint(ctx, &v))

        switch (FIELD_NUM(v)) {
            case PBIDX_TX_FEES: {
                CHECK_NOT_DUPLICATED(parser_tx_obj.seen.fees)
                err = _readArray(ctx, &parser_tx_obj.feesPtr, &parser_tx_obj.feesLen);
                break;
            }
            case PBIDX_TX_MULTISIG: {
                // This is a repeated field
                err = parser_readPB_Multisig(ctx, &parser_tx_obj.multisig);
                break;
            }
            case PBIDX_TX_SENDMSG: {
                CHECK_NOT_DUPLICATED(parser_tx_obj.seen.tx_message)
                err = _readArray(ctx, &parser_tx_obj.sendmsgPtr, &parser_tx_obj.sendmsgLen);
                parser_tx_obj.msgType = Msg_Send;
                break;
            }
            case PBIDX_TX_VOTEMSG: {
                CHECK_NOT_DUPLICATED(parser_tx_obj.seen.tx_message)
                err = _readArray(ctx, &parser_tx_obj.votemsgPtr, &parser_tx_obj.votemsgLen);
                parser_tx_obj.msgType = Msg_Vote;
                break;
            }
            case PBIDX_TX_UPDATE_MULTISIGMSG: {
                CHECK_NOT_DUPLICATED(parser_tx_obj.seen.tx_message)
                err = _readArray(ctx, &parser_tx_obj.updatemsgPtr, &parser_tx_obj.updatemsgLen);
                parser_tx_obj.msgType = Msg_Update;
                break;
            }
            case PBIDX_TX_CREATEPROPOSALMSG: {
                CHECK_NOT_DUPLICATED(parser_tx_obj.seen.tx_message)
                err = _readArray(ctx, &parser_tx_obj.createProposalmsgPtr, &parser_tx_obj.createProposalLen);
                parser_tx_obj.msgType = Msg_CreateProposal;
                break;
            }
            case PBIDX_TX_UPDATEELECTORATEMSG: {
                CHECK_NOT_DUPLICATED(parser_tx_obj.seen.tx_message)
                err = _readArray(ctx, &parser_tx_obj.createProposalmsg.updateelectoratemsgPtr,
                                      &parser_tx_obj.createProposalmsg.updateelectoratemsgLen);
                parser_tx_obj.msgType = Msg_UpdateElectorate;
                break;
            }
            default:
                // Unknown fields are rejected to avoid malleability
                return parser_unexpected_field;
        }
    }

    return err;
}

parser_error_t parser_readRoot(parser_context_t *ctx) {
    // ---------- READ CUSTOM HEADER (not protobuf)
    //version | len(chainID) | chainID      | nonce             | signBytes
    //4bytes  | uint8        | ascii string | int64 (bigendian) | serialized transaction

    if (ctx->bufferLen < TX_BUFFER_MIN) {
        return parser_unexpected_buffer_end;
    }

    parser_tx_obj.version = (uint32_t *) (ctx->buffer + 0);
    parser_tx_obj.chainIDLen = *(ctx->buffer + 4);

    if (parser_tx_obj.chainIDLen < TX_CHAINIDLEN_MIN) {
        return parser_unexpected_chain;
    }

    if (parser_tx_obj.chainIDLen > TX_CHAINIDLEN_MAX) {
        return parser_unexpected_buffer_end;
    }

    parser_tx_obj.chainID = ctx->buffer + 5;
    if (_checkChainIDValid(parser_tx_obj.chainID, parser_tx_obj.chainIDLen)) {
        return parser_unexpected_characters;
    }

    const uint8_t *p_src = ctx->buffer + 5 + parser_tx_obj.chainIDLen;
    uint8_t *p_dst = (uint8_t *) &parser_tx_obj.nonce;
    p_dst[0] = *(p_src + 7);
    p_dst[1] = *(p_src + 6);
    p_dst[2] = *(p_src + 5);
    p_dst[3] = *(p_src + 4);
    p_dst[4] = *(p_src + 3);
    p_dst[5] = *(p_src + 2);
    p_dst[6] = *(p_src + 1);
    p_dst[7] = *(p_src + 0);

    ctx->lastConsumed = 5 + parser_tx_obj.chainIDLen + 8;

    if (ctx->lastConsumed > ctx->bufferLen) {
        return parser_unexpected_buffer_end;
    }

    // ---------- VALIDATE HEADER
    // Check version
    if (*parser_tx_obj.version != 0x00feca00) {
        return parser_unexpected_version;
    }

    FAIL_ON_ERROR( _checkValidReadableChars(parser_tx_obj.chainID, parser_tx_obj.chainIDLen))

    ctx->offset += ctx->lastConsumed;
    ctx->lastConsumed = 0;

    // ---------- READ SERIALIZED TRANSACTION
    FAIL_ON_ERROR(parser_readPB_Root(ctx))

    return parser_ok;
}

parser_error_t parser_Tx(parser_context_t *ctx) {

    FAIL_ON_ERROR(parser_readRoot(ctx))

    WITH_CONTEXT(parser_tx_obj.feesPtr, parser_tx_obj.feesLen,
                 parser_readPB_Fees(&__tmpctx, &parser_tx_obj.fees))

    //Tx should contains only one of the following
    switch (parser_tx_obj.msgType) {
        case Msg_Send: {
            WITH_CONTEXT(parser_tx_obj.sendmsgPtr, parser_tx_obj.sendmsgLen,
                         parser_readPB_SendMsg(&__tmpctx, &parser_tx_obj.sendmsg))
            break;
        }
        case Msg_Vote: {
            WITH_CONTEXT(parser_tx_obj.votemsgPtr, parser_tx_obj.votemsgLen,
                         parser_readPB_VoteMsg(&__tmpctx, &parser_tx_obj.votemsg))
            break;
        }
        case Msg_Update: {
            WITH_CONTEXT(parser_tx_obj.updatemsgPtr, parser_tx_obj.updatemsgLen,
                         parser_readPB_UpdateMultisigMsg(&__tmpctx, &parser_tx_obj.updatemsg))
            break;
        }
        case Msg_CreateProposal: {
            WITH_CONTEXT(parser_tx_obj.createProposalmsgPtr, parser_tx_obj.createProposalLen,
                         parser_readPB_CreateProposalMsg(&__tmpctx, &parser_tx_obj.createProposalmsg))
            break;
        }
        case Msg_UpdateElectorate: {
            WITH_CONTEXT(parser_tx_obj.createProposalmsg.updateelectoratemsgPtr,
                         parser_tx_obj.createProposalmsg.updateelectoratemsgLen,
                         parser_readPB_UpdateElectorateMsg(&__tmpctx, &parser_tx_obj.createProposalmsg.updateelectoratemsg))
            break;
        }
        default:
            return parser_no_data;
    }

    return parser_ok;
}

bool_t parser_IsMainnet(const uint8_t *chainID, uint16_t chainIDLen) {
    if (chainIDLen < APP_MAINNET_CHAINID_PREFIX_LEN)
        return bool_false;

    const char *expectedChainIDPrefix = APP_MAINNET_CHAINID_PREFIX;
    for (uint16_t i = 0; i < APP_MAINNET_CHAINID_PREFIX_LEN; i++) {
        if (chainID[i] != expectedChainIDPrefix[i]) {
            return bool_false;
        }
    }

    return bool_true;
}

const char *parser_getHRP(const uint8_t *chainID, uint16_t chainIDLen) {
    if (parser_IsMainnet(chainID, chainIDLen) == bool_true)
        return APP_MAINNET_HRP;

    return APP_TESTNET_HRP;
}

parser_error_t parser_getAddress(const uint8_t *chainID, uint16_t chainIDLen,
                                 char *addr, uint16_t addrLen,
                                 const uint8_t *ptr, uint16_t len) {
    if (addrLen < IOV_ADDR_MAXLEN) {
        return parser_unexpected_buffer_end;
    }

    const char *hrp = parser_getHRP(chainID, chainIDLen);
    bech32EncodeFromBytes(addr, hrp, ptr, len);

    return parser_ok;
}

parser_error_t parser_arrayToString(char *out, uint16_t outLen,
                                    const uint8_t *in, uint8_t inLen,
                                    uint8_t pageIdx, uint8_t *pageCount) {
    // This function assumes that in is not zero-terminated
    // but outlen needs to be zero-terminated so it will reserve the last byte for termination

    if (pageCount == NULL && inLen > outLen - 1) {
        // It does not fit and paging is disabled
        return parser_unexpected_buffer_end;
    }

    MEMSET((void *) out, 0, outLen);      // Ensure it will be zero terminated

    if (pageCount != NULL) {
        *pageCount = 1 + inLen / (outLen - 1);
    } else {
        pageIdx = 0;
    }

    const int16_t offset = (outLen - 1) * pageIdx;

    // Limit chunk size
    int16_t chunkSize = outLen - 1;
    if (chunkSize > inLen - offset) {
        chunkSize = inLen - offset;
    }

    MEMCPY(out, in + offset, chunkSize);

    return parser_ok;
}

parser_error_t parser_formatAmountFriendly(char *out, uint16_t outLen, parser_coin_t *coin) {
    if (outLen < IOV_WHOLE_DIGITS + IOV_FRAC_DIGITS + 2) {
        return parser_unexpected_buffer_end;
    }
    MEMSET(out, 0, outLen);

    if (int64_to_str(out, outLen, coin->whole)) return parser_unexpected_buffer_end;
    uint8_t out_p = strlen(out);
    *(out + out_p) = '.';
    out_p++;
    MEMSET(out + out_p, '0', IOV_FRAC_DIGITS);        // Fill with 9 zeros

    if (coin->fractional > 0) {
        // concatenate and fill with zeros
        char f[IOV_FRAC_DIGITS + 1];
        MEMSET(f, 0, IOV_FRAC_DIGITS + 1);
        if (int64_to_str(f, IOV_FRAC_DIGITS + 1, coin->fractional)) return parser_unexpected_buffer_end;

        uint8_t fLen = strlen(f);
        MEMCPY(out + out_p + (IOV_FRAC_DIGITS - fLen), f, fLen);
    }

    return parser_ok;
}
