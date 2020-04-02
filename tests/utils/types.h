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
#pragma once

typedef struct {
    std::string quantity;
    uint64_t fractionalDigits;
    std::string tockenTicker;
} token_amount_t;

typedef struct {
    std::string address;
    uint32_t weight;
} participant_t;

typedef struct {
    std::string sender;
    std::string recipient;
    std::vector<uint64_t> multisig;
    std::string memo;
    //VoteMsg related
    std::string voter;
    uint64_t proposalId;
    std::string voteOption;
    //UpdateMsg related
    uint64_t contractId;
    std::vector<participant_t> participant;
    uint32_t activation_th;
    uint32_t admin_th;
    //CreateProposal related
    std::string proposalTitle;
    std::string proposalDescription;
    std::string proposalAuthor;
    uint64_t  electionRuleId;
    uint64_t  startTime;
    //UpdateElectorate related
    uint64_t electorateId;
    std::vector<participant_t> electors;
    //
    token_amount_t amount;
    token_amount_t fee;
} transaction_t;

typedef struct {
    std::string description;
    std::string chainId;
    transaction_t transaction;
    uint64_t nonce;
    std::string bytes;

    bool valid;
    bool testnet;
    std::vector<uint8_t> blob;
    std::vector<std::string> expected_ui_output;
} testcaseData_t;

typedef struct {
    std::shared_ptr<Json::Value> testcases;
    int64_t index;
    std::string description;
} testcase_t;
