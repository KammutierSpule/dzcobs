// /////////////////////////////////////////////////////////////////////////////
///	@file dzcobs_decode.c
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
#include <dzcobs/dzcobs_decode.h>
#include <stdbool.h>
#include "dzcobs/dzcobs.h"

// Definitions
// /////////////////////////////////////////////////////////////////////////////

// Implementation
// /////////////////////////////////////////////////////////////////////////////
static eDZCOBS_ret dzcobs_decode_plain( const sDZCOBS_decodectx *aDecodeCtx, size_t *aOutDecodedLen )
{
	// Assume input parameters and conditions are validated

	const uint8_t *pReadEncoded		 = aDecodeCtx->srcBufEncoded;
	const uint8_t *pReadEncodedEnd = aDecodeCtx->srcBufEncoded + aDecodeCtx->srcBufEncodedLen -
																	 2; // remove userbits and hash8

	uint8_t *pDecoded					 = aDecodeCtx->dstBufDecoded;
	const uint8_t *pDecodedEnd = aDecodeCtx->dstBufDecoded + aDecodeCtx->dstBufDecodedSize;

	while( pReadEncoded < pReadEncodedEnd )
	{
		uint8_t code = *pReadEncoded++;

		if( code == 0 )
		{
			return DZCOBS_RET_ERR_BAD_ENCODED_PAYLOAD;
		}

		code--;

		const size_t remain_output_size = (size_t)( pDecodedEnd - pDecoded );

		if( code > remain_output_size )
		{
			return DZCOBS_RET_ERR_WRITE_OVERFLOW;
		}

		const size_t remain_read_size = (size_t)( pReadEncodedEnd - pReadEncoded );

		if( code > remain_read_size )
		{
			return DZCOBS_RET_ERR_READ_OVERFLOW;
		}

		for( size_t i = code; i != 0; i-- )
		{
			const uint8_t src_byte = *pReadEncoded++;

			if( src_byte == 0 )
			{
				return DZCOBS_RET_ERR_BAD_ENCODED_PAYLOAD;
			}

			*pDecoded++ = src_byte;
		}

		if( pReadEncoded >= pReadEncodedEnd )
		{
			break;
		}

		if( code != ( DZCOBS_CODE_JUMP_PLAIN - 1 ) )
		{
			*pDecoded++ = 0;
		}
	}

	*aOutDecodedLen = (size_t)( pDecoded - aDecodeCtx->dstBufDecoded );

	return DZCOBS_RET_SUCCESS;
}

static eDZCOBS_ret dzcobs_decode_dictionary( const sDZCOBS_decodectx *aDecodeCtx,
																						 size_t *aOutDecodedLen,
																						 const sDICT_ctx *aDict )
{
	// Assume input parameters and conditions are validated

	const uint8_t *pReadEncoded		 = aDecodeCtx->srcBufEncoded;
	const uint8_t *pReadEncodedEnd = aDecodeCtx->srcBufEncoded + aDecodeCtx->srcBufEncodedLen -
																	 2; // remove userbits and hash8

	uint8_t *pDecoded					 = aDecodeCtx->dstBufDecoded;
	const uint8_t *pDecodedEnd = aDecodeCtx->dstBufDecoded + aDecodeCtx->dstBufDecodedSize;

	bool isPreviousCodeDictionary = false;
	bool isToPlaceZero						= false;

	while( pReadEncoded < pReadEncodedEnd )
	{
		uint8_t code = *pReadEncoded++;

		if( code == 0 )
		{
			return DZCOBS_RET_ERR_BAD_ENCODED_PAYLOAD;
		}

		size_t remain_output_size = (size_t)( pDecodedEnd - pDecoded );

		if( ( remain_output_size == 0 ) && ( code != 1 ) && ( !isPreviousCodeDictionary ) )
		{
			return DZCOBS_RET_ERR_WRITE_OVERFLOW;
		}

		if( code >= DZCOBS_DICTIONARY_BITMASK )
		{
			isToPlaceZero = false;

			const uint8_t dictIdx = ( code & ~DZCOBS_DICTIONARY_BITMASK );

			uint8_t wordSize = 0;

			const uint8_t *dictionary_word = dzcobs_dictionary_get( aDict, dictIdx, &wordSize );

			if( dictionary_word == NULL )
			{
				return DZCOBS_RET_ERR_WORD_NOT_FOUND_ON_DICTIONARY;
			}

			if( remain_output_size < wordSize )
			{
				return DZCOBS_RET_ERR_WRITE_OVERFLOW;
			}

			while( wordSize-- )
			{
				*pDecoded++ = *dictionary_word++;
			}

			if( pReadEncoded >= pReadEncodedEnd )
			{
				break;
			}

			continue;
		}

		if( isToPlaceZero )
		{
			isToPlaceZero = false;

			*pDecoded++ = 0;
			remain_output_size--;
		}

		isPreviousCodeDictionary = false;

		code--;

		if( code > remain_output_size )
		{
			return DZCOBS_RET_ERR_WRITE_OVERFLOW;
		}

		const size_t remain_read_size = (size_t)( pReadEncodedEnd - pReadEncoded );

		if( code > remain_read_size )
		{
			return DZCOBS_RET_ERR_READ_OVERFLOW;
		}

		for( size_t i = code; i != 0; i-- )
		{
			const uint8_t src_byte = *pReadEncoded++;

			if( src_byte == 0 )
			{
				return DZCOBS_RET_ERR_BAD_ENCODED_PAYLOAD;
			}

			*pDecoded++ = src_byte;
		}

		if( pReadEncoded >= pReadEncodedEnd )
		{
			break;
		}

		if( code == 0 )
		{
			*pDecoded++		= 0;
			isToPlaceZero = false;
		}
		else
		{
			if( code != ( DZCOBS_CODE_JUMP_DICTIONARY - 1 ) )
			{
				isToPlaceZero = true;
			}
		}
	}

	*aOutDecodedLen = (size_t)( pDecoded - aDecodeCtx->dstBufDecoded );

	return DZCOBS_RET_SUCCESS;
}

eDZCOBS_ret dzcobs_decode( const sDZCOBS_decodectx *aDecodeCtx,
													 size_t *aOutDecodedLen,
													 uint8_t *aOutUser6bitDataRightAlgn )
{
	if( ( !aDecodeCtx ) || ( !aDecodeCtx->srcBufEncoded ) || ( !aDecodeCtx->dstBufDecoded ) || ( !aOutDecodedLen ) ||
			( aDecodeCtx->dstBufDecodedSize == 0 ) || ( aDecodeCtx->srcBufEncodedLen < 3 ) )
	{
		return DZCOBS_RET_ERR_BAD_ARG;
	}

	const uint8_t *pReadEncoded = aDecodeCtx->srcBufEncoded + aDecodeCtx->srcBufEncodedLen - 1;

	const uint8_t receivedChecksum8		 = *pReadEncoded--;
	const uint8_t receivedUserEncoding = *pReadEncoded--;

	if( ( receivedChecksum8 == 0 ) || ( receivedUserEncoding == 0 ) )
	{
		return DZCOBS_RET_ERR_BAD_ENCODED_PAYLOAD;
	}

	uint8_t checksum8 = 0;

	const uint8_t *pData		= aDecodeCtx->srcBufEncoded;
	const uint8_t *pDataEnd = aDecodeCtx->srcBufEncoded + aDecodeCtx->srcBufEncodedLen - 1; // -1 removed CRC

	while( pData < pDataEnd )
	{
		const uint8_t value = *pData++;
		checksum8 += DZCOBS_HASH8( value );
	}

	if( ( ( checksum8 != 0 ) && ( checksum8 != receivedChecksum8 ) ) ||
			( ( checksum8 == 0 ) && ( receivedChecksum8 != DZCOBS_HASH_VALUE_WHEN_CRC_IS_ZERO ) ) )
	{
		return DZCOBS_RET_ERR_CRC;
	}

	// Get and validate encoding type
	const eDZCOBS_encoding encoding = (eDZCOBS_encoding)( receivedUserEncoding & 0x03 );

	eDZCOBS_ret ret = DZCOBS_RET_ERR_BAD_ENCODED_PAYLOAD;

	switch( encoding )
	{
	case DZCOBS_PLAIN:
		ret = dzcobs_decode_plain( aDecodeCtx, aOutDecodedLen );
		break;
	// [[fallthrough]]
	case DZCOBS_USING_DICT_1:
	case DZCOBS_USING_DICT_2:
	{
		const sDICT_ctx *pDict = aDecodeCtx->pDict[encoding - DZCOBS_USING_DICT_1];

		if( pDict == NULL )
		{
			return DZCOBS_RET_ERR_NO_DICTIONARY_TO_DECODE;
		}

		ret = dzcobs_decode_dictionary( aDecodeCtx, aOutDecodedLen, pDict );
	}
	break;

	case DZCOBS_RESERVED:
	default:
		break;
	}

	if( ret == DZCOBS_RET_SUCCESS )
	{
		*aOutUser6bitDataRightAlgn = ( receivedUserEncoding >> 2 ) & 0x3F;
	}

	return ret;
}

// EOF
// /////////////////////////////////////////////////////////////////////////////
