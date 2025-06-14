// /////////////////////////////////////////////////////////////////////////////
///	@file dzcobs.cpp
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

// Includes
// /////////////////////////////////////////////////////////////////////////////
#include <dzcobs/dzcobs.h>
#include <stdbool.h>
#include "dzcobs/dzcobs_dictionary.h"
#include "dzcobs_assert.h"

// Definitions
// /////////////////////////////////////////////////////////////////////////////
static eDZCOBS_ret dzcobs_encode_inc_plain( sDZCOBS_ctx *aCtx, const uint8_t *aSrcBuf, size_t aSrcBufSize );
static eDZCOBS_ret dzcobs_encode_inc_dictionary( sDZCOBS_ctx *aCtx, const uint8_t *aSrcBuf, size_t aSrcBufSize );

// Implementation
// /////////////////////////////////////////////////////////////////////////////

eDZCOBS_ret dzcobs_encode_set_dictionary( sDZCOBS_ctx *aCtx, const sDICT_ctx *aDictCtx, eDZCOBS_encoding aDictEncoding )
{
	if( ( !aCtx ) || ( !aDictCtx ) ||
			( !( ( aDictEncoding == DZCOBS_USING_DICT_1 ) || ( aDictEncoding == DZCOBS_USING_DICT_2 ) ) ) )
	{
		return DZCOBS_RET_ERR_BAD_ARG;
	}

	const uint8_t idx = (uint8_t)( aDictEncoding - DZCOBS_USING_DICT_1 );
	aCtx->pDict[idx]	= aDictCtx;

	return DZCOBS_RET_SUCCESS;
}

eDZCOBS_ret dzcobs_encode_inc_begin( sDZCOBS_ctx *aCtx,
																		 eDZCOBS_encoding aEncoding,
																		 uint8_t *aDstBuf,
																		 size_t aDstBufSize )
{
	if( ( !aCtx ) || ( !aDstBuf ) || ( aDstBufSize < 2 ) )
	{
		return DZCOBS_RET_ERR_BAD_ARG;
	}

	if( ( ( aEncoding == DZCOBS_USING_DICT_1 ) && ( aCtx->pDict[0] == NULL ) ) ||
			( ( aEncoding == DZCOBS_USING_DICT_2 ) && ( aCtx->pDict[1] == NULL ) ) )
	{
		return DZCOBS_RET_ERR_BAD_ARG;
	}

	aCtx->pDst		 = aDstBuf;
	aCtx->pCodeDst = aDstBuf;
	aCtx->pCurDst	 = aDstBuf + 1;
	aCtx->pDstEnd	 = aDstBuf + aDstBufSize;
	aCtx->code		 = 1;
	aCtx->hashsum	 = 0;

	aCtx->isLastCodeDictionary = false;

	aCtx->encoding = aEncoding;

	switch( aEncoding )
	{
	case DZCOBS_PLAIN:
		aCtx->encFunc = dzcobs_encode_inc_plain;
		break;

	case DZCOBS_USING_DICT_1:
	case DZCOBS_USING_DICT_2:
		aCtx->encFunc = dzcobs_encode_inc_dictionary;
		break;
	case DZCOBS_RESERVED:
	default:
		aCtx->encFunc = NULL;
		break;
	}

	DZCOBS_ASSERT( aCtx->encFunc != NULL );

	return DZCOBS_RET_SUCCESS;
}

eDZCOBS_ret dzcobs_encode_inc_end( sDZCOBS_ctx *aCtx, size_t *aOutSizeEncoded )
{
	if( ( !aCtx ) || ( !aOutSizeEncoded ) )
	{
		return DZCOBS_RET_ERR_BAD_ARG;
	}

	if( ( aCtx->pCurDst + 1 ) > aCtx->pDstEnd )
	{
		return DZCOBS_RET_ERR_WRITE_OVERFLOW;
	}

	if( aCtx->user6bits == 0 )
	{
		return DZCOBS_RET_ERR_INVALID_USER6BITS;
	}

	if( aCtx->isLastCodeDictionary )
	{
		DZCOBS_ASSERT( ( aCtx->pCodeDst + 1 ) == aCtx->pCurDst );
		aCtx->pCurDst--;
	}
	else
	{
		const uint8_t code = aCtx->code;
		aCtx->hashsum += DZCOBS_HASH8( code );
		*aCtx->pCodeDst = code;
	}

	// Add (tail) header info

	// User 6 bits and encoding info
	const uint8_t encodingByte = (uint8_t)( aCtx->user6bits << 2 ) | ( (uint8_t)aCtx->encoding & 0x03 );

	aCtx->hashsum += DZCOBS_HASH8( encodingByte );

	*aCtx->pCurDst++ = encodingByte;

	// Final Hash
	const uint8_t finalHash = aCtx->hashsum;

	*aCtx->pCurDst++ = ( finalHash == 0x00 ) ? DZCOBS_HASH_VALUE_WHEN_CRC_IS_ZERO : finalHash; // Avoid zero ending CRC.

	// Calc encoded size
	*aOutSizeEncoded = (size_t)( aCtx->pCurDst - aCtx->pDst );

	aCtx->encFunc = NULL;

	return DZCOBS_RET_SUCCESS;
}

eDZCOBS_ret dzcobs_encode_inc( sDZCOBS_ctx *aCtx, const uint8_t *aSrcBuf, size_t aSrcBufSize )
{
	if( ( !aCtx ) || ( !aSrcBuf ) )
	{
		return DZCOBS_RET_ERR_BAD_ARG;
	}

	if( aCtx->encFunc == NULL )
	{
		return DZCOBS_RET_ERR_NOTINITIALIZED;
	}

	if( aSrcBufSize == 0 )
	{
		return DZCOBS_RET_SUCCESS;
	}

	return aCtx->encFunc( aCtx, aSrcBuf, aSrcBufSize );
}

eDZCOBS_ret dzcobs_encode_inc_plain( sDZCOBS_ctx *aCtx, const uint8_t *aSrcBuf, size_t aSrcBufSize )
{
	DZCOBS_ASSERT( aCtx != NULL );
	DZCOBS_ASSERT( aSrcBuf != NULL );
	DZCOBS_ASSERT( aSrcBufSize > 0 );

	uint8_t code			= aCtx->code;
	uint8_t *pCodeDst = aCtx->pCodeDst;
	uint8_t *pCurDst	= aCtx->pCurDst;
	uint8_t hashsum		= aCtx->hashsum;

	while( aSrcBufSize )
	{
		aSrcBufSize--;

		const uint8_t src_byte = *aSrcBuf++;

		if( src_byte == 0 )
		{
			hashsum += DZCOBS_HASH8( code );
			*pCodeDst = code;
			pCodeDst	= pCurDst++;
			code			= 1;
		}
		else
		{
			hashsum += DZCOBS_HASH8( src_byte );
			*pCurDst++ = src_byte;
			code++;

			if( ( code == DZCOBS_CODE_JUMP_PLAIN ) && ( aSrcBufSize ) )
			{
				hashsum += DZCOBS_HASH8( code );

				*pCodeDst = code;
				pCodeDst	= pCurDst++;
				code			= 1;
			}
		}
	}

	aCtx->code		 = code;
	aCtx->pCodeDst = pCodeDst;
	aCtx->pCurDst	 = pCurDst;
	aCtx->hashsum	 = hashsum;

	return DZCOBS_RET_SUCCESS;
}

eDZCOBS_ret dzcobs_encode_inc_dictionary( sDZCOBS_ctx *aCtx, const uint8_t *aSrcBuf, size_t aSrcBufSize )
{
	DZCOBS_ASSERT( aCtx != NULL );
	DZCOBS_ASSERT( aSrcBuf != NULL );
	DZCOBS_ASSERT( aSrcBufSize > 0 );
	DZCOBS_ASSERT( ( aCtx->encoding == DZCOBS_USING_DICT_1 ) || ( aCtx->encoding == DZCOBS_USING_DICT_2 ) );

	uint8_t code			= aCtx->code;
	uint8_t *pCodeDst = aCtx->pCodeDst;
	uint8_t *pCurDst	= aCtx->pCurDst;
	uint8_t hashsum		= aCtx->hashsum;

	const sDICT_ctx *pDict = aCtx->pDict[aCtx->encoding - DZCOBS_USING_DICT_1];

	while( aSrcBufSize )
	{
		size_t sizeOfKeyFound = 0;

		uint8_t foundIdx = dzcobs_dictionary_search( pDict, aSrcBuf, aSrcBufSize, &sizeOfKeyFound );

		if( foundIdx )
		{
			DZCOBS_ASSERT( sizeOfKeyFound > 0 );
			DZCOBS_ASSERT( sizeOfKeyFound <= aSrcBufSize );

			foundIdx -= 1; // remove base index
			const uint8_t dictEntry = DZCOBS_DICTIONARY_BITMASK | foundIdx;

			hashsum += DZCOBS_HASH8( dictEntry );

			if( code != 1 )
			{
				hashsum += DZCOBS_HASH8( code );
				*pCodeDst = code;
				pCodeDst	= pCurDst++;
				code			= 1;
			}

			*pCodeDst = dictEntry;
			pCodeDst	= pCurDst++;

			// advance keyword
			aSrcBufSize -= sizeOfKeyFound;
			aSrcBuf += sizeOfKeyFound;

			aCtx->isLastCodeDictionary = true;
			continue;
		}

		aCtx->isLastCodeDictionary = false;

		// Continue with regular plain encoding
		aSrcBufSize--;

		const uint8_t src_byte = *aSrcBuf++;

		if( src_byte == 0 )
		{
			hashsum += DZCOBS_HASH8( code );
			*pCodeDst = code;
			pCodeDst	= pCurDst++;
			code			= 1;
		}
		else
		{
			hashsum += DZCOBS_HASH8( src_byte );
			*pCurDst++ = src_byte;
			code++;

			if( ( code == DZCOBS_CODE_JUMP_DICTIONARY ) && ( aSrcBufSize ) )
			{
				hashsum += DZCOBS_HASH8( code );

				*pCodeDst = code;
				pCodeDst	= pCurDst++;
				code			= 1;
			}
		}
	}

	aCtx->code		 = code;
	aCtx->pCodeDst = pCodeDst;
	aCtx->pCurDst	 = pCurDst;
	aCtx->hashsum	 = hashsum;

	return DZCOBS_RET_SUCCESS;
}

// EOF
// /////////////////////////////////////////////////////////////////////////////
