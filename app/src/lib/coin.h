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

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

#define HDPATH_0_DEFAULT     (0x80000000u | 0x2cu)
#define HDPATH_1_DEFAULT     (0x80000000u | 0xeau)

#define APP_MAINNET_HRP          "iov"
#define APP_MAINNET_CHAINID      "iov-mainnet"
#define APP_MAINNET_CHAINID_LEN   11

#define APP_TESTNET_HRP          "tiov"

#define IOV_PK_PREFIX      "sigs/ed25519/"
#define IOV_PK_PREFIX_LEN  13
#define IOV_TICKER_MAXLEN   5
#define IOV_ADDR_MAXLEN    45

#define IOV_WHOLE_DIGITS   15
#define IOV_FRAC_DIGITS    9

#ifdef __cplusplus
}
#endif
