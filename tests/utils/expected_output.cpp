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
#include <fmt/core.h>
#include <lib/coin.h>
#include "testcases.h"
#include "zxmacros.h"
#include "expected_output.h"

bool TestcaseIsValid(const Json::Value &tc) {
    return true;
}

template<typename S, typename... Args>
void addTo(std::vector<std::string> &answer, const S &format_str, Args &&... args) {
    answer.push_back(fmt::format(format_str, args...));
}

std::string FormatAddress(const std::string &address, uint8_t idx, uint8_t *pageCount) {
    char outBuffer[40];
    pageString(outBuffer, sizeof(outBuffer), address.c_str(), idx, pageCount);

    return std::string(outBuffer);
}

//std::string FormatAmount(const std::string &amount) {
//    char buffer[500];
//    MEMZERO(buffer, sizeof(buffer));
//    fpstr_to_str(buffer, amount.c_str(), COIN_AMOUNT_DECIMAL_PLACES);
//    return std::string(buffer);
//}
//
//std::vector<uint8_t> prepareBlob(const std::string &base64Cbor) {
//    std::string cborString;
//    macaron::Base64::Decode(base64Cbor, cborString);
//
//    // Allocate and prepare buffer
//    // CBOR payload
//    auto bufferAllocation = std::vector<uint8_t>(cborString.size());
//
//    MEMCPY(bufferAllocation.data(), cborString.c_str(), cborString.size());
//
//    return bufferAllocation;
//}

std::vector<std::string> GenerateExpectedUIOutput(const testcaseData_t &tcd) {
    auto answer = std::vector<std::string>();

    if (!tcd.valid) {
        answer.emplace_back("Test case is not valid!");
        return answer;
    }

    switch (getMsgType(tcd))
    {
        case Msg_Invalid:
            answer.emplace_back("Test case is not valid!");
            return answer;
        case Msg_Send:
            return GenerateExpectedSendMsgOutput(tcd);
        case Msg_Vote:
            return GenerateExpectedVoteMsgOutput(tcd);
    }

    return answer;
}

std::vector<std::string> GenerateExpectedSendMsgOutput(const testcaseData_t &tcd) {
    auto answer = std::vector<std::string>();

    uint8_t dummy;
    char buffer[1000];

    addTo(answer, "0 | ChainID : {}", tcd.chainId);
    addTo(answer, "1 | Source : {}", FormatAddress(tcd.transaction.sender, 0, &dummy));
    addTo(answer, "1 | Source : {}", FormatAddress(tcd.transaction.sender, 1, &dummy));
    addTo(answer, "2 | Dest : {}", FormatAddress(tcd.transaction.recipient, 0, &dummy));
    addTo(answer, "2 | Dest : {}", FormatAddress(tcd.transaction.recipient, 1, &dummy));

    fpstr_to_str(buffer, sizeof(buffer), tcd.transaction.amount.quantity.c_str(), IOV_FRAC_DIGITS);
    addTo(answer, "3 | Amount [{}] : {}", tcd.transaction.amount.tockenTicker, buffer);

    fpstr_to_str(buffer, sizeof(buffer), tcd.transaction.fee.quantity.c_str(), IOV_FRAC_DIGITS);
    addTo(answer, "4 | Fees [{}] : {}", tcd.transaction.fee.tockenTicker, buffer);

    if (!tcd.transaction.memo.empty()) {
        char inBuffer[1000];
        asciify_ext(tcd.transaction.memo.c_str(), inBuffer);

        char outBuffer[40];
        uint16_t idx = 0;
        uint8_t pageCount = 1;

        while (pageCount > 0 && idx < pageCount) {
            pageString(outBuffer, sizeof(outBuffer), inBuffer, idx, &pageCount);
            idx++;
            addTo(answer, "5 | Memo : {}", outBuffer);
        }
    }

    for (size_t i = 0; i < tcd.transaction.multisig.size(); i++) {
        if (tcd.transaction.multisig.size() > 1) {
            addTo(answer, "{} | Multisig [{}/{}] : {}",
                  6 + i, i + 1, tcd.transaction.multisig.size(), tcd.transaction.multisig[i]);
        } else {
            addTo(answer, "{} | Multisig : {}", 6+i, tcd.transaction.multisig[i]);
        }
    }

    return answer;
}

std::vector<std::string> GenerateExpectedVoteMsgOutput(const testcaseData_t &tcd) {
    auto answer = std::vector<std::string>();

    uint8_t dummy;
    char buffer[1000];

    addTo(answer, "0 | ChainID : {}", tcd.chainId);
    fpuint64_to_str(buffer, sizeof(buffer), tcd.transaction.proposalId, 0);
    addTo(answer, "1 | ProposalId : {}", buffer);
    addTo(answer, "2 | Selection : {}", tcd.transaction.voteOption);
    addTo(answer, "3 | Voter : {}", FormatAddress(tcd.transaction.voter, 0, &dummy));
    addTo(answer, "3 | Voter : {}", FormatAddress(tcd.transaction.voter, 1, &dummy));

    return answer;
}

MsgType getMsgType(const testcaseData_t &tcd) {
    std::string type = tcd.description;
    if(type == MSG_TYPE_SEND_STR)
        return Msg_Send;
    if(type == MSG_TYPE_VOTE_STR)
        return Msg_Vote;

    return Msg_Invalid;
}
