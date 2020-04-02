# IOV App
## General structure

The general structure of commands and responses is as follows:

#### Commands

| Field   | Type     | Content                | Note |
| :------ | :------- | :--------------------- | ---- |
| CLA     | byte (1) | Application Identifier | 0x22 |
| INS     | byte (1) | Instruction ID         |      |
| P1      | byte (1) | Parameter 1            |      |
| P2      | byte (1) | Parameter 2            |      |
| L       | byte (1) | Bytes in payload       |      |
| PAYLOAD | byte (L) | Payload                |      |

#### Response

| Field   | Type     | Content     | Note                     |
| ------- | -------- | ----------- | ------------------------ |
| ANSWER  | byte (?) | Answer      | depends on the command   |
| SW1-SW2 | byte (2) | Return code | see list of return codes |

#### Return codes

| Return code | Description             |
| ----------- | ----------------------- |
| 0x6400      | Execution Error         |
| 0x6982      | Empty buffer            |
| 0x6983      | Output buffer too small |
| 0x6986      | Command not allowed     |
| 0x6D00      | INS not supported       |
| 0x6E00      | CLA not supported       |
| 0x6F00      | Unknown                 |
| 0x9000      | Success                 |

---------

## Command definition

### GET_VERSION

#### Command

| Field | Type     | Content                | Expected |
| ----- | -------- | ---------------------- | -------- |
| CLA   | byte (1) | Application Identifier | 0x22     |
| INS   | byte (1) | Instruction ID         | 0x00     |
| P1    | byte (1) | Parameter 1            | ignored  |
| P2    | byte (1) | Parameter 2            | ignored  |
| L     | byte (1) | Bytes in payload       | 0        |

#### Response

| Field   | Type     | Content          | Note                            |
| ------- | -------- | ---------------- | ------------------------------- |
| TESTNET | byte (1) | Testnet Mode     | 0xFF means testnet mode is enabled |
| MAJOR   | byte (1) | Version Major    |                                 |
| MINOR   | byte (1) | Version Minor    |                                 |
| PATCH   | byte (1) | Version Patch    |                                 |
| LOCKED  | byte (1) | Device is locked |                                 |
| SW1-SW2 | byte (2) | Return code      | see list of return codes        |

--------------

### INS_GET_ADDR_ED25519

#### Command

| Field      | Type           | Content                   | Expected           |
| ---------- | -------------- | ------------------------- | ------------------ |
| CLA        | byte (1)       | Application Identifier    | 0x22               |
| INS        | byte (1)       | Instruction ID            | 0x01               |
| P1         | byte (1)       | Request User confirmation | No = 0             |
| P2         | byte (1)       | ignored                   |                    |
| L          | byte (1)       | Bytes in payload          | (depends)          |
| Path[0]    | bytes (4)      | Derivation Path Data      | 0x80000000 + 44    |
| Path[1]    | bytes (4)      | Derivation Path Data      | 0x80000000 + 234   |
| Path[2]    | bytes (4)      | Derivation Path Data      | 0x80000000 + index |

#### Response

| Field   | Type      | Content               | Note                     |
| ------- | --------- | --------------------- | ------------------------ |
| PK      | byte (32) | Public Key            |                          |
| ADDR    | byte (??) | bech32 Address        |                          |
| SW1-SW2 | byte (2)  | Return code           | see list of return codes |

--------------

### INS_SIGN_ED25519

#### Command

| Field | Type     | Content                | Expected  |
| ----- | -------- | ---------------------- | --------- |
| CLA   | byte (1) | Application Identifier | 0x22      |
| INS   | byte (1) | Instruction ID         | 0x02      |
| P1    | byte (1) | Payload desc           | 0 = init  |
|       |          |                        | 1 = add   |
|       |          |                        | 2 = last  |
| P2    | byte (1) | ----                   | not used  |
| L     | byte (1) | Bytes in payload       | (depends) |

The first packet/chunk includes only the derivation path

All other packets/chunks contain data chunks that are described below

*First Packet*

| Field      | Type     | Content                | Expected           |
| ---------- | -------- | ---------------------- | ------------------ |
| Path[0]    | byte (4) | Derivation Path Data   | 0x80000000 + 44    |
| Path[1]    | byte (4) | Derivation Path Data   | 0x80000000 + 234   |
| Path[2]    | byte (4) | Derivation Path Data   | 0x80000000 + index |

*Other Chunks/Packets*

| Field   | Type     | Content         | Expected |
| ------- | -------- | --------------- | -------- |
| Data    | bytes... | Message         |          |

Data is defined as:

| Field   | Type     | Content         | Expected |
| ------- | -------- | --------------- | -------- |
| Message | bytes..  | Payload to sign |          |

#### Response

| Field   | Type      | Content     | Note                     |
| ------- | --------- | ----------- | ------------------------ |
| SIG     | byte (64) | Signature   |                          |
| SW1-SW2 | byte (2)  | Return code | see list of return codes |

--------------
