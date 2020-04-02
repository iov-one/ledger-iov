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
#include <stddef.h>
#include "parser_txdef.h"

void parser_metadataInit(parser_metadata_t *metadata) {
    metadata->seen.schema = 0;

    metadata->schema = 0;
}

void parser_coinInit(parser_coin_t *coin) {
    coin->seen.whole = 0;
    coin->seen.fractional = 0;
    coin->seen.ticker = 0;

    coin->whole = 0;
    coin->fractional = 0;
    coin->tickerPtr = NULL;
    coin->tickerLen = 0;
}

void parser_multisigInit(parser_multisig_t *v) {
    v->count = 0;
}

void parser_feesInit(parser_fees_t *fees) {
    fees->seen.payer = 0;
    fees->seen.coin = 0;

    fees->payerPtr = NULL;
    fees->payerLen = 0;

    fees->coinPtr = NULL;
    fees->coinLen = 0;
    parser_coinInit(&fees->coin);
}

void parser_sendmsgInit(parser_sendmsg_t *msg) {
    msg->seen.metadata = 0;
    msg->seen.source = 0;
    msg->seen.destination = 0;
    msg->seen.amount = 0;
    msg->seen.memo = 0;
    msg->seen.ref = 0;

    msg->metadataPtr = NULL;
    msg->metadataLen = 0;
    parser_metadataInit(&msg->metadata);

    msg->sourcePtr = NULL;
    msg->sourceLen = 0;

    msg->destinationPtr = NULL;
    msg->destinationLen = 0;

    msg->amountPtr = NULL;
    msg->amountLen = 0;
    parser_coinInit(&msg->amount);

    msg->memoPtr = NULL;
    msg->memoLen = 0;

    msg->refPtr = NULL;
    msg->refLen = 0;
}

void parser_votemsgInit(parser_votemsg_t *msg) {
    msg->seen.metadata = 0;
    msg->seen.id = 0;
    msg->seen.voter = 0;
    msg->seen.voteOption = 0;

    msg->metadataPtr = NULL;
    msg->metadataLen = 0;
    parser_metadataInit(&msg->metadata);

    msg->proposalIdPtr = NULL;
    msg->proposalIdLen = 0;

    msg->voterPtr = NULL;
    msg->voterLen = 0;

    msg->voteOption = 0;
}

void parser_updatemsgInit(parser_updatemultisigmsg_t *msg) {
    msg->seen.metadata = 0;
    msg->seen.id = 0;
    msg->seen.activation_th = 0;
    msg->seen.admin_th = 0;

    msg->metadataPtr = NULL;
    msg->metadataLen = 0;
    parser_metadataInit(&msg->metadata);

    msg->participantsCount = 0;

    msg->contractIdPtr = NULL;
    msg->contractIdLen = 0;
}

void parser_ParticipantmsgInit(parser_participant_t *msg) {
    msg->seen.weight = 0;
    msg->seen.signature = 0;

    msg->addressLen = 0;
    msg->addressPtr = NULL;

    msg->weight = 0;
}

void parser_createProposalmsgInit(parser_createproposalmsg_t *msg) {
    msg->seen.metadata = 0;
    msg->seen.title = 0;
    msg->seen.raw_option = 0;
    msg->seen.description = 0;
    msg->seen.election_rule_id = 0;
    msg->seen.start_time = 0;
    msg->seen.author = 0;

    msg->metadataPtr = NULL;
    msg->metadataLen = 0;
    parser_metadataInit(&msg->metadata);

    msg->titlePtr = NULL;
    msg->titleLen = 0;

    msg->rawOptionPtr = NULL;
    msg->rawOptionLen = 0;

    msg->updateelectoratemsgPtr = NULL;
    msg->updateelectoratemsgLen = 0;
    parser_updateElectoratemsgInit(&msg->updateelectoratemsg);

    msg->descriptionPtr = NULL;
    msg->descriptionLen = 0;

    msg->electionRuleIdPtr = NULL;
    msg->electionRuleIdLen = 0;

    msg->startTime = 0;

    msg->authorPtr = NULL;
    msg->authorLen = 0;
}

void parser_updateElectoratemsgInit(parser_updateelectorate_t *msg) {
    msg->seen.metadata = 0;
    msg->seen.electorate_id = 0;

    msg->metadataPtr = NULL;
    msg->metadataLen = 0;
    parser_metadataInit(&msg->metadata);

    msg->electorateIdPtr = NULL;
    msg->electorateIdLen = 0;

    msg->electorCount = 0;
}

void parser_txInit(parser_tx_t *tx) {
    tx->seen.fees = 0;
    tx->seen.tx_message = 0;

    tx->version = NULL;
    tx->chainIDLen = 0;
    tx->chainID = NULL;
    tx->nonce = 0;
    tx->msgType = Msg_Invalid;

    tx->feesPtr = NULL;
    tx->feesLen = 0;
    parser_feesInit(&tx->fees);

    tx->multisigPtr = NULL;
    tx->multisigLen = 0;
    parser_multisigInit(&tx->multisig);

    tx->sendmsgPtr = NULL;
    tx->sendmsgLen = 0;
    parser_sendmsgInit(&tx->sendmsg);

    tx->votemsgPtr = NULL;
    tx->votemsgLen = 0;
    parser_votemsgInit(&tx->votemsg);

    tx->updatemsgPtr = NULL;
    tx->updatemsgLen = 0;
    parser_updatemsgInit(&tx->updatemsg);

    tx->createProposalmsgPtr = NULL;
    tx->createProposalLen = 0;
    parser_createProposalmsgInit(&tx->createProposalmsg);
}

