// /////////////////////////////////////////////////////////////////////////////
///	@file dzcobs_decode.h
///	@brief decoding declarations
///
///	@par  Plataform Target:	Any
/// @par  Tab Size: 2
///
/// @copyright (C) 2025 Mario Luzeiro All rights reserved.
/// @author Mario Luzeiro <mluzeiro@ua.pt>
///
/// @par  License: Distributed under the 3-Clause BSD License. See accompanying
/// file LICENSE or a copy at https://opensource.org/licenses/BSD-3-Clause
/// SPDX-License-Identifier: BSD-3-Clause
///
// /////////////////////////////////////////////////////////////////////////////
#ifndef _DZCOBS_DECODE_H_
#define _DZCOBS_DECODE_H_

// Includes
// /////////////////////////////////////////////////////////////////////////////
#include "dzcobs.h"
#include "dzcobs_dictionary.h"

// clang-format off
#ifdef __cplusplus
extern "C" {
#endif
// clang-format on

// Definitions
// /////////////////////////////////////////////////////////////////////////////

typedef struct s_DZRCOB_decodectx
{
	const uint8_t *srcBufEncoded; ///< Source buffer encoded
	size_t srcBufEncodedLen;			///< Source buffer encoded data length
	uint8_t *dstBufDecoded;				///< Destiny buffer
	size_t dstBufDecodedSize;			///< Destiny buffer size

	///< Dictionaries that may be used on this decoding
	const sDICT_ctx *pDict[DZCOBS_DICT_N];
} sDZCOBS_decodectx;

/**
 * @brief Decodes a source encoded buffer.
 *
 * @param aDecodeCtx Struct with previous initialized
 * @param aOutDecodedLen Size of decoded data
 * @param uint8_t *aOutUser6bitDataRightAlgn The 6 bit user data that arrived in
 * the package. 1...63 (0 is not possible)
 * @retval RCOBS_RET_SUCCESS if decoded is ok
 * @retval RCOBS_RET_ERR_BAD_ARG if invalid arguments are passed
 * @retval RCOBS_RET_ERR_OVERFLOW if it overflows the destiny buffer
 * @retval RCOBS_RET_ERR_BAD_ENCODED_PAYLOAD if some invalid value (eg: 0x00)
 */
eDZCOBS_ret dzcobs_decode( const sDZCOBS_decodectx *aDecodeCtx,
													 size_t *aOutDecodedLen,
													 uint8_t *aOutUser6bitDataRightAlgn );

#ifdef __cplusplus
}
#endif

#endif

// EOF
// /////////////////////////////////////////////////////////////////////////////
