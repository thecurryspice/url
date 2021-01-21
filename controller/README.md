# Packet Type

URL receives 2 types of packets : Command and Data.
Command packet is a fixed message of 3 bytes, while a Data packet can be upto 8 bytes.

# MESSAGE FRAME

```
|--------|--------|-------->
   HEAD    COMMAND   DATA
```

## HEAD

The `HEAD` byte is made up of two fields: a 4-bit `START` sequence and a 4-bit `LENGTH` number.  
The value of the `START` sequence is a nonce, which is generated from a *context* established during the security handshake (see *Security* section below).
The maximum value of `LENGTH` shall be open to implementation, constrained to a lower bound the device is expected to send this value during the capability exchange after the security handshake.

The `COMMAND` byte is broken into two fields : 5-bit `OPCODE` and 3-bit `COLOUR`

### OPCODE

The MSB in the OPCODE determines whether a frame denare
The mnemonics are with respect to the application host, not the device. For example, `GET_TIME` involves data transfer from Slave to Master.

|Opcode (D2C)|Mnemonic|Description|Mnemonic|Opcode (C2D)|
|:---:|:---:|:---:|:---:|:---:|
|00000|GET_TIME|Real time value in seconds (64 bit integer)|SET_TIME|10000|
|00001|GET_CONTROL|Control attributes related to the chosen colour|SET_CONTROL|10001|
|00010|GET_CLRDEPTH|Colour Depth of the LED driver|SET_CLRDEPTH|10010|
|00100|GET_MINVAL|Minimum value of a colour|SET_MINVAL|10010|
|00101|GET_MAXVAL|Maximum value of a colour|SET_MAXVAL|10011|
|01000|GET_TIMEON|`onTime` value|SET_TIMEON|10110|
|01001|GET_TIMEOFF|`offTime` value|SET_TIMEOFF|10111|
|01010|GET_TIMERON|Wake-Up timer value|SET_TIMERON|11000|
|01011|GET_TIMEROFF|Shut-Down timer value|SET_TIMEROFF|11011|
|01111|RES_SEC|Security Handshake|REQ_SEC|11111|


|Opcode|Mnemonic|Description|Data Packet (bytes)|
|:---:|:---:|:---:|:---:|
|00000|ACK| Acknowledgement|1 byte opcode of the acknowledged command + Response (Optional)|
|00001|NACK| Negative Acknowledgement - rejection of a command|1 byte opcode of the rejected command + Response (Optional)|
|10000|SET_TIME| Real time value in seconds |  Variable |
|10001|SET_CONTROL|Control attributes related to the chosen colour| 1 |
|10010|SET_CLRDEPTH|Colour Depth of the LED driver| 1 |
|10100|SET_MINVAL|Minimum value of a colour| 1 byte global minimum, 1 byte pulse minimum |
|10101|SET_MAXVAL|Maximum value of a colour| 1 byte global maximum, 1 byte pulse maximum |
|11000|SET_TIMEON|pulse-on time value in ms | upto 4 |
|11001|SET_TIMEOFF|pulse-off time value in ms | upto 4 |
|11010|SET_TIMERON|Wake-Up timer value| upto 8 |
|11011|SET_TIMEROFF|Shut-Down timer value| upto 8 |
|01111|RES_SEC|Security Handshake|upto 16|


> Need to revise this. Device needs to send back only limited commands.

### COLOUR

The colour field determines the targeted colour.

|COLOUR|Description|
|:---:|:---:|
|000|Default|
|001|Red|
|010|Green|
|011|Blue|
|100|Warm White|
|101|Cool White|
|11x|Reserved|


### PHASE

When used with the `REx_SEC` opcode, the `COLOUR` field determines a `PHASE` number, which is used to denote different steps in establishing an authorised connection.

---

## CONTROL BYTE

Control Byte contains the routing information and toggles the state of active channels.

```
pos     7   6   5   4   3   2   1   0
route   L   T   M   B   U   P   F   R
```

|Abb|Acr|
|:---:|:---:|
| R | Random |
| F | Flash |
| L | Watch External Light Levels |
| P | Pulse (Breather Mode) |
| U | User |
| B | Bass |
| M | Mid |
| T | Treble |


---

## SECURITY

The implementation of security-features in this standard is aimed at replay protection instead of data encryption. However, encrypting every message easily exceeds the computation budget, violating real-time constrains. Nonentheless, there is a requirement of keeping the data encrypted to prevent MITM attacks.

URL uses a secure handshake at the application-layer to guard against brute-force injections and replay attacks. This makes it agnostic to the protocol and medium being used, although injecting marginal compuatation overhead.  
This handshake follows a specific flow which grants the controller a leased, authorised connection.

## Phase 1

1. Controller sends a security request command (`REQ_SEC`), for which the data-payload defaults to 16 bytes. This data payload contains the ECDSA public key of the controller. The device sends back a security response command (`RES_SEC`), with its own public key.
2. The device initialises a 63-bit LFSR at a random seed, encodes it with its private key and the controller's public key, and streams this packet. The controller waits against a timeout until it receives a `RES_SEC-PHASE2`. Upon reception, the controller creates a `REQ_SEC-PHASE2`, encoding the next LFSR sequence using its own private key and the device's public key.
3. The third phase marks the creation of a security `CONTEXT`
If the `SNIPPET` received by the device does not match the expected value at the device's end, a `NACK` is sent.
If the `SNIPPET` received by the controller does not match the expected value at the controller's end, the `CONTEXT` is assumed to be out of sync. The only way to assume control of the device is to re-initiate the security handshake.

NACKs : In case of `CONTEXT` out-of-sync, the 1 byte data packet for the `NACK` is reserved to be 0x00

---

> Not needed because the `SNIPPET` is already masked with the context

The `SNIPPET` is generated by using the value of the last populated `0x07 & LENGTH` field.  

Inside the LFSR data structure, the bits present at the position `0x07 & LENGTH` in the least significant 4 bytes are extracted to form the `SNIPPET` nibble.
This ensures that



## Summary

| Phase | Header | Command | Data | Direction | Mask |
|:---:|:---:|:---:|:---:|:---:|:---:|
| 1 | 0xFF | `REQ_SEC-PHASE1` | public key | C2D | None |
| 1 | 0xFF | `RES_SEC-PHASE1` | public key | D2C | None |
| 2 | 0xFF | `RES_SEC-PHASE2` | encoded random-seed for 63-bit LFSR | D2C | None |
| 2 | 0xFF | `REQ_SEC-PHASE2` | encoded next LFSR sequence | C2D | None |
| 3 | 0x`SNIPPET` `LENGTH` | `RES_SEC-PHASE3` | Lease time in terms of number of commands requests | D2C | `CONTEXT` |
| 3 | 0x`SNIPPET` `LENGTH` | `REQ_SEC-PHASE3` | Clean exit from Security handshake (0x0) | C2D | `CONTEXT` |
| 3 | 0x`SNIPPET` `1111`   | `RES_SEC-PHASE3` | Acknowledgement of clean exit (0x0) | D2C | `CONTEXT` |

---








## DATA PACKETS

X = R/G/B

TIME 		:	T<X><char><char><char><char>

```					
	where   :	<char>    <char>			16-bit total-value
			  multiplier  remainder		255*multiplier + remainder
```

USER 		:	U<X><char>

EXTLIGHT	:	E<X><char>

```
Eg:	~TGmXxM	==> 	onTime = 255*109 + 88	offTime = 255*120 + 77
	~URn
	~EB/
```
