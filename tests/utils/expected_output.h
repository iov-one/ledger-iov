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
#include <fstream>
#include <vector>
#include <json/json.h>
#include "types.h"
#include <lib/parser_txdef.h>

#define MSG_TYPE_SEND_STR "bcp/send"
#define MSG_TYPE_VOTE_STR "bns/vote"

std::vector<std::string> GenerateExpectedUIOutput(const testcaseData_t &tcd);

//Generate output for specific type of message
MsgType getMsgType(const testcaseData_t &tcd);
std::vector<std::string> GenerateExpectedSendMsgOutput(const testcaseData_t &tcd);
std::vector<std::string> GenerateExpectedVoteMsgOutput(const testcaseData_t &tcd);
