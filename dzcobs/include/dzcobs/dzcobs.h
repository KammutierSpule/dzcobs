// /////////////////////////////////////////////////////////////////////////////
///	@file dzcobs.h
///	@brief encoding declarations
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
#ifndef _DZCOBS_H_
#define _DZCOBS_H_

// Includes
// /////////////////////////////////////////////////////////////////////////////
#include <stdbool.h>
#include "dzcobs_dictionary.h"

// clang-format off
#ifdef __cplusplus
extern "C" {
#endif
// clang-format on

// Definitions
// /////////////////////////////////////////////////////////////////////////////

#ifndef DZCOBS_IS_DEBUG_BUILD
#ifdef ASAP_IS_DEBUG_BUILD
#define DZCOBS_IS_DEBUG_BUILD 1
#endif
#endif

enum
{
	DZCOBS_FRAME_HEADER_SIZE = ( 2 )
};

// Order-independent multiset hash
// This is slight better than checksum and xor (see test_checksum.cpp)
#define DZCOBS_HASH8( b ) ( ( ( ( b ) ^ ( ( b ) >> 3 ) ) * 167 ) ^ ( ( b ) << 1 ) )

typedef enum e_DZCOBS_ret
{
	DZCOBS_RET_SUCCESS = 0,
	DZCOBS_RET_ERR_BAD_ARG,
	DZCOBS_RET_ERR_WRITE_OVERFLOW,
	DZCOBS_RET_ERR_READ_OVERFLOW,
	DZCOBS_RET_ERR_NOTINITIALIZED,
	DZCOBS_RET_ERR_BAD_ENCODED_PAYLOAD,
	DZCOBS_RET_ERR_CRC,
	DZCOBS_RET_ERR_NO_DICTIONARY_TO_DECODE,
	DZCOBS_RET_ERR_WORD_NOT_FOUND_ON_DICTIONARY,
	DZCOBS_RET_ERR_INVALID_USER6BITS
} eDZCOBS_ret;

typedef enum e_DZCOBS_encoding
{
	DZCOBS_PLAIN				= 0, ///< No compression
	DZCOBS_USING_DICT_1 = 1, ///< Compression using dictionary 1
	DZCOBS_USING_DICT_2 = 2, ///< Compression using dictionary 2
	DZCOBS_RESERVED			= 3, ///< For future uses
} eDZCOBS_encoding;

typedef struct s_DZRCOB_ctx sDZCOBS_ctx;

typedef eDZCOBS_ret ( *dzcobs_encode_inc_funcPtr )( sDZCOBS_ctx *aCtx, const uint8_t *aSrcBuf, size_t aSrcBufSize );

struct s_DZRCOB_ctx
{
	uint8_t *pDst;		 ///< Initial destiny pointer
	uint8_t *pCodeDst; ///< Destiny pointer to put the code
	uint8_t *pCurDst;	 ///< Current destiny pointer
	uint8_t *pDstEnd;	 ///< Last position pointer, 1 position outside buffer range

	uint8_t code;			 ///< Current code
	uint8_t hashsum;	 ///< Current sum of DZCOBS_HASH8
	uint8_t user6bits; ///< User application 6 bits, cannot be 0, so must be 1..63 (right aligned)

	bool isLastCodeDictionary;

	const sDICT_ctx *pDict[DZCOBS_DICT_N];

	dzcobs_encode_inc_funcPtr encFunc;

	eDZCOBS_encoding encoding;
};

#define DZCOBS_ONE_BYTE_OVERHEAD_EVERY ( 127 )
#define Z_DZCOBS_DIV_ROUND_UP( n, d ) ( ( ( n ) + ( d ) - 1 ) / ( d ) )
#define DZCOBS_MAX_OVERHEAD( size ) Z_DZCOBS_DIV_ROUND_UP( ( size ), DZCOBS_ONE_BYTE_OVERHEAD_EVERY )
#define DZCOBS_MAX_ENCODED_SIZE( size ) ( ( size ) + DZCOBS_MAX_OVERHEAD( ( size ) ) + ( ( size ) == 0 ) )

enum
{
	DZCOBS_HASH_VALUE_WHEN_CRC_IS_ZERO = ( 0xFF )
};

enum
{
	DZCOBS_CODE_JUMP_DICTIONARY = ( 0x7F ),
	DZCOBS_DICTIONARY_BITMASK		= ( 0x80 ),
	DZCOBS_CODE_JUMP_PLAIN			= ( 0xFF )
};

// Declarations
// /////////////////////////////////////////////////////////////////////////////

/**
 * @brief Set the pointer to an existent created dictionary context
 *
 * @param aCtx The encoding context.
 * @param aDictCtx The dictionary context previous created
 * @param aDictEncoding must be DZCOBS_USING_DICT_1 or DZCOBS_USING_DICT_2
 * @return eDZCOBS_ret
 */
eDZCOBS_ret dzcobs_encode_set_dictionary( sDZCOBS_ctx *aCtx,
																					const sDICT_ctx *aDictCtx,
																					eDZCOBS_encoding aDictEncoding );

/**
 * @brief Begin an incremental encoding of data
 *
 * @param aCtx Context to be initialized
 * @param aEncoding The desired encoding type for this frame
 * @param aDstBuf Destiny buffer
 * @param aDstBufSize Destiny buffer size
 * @return eRCOBS_ret
 */
eDZCOBS_ret dzcobs_encode_inc_begin( sDZCOBS_ctx *aCtx,
																		 eDZCOBS_encoding aEncoding,
																		 uint8_t *aDstBuf,
																		 size_t aDstBufSize );

/**
 * @brief Add the data to encoding
 *
 * @param aCtx Context in use
 * @param aSrcBuf Source buffer of data to add
 * @param aSrcBufSize Size of source buffer
 * @return eRCOBS_ret
 */
eDZCOBS_ret dzcobs_encode_inc( sDZCOBS_ctx *aCtx, const uint8_t *aSrcBuf, size_t aSrcBufSize );

/**
 * @brief Finalize the encoding. It does not add the 0 to the end of buffer. You
 * may add it if you want.
 *
 * @param aCtx Context in use
 * @param aOutSizeEncoded Size of encoded data
 * @return eRCOBS_ret
 */
eDZCOBS_ret dzcobs_encode_inc_end( sDZCOBS_ctx *aCtx, size_t *aOutSizeEncoded );

#ifdef __cplusplus
}
#endif

#endif

// EOF
// /////////////////////////////////////////////////////////////////////////////
