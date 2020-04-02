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

#include "coin.h"

//version | len(chainID) | chainID      | nonce             | signBytes
//4bytes  | uint8        | ascii string | int64 (bigendian) | serialized transaction

//1, 2    Fees: Payer           [Addr - Bytes]
//1, 3    Fees: Coin            [Coin] -> Stringify

//51, 1   SendMsg: Metadata     [?????]
//51, 2   SendMsg: Source       [Addr - Bytes]
//51, 3   SendMsg: Destination  [Addr - Bytes]
//51, 4   SendMsg: Amount       [Coin] -> Stringify
//51, 5   SendMsg: Memo         [String]
//51, 6   SendMsg: Ref          [?????]

//[Coin] -> Stringify
//?, 1    Whole
//?, 2    Fractional
//?, 3    Ticker

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

#define TX_BUFFER_MIN       4
#define TX_CHAINIDLEN_MIN   4
#define TX_CHAINIDLEN_MAX   32
#define TX_MEMOLEN_MAX      128
#define PBIDX_METADATA_SCHEMA      1

typedef struct {
    // These bits are to avoid duplicated fields
    struct {
        unsigned int schema : 1;
    } seen;

    uint32_t schema;
} parser_metadata_t;

#define PBIDX_COIN_WHOLE           1
#define PBIDX_COIN_FRACTIONAL      2
#define PBIDX_COIN_TICKER          3

typedef struct {
    // These bits are to avoid duplicated fields
    struct {
        unsigned int whole : 1;
        unsigned int fractional : 1;
        unsigned int ticker : 1;
    } seen;

    int64_t whole;
    int64_t fractional;
    const uint8_t *tickerPtr;
    uint16_t tickerLen;
} parser_coin_t;

#define PBIDX_FEES_PAYER           2
#define PBIDX_FEES_COIN            3

typedef struct {
    // These bits are to avoid duplicated fields
    struct {
        unsigned int payer : 1;
        unsigned int coin : 1;
    } seen;

    const uint8_t *payerPtr;
    uint16_t payerLen;

    const uint8_t *coinPtr;
    uint16_t coinLen;
    parser_coin_t coin;
} parser_fees_t;

#define PBIDX_MULTISIG_COUNT_MAX        8

typedef struct {
    // These bits are to avoid duplicated fields
    uint8_t count;
    uint64_t values[PBIDX_MULTISIG_COUNT_MAX];
} parser_multisig_t;

#define PBIDX_SENDMSG_METADATA          1
#define PBIDX_SENDMSG_SOURCE            2
#define PBIDX_SENDMSG_DESTINATION       3
#define PBIDX_SENDMSG_AMOUNT            4
#define PBIDX_SENDMSG_MEMO              5
#define PBIDX_SENDMSG_REF               6

typedef struct {
    // These bits are to avoid duplicated fields
    struct {
        unsigned int metadata : 1;
        unsigned int source : 1;
        unsigned int destination : 1;
        unsigned int amount : 1;
        unsigned int memo : 1;
        unsigned int ref : 1;
    } seen;

    const uint8_t *metadataPtr;
    uint16_t metadataLen;
    parser_metadata_t metadata;

    const uint8_t *sourcePtr;
    uint16_t sourceLen;

    const uint8_t *destinationPtr;
    uint16_t destinationLen;

    const uint8_t *amountPtr;
    uint16_t amountLen;
    parser_coin_t amount;

    const uint8_t *memoPtr;
    uint16_t memoLen;

    const uint8_t *refPtr;
    uint16_t refLen;
} parser_sendmsg_t;

#define PBIDX_VOTEMSG_METADATA      1
#define PBIDX_VOTEMSG_PROPOSAL_ID   2
#define PBIDX_VOTEMSG_VOTER         3
#define PBIDX_VOTEMSG_VOTE          4

#define VOTE_OPTION_INVALID    0
#define VOTE_OPTION_YES        1
#define VOTE_OPTION_NO         2
#define VOTE_OPTION_ABSTAIN    3

#define VOTE_OPTION_INVALID_STR    "invalid"
#define VOTE_OPTION_YES_STR        "yes"
#define VOTE_OPTION_NO_STR         "no"
#define VOTE_OPTION_ABSTAIN_STR    "abstain"

typedef struct {
    // These bits are to avoid duplicated fields
    struct {
        unsigned int metadata : 1;
        unsigned int id : 1;
        unsigned int voter : 1;
        unsigned int voteOption : 1;
    } seen;

    const uint8_t *metadataPtr;
    uint16_t metadataLen;
    parser_metadata_t metadata;

    const uint8_t *proposalIdPtr;
    uint16_t proposalIdLen;

    const uint8_t *voterPtr;
    uint16_t voterLen;

    uint8_t voteOption;
} parser_votemsg_t;

#define PBIDX_UPDATEMSG_METADATA          1
#define PBIDX_UPDATEMSG_ID                2
#define PBIDX_UPDATEMSG_PARTICIPANTS      3
#define PBIDX_UPDATEMSG_ACTIVATION_TH     4
#define PBIDX_UPDATEMSG_ADMIN_TH          5

#define PBIDX_PARTICIPANTMSG_ADDRESS      1
#define PBIDX_PARTICIPANTMSG_WEIGHT       2

#define PBIDX_UPDATEMSG_PARTICIPANTS_MAX 16

typedef struct {
    // These bits are to avoid duplicated fields
    struct {
        unsigned int signature : 1;
        unsigned int weight : 1;
    } seen;

    // On .proto file this field is named as "signature" but is intended to encode an address
    const uint8_t *addressPtr;
    uint16_t addressLen;

    uint32_t weight;
} parser_participant_t;

typedef struct {
    // These bits are to avoid duplicated fields
    struct {
        unsigned int metadata : 1;
        unsigned int id : 1;
        unsigned int activation_th : 1;
        unsigned int admin_th : 1;
    } seen;

    const uint8_t *metadataPtr;
    uint16_t metadataLen;
    parser_metadata_t metadata;

    const uint8_t *contractIdPtr;
    uint16_t contractIdLen;

    //Participants is a repeated field
    uint8_t participantsCount; //Total participants fields in Tx
    parser_participant_t participant_array[PBIDX_UPDATEMSG_PARTICIPANTS_MAX];

    uint32_t activation_th;
    uint32_t admin_th;
} parser_updatemultisigmsg_t;

#define PBIDX_UPDATEELECTORATEMSG_METADATA      1
#define PBIDX_UPDATEELECTORATEMSG_ELECTORATE_ID 2
#define PBIDX_UPDATEELECTORATEMSG_ELECTOR       3

#define PBIDX_UPDATEELECTORATEMSG_ELECTOR_MAX  8

typedef struct {
    struct {
        unsigned int metadata : 1;
        unsigned int electorate_id :1;
    } seen;

    const uint8_t *metadataPtr;
    uint16_t metadataLen;
    parser_metadata_t metadata;

    const uint8_t *electorateIdPtr;
    uint16_t electorateIdLen;

    //Elector is a repeated field
    uint8_t electorCount; //Total Electors fields in Tx
    parser_participant_t elector_array[PBIDX_UPDATEELECTORATEMSG_ELECTOR_MAX];
} parser_updateelectorate_t;

#define PBIDX_CREATEPROPOSALMSG_METADATA    1
#define PBIDX_CREATEPROPOSALMSG_TITLE       2
#define PBIDX_CREATEPROPOSALMSG_OPTION      3
#define PBIDX_CREATEPROPOSALMSG_DESCRIPTION 4
#define PBIDX_CREATEPROPOSALMSG_RULEID      5
#define PBIDX_CREATEPROPOSALMSG_STARTTIME   6
#define PBIDX_CREATEPROPOSALMSG_AUTHOR      7

typedef struct {
    // These bits are to avoid duplicated fields
    struct {
        unsigned int metadata : 1;
        unsigned int title : 1;
        unsigned int raw_option : 1;
        unsigned int description : 1;
        unsigned int election_rule_id : 1;
        unsigned int start_time: 1;
        unsigned int author : 1;
    } seen;

    const uint8_t *metadataPtr;
    uint16_t metadataLen;
    parser_metadata_t metadata;

    const uint8_t *titlePtr;
    uint16_t titleLen;

    const uint8_t *rawOptionPtr;
    uint16_t rawOptionLen;

    //UpdateElectorateMsg is encoded in raw_option field
    const uint8_t *updateelectoratemsgPtr;
    uint16_t updateelectoratemsgLen;
    parser_updateelectorate_t updateelectoratemsg;

    const uint8_t *descriptionPtr;
    uint16_t descriptionLen;

    const uint8_t *electionRuleIdPtr;
    uint16_t electionRuleIdLen;

    int64_t startTime;

    const uint8_t *authorPtr;
    uint16_t authorLen;
} parser_createproposalmsg_t;




#define PBIDX_TX_FEES                  1
#define PBIDX_TX_MULTISIG              4
#define PBIDX_TX_SENDMSG              51
#define PBIDX_TX_UPDATE_MULTISIGMSG   57
#define PBIDX_TX_CREATEPROPOSALMSG    73
#define PBIDX_TX_VOTEMSG              75
#define PBIDX_TX_UPDATEELECTORATEMSG  77

typedef enum {
    Msg_Invalid = 0,
    Msg_Send,
    Msg_Vote,
    Msg_Update,
    Msg_CreateProposal,
    Msg_UpdateElectorate
} MsgType;

typedef struct {
    const uint32_t *version;
    uint8_t chainIDLen;
    const uint8_t *chainID;
    int64_t nonce;
    MsgType msgType;

    ////
    struct {
        unsigned int fees : 1;
        unsigned int tx_message : 1;
    } seen;

    const uint8_t *feesPtr;
    uint16_t feesLen;
    parser_fees_t fees;             // PB Field 1

    const uint8_t *multisigPtr;
    uint16_t multisigLen;
    parser_multisig_t multisig;     // PB Field 4

    //TxMsg has only one of the following
    union {
        struct {
            const uint8_t *sendmsgPtr;
            uint16_t sendmsgLen;
            parser_sendmsg_t sendmsg;       // PB Field 51
        };
        struct {
            const uint8_t *updatemsgPtr;
            uint16_t updatemsgLen;
            parser_updatemultisigmsg_t updatemsg;   // PB Field 57
        };
        struct {
            const uint8_t *createProposalmsgPtr;
            uint16_t createProposalLen;
            parser_createproposalmsg_t createProposalmsg; // PB Field 73
        };
        struct {
            const uint8_t *votemsgPtr;
            uint16_t votemsgLen;
            parser_votemsg_t votemsg;       // PB Field 75
        };
        //
    };
} parser_tx_t;

void parser_coinInit(parser_coin_t *coin);
void parser_feesInit(parser_fees_t *fees);
void parser_multisigInit(parser_multisig_t *msg);
void parser_sendmsgInit(parser_sendmsg_t *msg);
void parser_votemsgInit(parser_votemsg_t *msg);
void parser_updatemsgInit(parser_updatemultisigmsg_t *msg);
void parser_ParticipantmsgInit(parser_participant_t *msg);
void parser_createProposalmsgInit(parser_createproposalmsg_t *msg);
void parser_updateElectoratemsgInit(parser_updateelectorate_t *msg);
void parser_txInit(parser_tx_t *tx);

#ifdef __cplusplus
}
#endif
