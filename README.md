# DZCOBS

**Dictionary Compression for [COBS](https://en.wikipedia.org/wiki/Consistent_Overhead_Byte_Stuffing) encoding**

## Concepts
### COBS
COBS (Consistent Overhead Byte Stuffing) is a framing method used to encode payloads for transmission over byte streams. It ensures that the data contains no zero bytes, allowing a zero byte to be safely used as a frame delimiter. COBS guarantees that the encoded frame size can be determined based on the original payload size, with minimal and consistent overhead.

### Dictionary based Compression
In addition to COBS encoding, a dictionary-based compression scheme is applied to reduce the overall data size of the encoded frame.

## Use cases and targets
  - Mid to high-end range microcontrollers.
  - Transmit data over slow streams (eg: UART) where there is available more CPU power than bandwith.
  - Reduce payload transmition costs (eg: over GPRS, 4G)
  - LOG and sensor data storage on non-volatile memory. Reduces storage space taking advantage of repetitive patterns.

## Usage
To integrate the code on your project, you need to consider only the following two folders:
  - [dzcobs](/dzcobs)
  -    [dzcobs/include](/dzcobs/include) header files to be added to your include paths.
  -    [dzcobs/src](/dzcobs/src) source code to be compiled by your project.

You may want to perform a [git sparse checkout](https://git-scm.com/docs/git-sparse-checkout) to your project with only those folders.

No CMAKE file to build project as a library is currently supported.

All other files on this repository are intended for internal use.

## License
Distributed under the 3-Clause BSD License. See accompanying file LICENSE or a copy at https://opensource.org/licenses/BSD-3-Clause

SPDX-License-Identifier: BSD-3-Clause

## Contribution
Any contribution to the project be submitted under the same license of this project.

## References
Other related project references:
  - [rCOBS](https://github.com/Dirbaio/rcobs)
