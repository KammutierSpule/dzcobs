// /////////////////////////////////////////////////////////////////////////////
///	@file test_dzcobs.cpp
///	@brief Unit tests for DZCOBS
///
///	@par  Plataform Target:	Tests
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
#include <CppUTest/TestHarness.h>
#include <CppUTest/UtestMacros.h>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dzcobs/dzcobs.h>
#include <dzcobs/dzcobs_decode.h>

// Definitions
// /////////////////////////////////////////////////////////////////////////////

#define UTEST_GUARD_BYTE ( 0xEE )
#define UTEST_GUARD_SIZE ( 4 )
#define UTEST_ENCODED_DECODED_DATA_MAX_SIZE ( 1024 )

// Setup
// /////////////////////////////////////////////////////////////////////////////

// clang-format off
// NOLINTBEGIN
/// Dictionary string, descendent order, null terminated
static const char s_TEST_Dictionary1[] =
	DICT_ADD_WORD(2, "\x01\x01")
	DICT_ADD_WORD(3, "\x02\x00\x02")
	DICT_ADD_WORD(4, "\x03\x00\x00\x03")
	DICT_ADD_WORD(5, "\x04\x00\x00\x00\x04")
;
// NOLINTEND
// clang-format on

const size_t s_TEST_Dictionary1_size = sizeof( s_TEST_Dictionary1 );

// clang-format off
// NOLINTBEGIN
TEST_GROUP( DZCOBS ){
	void setup()
	{
		buffer = new uint8_t[UTEST_GUARD_SIZE + UTEST_ENCODED_DECODED_DATA_MAX_SIZE + UTEST_GUARD_SIZE];
	}

	void teardown()
	{
		delete [] buffer;
		buffer = nullptr;
	}

	uint8_t *buffer;
};
// NOLINTEND
// clang-format on

void debug_dump_buffer( const uint8_t *aBuffer, size_t aBufferSize )
{
	if( ( !aBuffer ) || ( !aBufferSize ) )
	{
		return;
	}

	printf( "(%lu) {", aBufferSize );

	while( aBufferSize > 0 )
	{
		aBufferSize--;
		printf( "0x%02X ", *aBuffer++ );
	}

	printf( "}\n" );
}

// Test data
// /////////////////////////////////////////////////////////////////////////////

// clang-format off
// NOLINTBEGIN
static uint8_t s_dzcobs_datatest_plainencoding[] = {
	// 0
	1, 'A',	// decoded
	2 + DZCOBS_FRAME_HEADER_SIZE, 0x02, 'A', 0xFC /*Encoding*/, 0x54 /*CHECKSUM*/,	// encoded
	// 1
	4, 'A', 'B', 'C', 'D',	// decoded
	5 + DZCOBS_FRAME_HEADER_SIZE, 0x05, 'A', 'B', 'C', 'D', 0xFC /*Encoding*/, 0x9C /*CHECKSUM*/,	// encoded
	// 2
	4, 'A', 'B', 0x00, 'C',	// decoded
	5 + DZCOBS_FRAME_HEADER_SIZE, 0x03, 'A', 'B', 0x02, 'C', 0xFC /*Encoding*/, 0x74 /*CHECKSUM*/,	// encoded
	// 3
	7, 'A', 0x00, 0x00, 0x00, 'B', 'C', 'D',	// decoded
	8 + DZCOBS_FRAME_HEADER_SIZE, 0x02, 'A', 0x01, 0x01, 0x04, 'B', 'C', 'D', 0xFC /*Encoding*/, 0x7B /*CHECKSUM*/,	// encoded
	// 4
	1, 0x00,	// decoded
	2 + DZCOBS_FRAME_HEADER_SIZE, 0x01, 0x01, 0xFC /*Encoding*/, 0x37 /*CHECKSUM*/,	// encoded
	// 5
	2, 0x00, 0x00,	// decoded
	3 + DZCOBS_FRAME_HEADER_SIZE, 0x01, 0x01, 0x01, 0xFC /*Encoding*/, 0xDC /*CHECKSUM*/,	// encoded
	// 6
	3, 0x00, 0x11, 0x00,	// decoded
	4 + DZCOBS_FRAME_HEADER_SIZE, 0x01, 0x02, 0x11, 0x01, 0xFC /*Encoding*/, 0xC8 /*CHECKSUM*/,	// encoded
};

#define TEST_USERBITS (0x3F)

// Encoded is using s_TEST_Dictionary1
static uint8_t s_dzcobs_datatest_dictionary[] = {
	// 0
	2, 0x01, 0x01,				// decoded
	1 + DZCOBS_FRAME_HEADER_SIZE, 0x80 + 0, ( TEST_USERBITS << 2 ) | 1 /*Encoding*/, 0x84 /*CHECKSUM*/,	// encoded
	// 1
	4, 0x01, 0x01, 0x01, 0x01,				// decoded
	2 + DZCOBS_FRAME_HEADER_SIZE, 0x80 + 0, 0x80 + 0, ( TEST_USERBITS << 2 ) | 1 /*Encoding*/, 0x74 /*CHECKSUM*/,	// encoded
	// 2
	5, 0x12, 0x01, 0x01, 0x01, 0x01,				// decoded
	4 + DZCOBS_FRAME_HEADER_SIZE, 0x02, 0x12, 0x80 + 0, 0x80 + 0, ( TEST_USERBITS << 2 ) | 1 /*Encoding*/, 0x12 /*CHECKSUM*/,	// encoded
	// 3
	6, 0x12, 0x01, 0x01, 0x23, 0x01, 0x01,				// decoded
	6 + DZCOBS_FRAME_HEADER_SIZE, 0x02, 0x12, 0x80 + 0, 0x02, 0x23, 0x80 + 0, ( TEST_USERBITS << 2 ) | 1 /*Encoding*/, 0x93 /*CHECKSUM*/,	// encoded
	// 4
	7, 0x12, 0x01, 0x01, 0x23, 0x02, 0x00, 0x02,				// decoded
	6 + DZCOBS_FRAME_HEADER_SIZE, 0x02, 0x12, 0x80 + 0, 0x02, 0x23, 0x80 + 1, ( TEST_USERBITS << 2 ) | 1 /*Encoding*/, 0x38 /*CHECKSUM*/,	// encoded
	// 5
	7, 0x12, 0x01, 0x01, 0x00, 0x02, 0x00, 0x02,				// decoded
	5 + DZCOBS_FRAME_HEADER_SIZE, 0x02, 0x12, 0x80 + 0, 0x01, 0x80 + 1, ( TEST_USERBITS << 2 ) | 1 /*Encoding*/, 0x5C /*CHECKSUM*/,	// encoded
	// 6
	1, 0x00,				// decoded
	2 + DZCOBS_FRAME_HEADER_SIZE, 0x01, 0x01, ( TEST_USERBITS << 2 ) | 1 /*Encoding*/, 0xDE /*CHECKSUM*/,	// encoded
	// 7
	2, 0x00, 0x00,	// decoded
	3 + DZCOBS_FRAME_HEADER_SIZE, 0x01, 0x01, 0x01, ( TEST_USERBITS << 2 ) | 1 /*Encoding*/, 0x83 /*CHECKSUM*/,	// encoded
	// 8
	9, 0x12, 0x01, 0x01, 0x00, 0x02, 0x00, 0x02, 0x12, 0x00,		// decoded
	8 + DZCOBS_FRAME_HEADER_SIZE, 0x02, 0x12, 0x80 + 0, 0x01, 0x80 + 1, 0x02, 0x12, 0x01, ( TEST_USERBITS << 2 ) | 1 /*Encoding*/, 0x9F /*CHECKSUM*/,	// encoded
	// 9
	4, 0x01, 0x01, 0x12, 0x00,		// decoded
	4 + DZCOBS_FRAME_HEADER_SIZE, 0x80 + 0, 0x02, 0x12, 0x01, ( TEST_USERBITS << 2 ) | 1 /*Encoding*/, 0xC7 /*CHECKSUM*/,	// encoded
	// 10
	4, 'A', 'B', 0x00, 'C',		// decoded
	5 + DZCOBS_FRAME_HEADER_SIZE, 0x03, 'A', 'B', 0x02, 'C', ( TEST_USERBITS << 2 ) | 1 /*Encoding*/, 0x1B /*CHECKSUM*/,	// encoded
	// 11
	5, 'A', 0x00, 'B', 0x00, 'C',		// decoded
	6 + DZCOBS_FRAME_HEADER_SIZE, 0x02, 'A', 0x02, 'B', 0x02, 'C', ( TEST_USERBITS << 2 ) | 1 /*Encoding*/, 0xBC /*CHECKSUM*/,	// encoded
	// 12
	4, 0x01, 0x01, 0x00, 'C',		// decoded
	4 + DZCOBS_FRAME_HEADER_SIZE, 0x80 + 0, 0x01, 0x02, 'C', ( TEST_USERBITS << 2 ) | 1 /*Encoding*/, 0xDE /*CHECKSUM*/,	// encoded
	// 13
	3, 0x01, 0x01, 'C',		// decoded
	3 + DZCOBS_FRAME_HEADER_SIZE, 0x80 + 0, 0x02, 'C', ( TEST_USERBITS << 2 ) | 1 /*Encoding*/, 0x39 /*CHECKSUM*/,	// encoded
	// 14
	6, 0x01, 0x01, 0x00, 'A', 0x00, 'B',		// decoded
	6 + DZCOBS_FRAME_HEADER_SIZE, 0x80 + 0, 0x01, 0x02, 'A', 0x02, 'B', ( TEST_USERBITS << 2 ) | 1 /*Encoding*/, 0x9C /*CHECKSUM*/,	// encoded
	// 15
	8, 0x01, 0x01, 0x00, 'A', 0x01, 0x01, 0x00, 'B',		// decoded
	8 + DZCOBS_FRAME_HEADER_SIZE, 0x80 + 0, 0x01, 0x02, 'A', 0x80 + 0, 0x01, 0x02, 'B', ( TEST_USERBITS << 2 ) | 1 /*Encoding*/, 0x31 /*CHECKSUM*/,	// encoded
	// 16
	5, 0x01, 0x01, 0x00, 0x01, 0x01,		// decoded
	3 + DZCOBS_FRAME_HEADER_SIZE, 0x80 + 0, 0x01, 0x80 + 0, ( TEST_USERBITS << 2 ) | 1 /*Encoding*/, 0x19 /*CHECKSUM*/,	// encoded
	// 17
	4, 0x01, 0x01, 'C', 0x00,		// decoded
	4 + DZCOBS_FRAME_HEADER_SIZE, 0x80 + 0, 0x02, 'C', 0x01, ( TEST_USERBITS << 2 ) | 1 /*Encoding*/, 0xDE /*CHECKSUM*/,	// encoded
	// 18
	6, 0x01, 0x01, 'C', 0x01, 0x01, 0x00,		// decoded
	6 + DZCOBS_FRAME_HEADER_SIZE, 0x80 + 0, 0x02, 'C', 0x80 + 0, 0x01, 0x01, ( TEST_USERBITS << 2 ) | 1 /*Encoding*/, 0x73 /*CHECKSUM*/,	// encoded
	// 19
	7, 0x00, 0x01, 0x01, 0x00, 0x01, 0x01, 0x00,		// decoded
	6 + DZCOBS_FRAME_HEADER_SIZE, 0x01, 0x80 + 0, 0x01, 0x80 + 0, 0x01, 0x01, ( TEST_USERBITS << 2 ) | 1 /*Encoding*/, 0x08 /*CHECKSUM*/,	// encoded
};

// NOLINTEND
// clang-format on

// Tests
// /////////////////////////////////////////////////////////////////////////////

// NOLINTBEGIN
TEST( DZCOBS, MacroEncodeMax )
// NOLINTEND
{
	// DZCOBS requires a minimum of 1 byte overhead,
	// and a maximum of ⌈n/DZCOBS_ONE_BYTE_OVERHEAD_EVERY⌉ bytes for n data bytes
	// (one byte in DZCOBS_ONE_BYTE_OVERHEAD_EVERY, rounded up)
	CHECK_EQUAL( 1, DZCOBS_MAX_ENCODED_SIZE( 0 ) );
	CHECK_EQUAL( 1 + 1, DZCOBS_MAX_ENCODED_SIZE( 1 ) );
	CHECK_EQUAL( DZCOBS_ONE_BYTE_OVERHEAD_EVERY + 1, DZCOBS_MAX_ENCODED_SIZE( DZCOBS_ONE_BYTE_OVERHEAD_EVERY ) );
	CHECK_EQUAL( ( DZCOBS_ONE_BYTE_OVERHEAD_EVERY + 1 ) + 2,
							 DZCOBS_MAX_ENCODED_SIZE( DZCOBS_ONE_BYTE_OVERHEAD_EVERY + 1 ) );
	CHECK_EQUAL( ( DZCOBS_ONE_BYTE_OVERHEAD_EVERY * 2 ) + ( 1 * 2 ),
							 DZCOBS_MAX_ENCODED_SIZE( DZCOBS_ONE_BYTE_OVERHEAD_EVERY * 2 ) );
	CHECK_EQUAL( ( DZCOBS_ONE_BYTE_OVERHEAD_EVERY * 2 + 1 ) + ( 1 * 2 ),
							 DZCOBS_MAX_ENCODED_SIZE( DZCOBS_ONE_BYTE_OVERHEAD_EVERY * 2 ) + 1 );
}

// NOLINTBEGIN
TEST( DZCOBS, DecodePlainManual )
// NOLINTEND
{
	size_t idx									 = 0;
	const uint8_t *pDatatest		 = s_dzcobs_datatest_plainencoding;
	const uint8_t *pDatatest_end = s_dzcobs_datatest_plainencoding + sizeof( s_dzcobs_datatest_plainencoding );

	while( pDatatest < pDatatest_end )
	{
		const uint8_t decodeDataSize = *pDatatest++;
		const uint8_t *decodeData		 = pDatatest;
		pDatatest += decodeDataSize;

		const uint8_t encodedDataSize = *pDatatest++;
		const uint8_t *encodedData		= pDatatest;
		pDatatest += encodedDataSize;

		eDZCOBS_ret ret		= DZCOBS_RET_SUCCESS;
		size_t decodedLen = 0;

		static const uint32_t guard = ( UTEST_GUARD_BYTE << 24 ) | ( UTEST_GUARD_BYTE << 16 ) | ( UTEST_GUARD_BYTE << 8 ) |
																	( UTEST_GUARD_BYTE << 0 );

		memset( buffer, UTEST_GUARD_BYTE, UTEST_ENCODED_DECODED_DATA_MAX_SIZE + UTEST_GUARD_SIZE * 2 );

		sDZCOBS_decodectx decodeCtx;
		decodeCtx.srcBufEncoded			= encodedData;
		decodeCtx.srcBufEncodedLen	= encodedDataSize;
		decodeCtx.dstBufDecoded			= buffer + UTEST_GUARD_SIZE;
		decodeCtx.dstBufDecodedSize = decodeDataSize; // used to test limit of the buffer

		uint8_t user6bitDataRightAlgn = 0;

		ret = dzcobs_decode( &decodeCtx, &decodedLen, &user6bitDataRightAlgn );

		CHECK_EQUAL( DZCOBS_RET_SUCCESS, ret );
		CHECK_EQUAL( 0x3F, user6bitDataRightAlgn );
		CHECK_EQUAL( 0, memcmp( buffer, &guard, UTEST_GUARD_SIZE ) );
		CHECK_EQUAL( 0,
								 memcmp( buffer + UTEST_GUARD_SIZE + UTEST_ENCODED_DECODED_DATA_MAX_SIZE, &guard, UTEST_GUARD_SIZE ) );

		if( decodeDataSize != decodedLen )
		{
			printf( "encodedData " );
			debug_dump_buffer( encodedData, encodedDataSize );

			printf( "decodedData " );
			debug_dump_buffer( decodeData, decodeDataSize );

			printf( "dstBufDecoded " );
			debug_dump_buffer( decodeCtx.dstBufDecoded, decodedLen );
		}

		CHECK_EQUAL( decodeDataSize, decodedLen );
		CHECK_EQUAL( 0, memcmp( decodeData, decodeCtx.dstBufDecoded, decodedLen ) );
		idx++;
	}
}

// NOLINTBEGIN
TEST( DZCOBS, EncodePlainManual )
// NOLINTEND
{
	size_t idx									 = 0;
	const uint8_t *pDatatest		 = s_dzcobs_datatest_plainencoding;
	const uint8_t *pDatatest_end = s_dzcobs_datatest_plainencoding + sizeof( s_dzcobs_datatest_plainencoding );

	while( pDatatest < pDatatest_end )
	{
		const uint8_t decodeDataSize = *pDatatest++;
		const uint8_t *decodeData		 = pDatatest;
		pDatatest += decodeDataSize;

		const uint8_t encodedDataSize = *pDatatest++;
		const uint8_t *encodedData		= pDatatest;
		pDatatest += encodedDataSize;

		static const uint32_t guard = ( UTEST_GUARD_BYTE << 24 ) | ( UTEST_GUARD_BYTE << 16 ) | ( UTEST_GUARD_BYTE << 8 ) |
																	( UTEST_GUARD_BYTE << 0 );

		memset( buffer, UTEST_GUARD_BYTE, UTEST_ENCODED_DECODED_DATA_MAX_SIZE + UTEST_GUARD_SIZE * 2 );

		eDZCOBS_ret ret		= DZCOBS_RET_SUCCESS;
		size_t encodedLen = 0;
		sDZCOBS_ctx ctx;

		ret = dzcobs_encode_inc_begin( &ctx, DZCOBS_PLAIN, buffer + UTEST_GUARD_SIZE, UTEST_ENCODED_DECODED_DATA_MAX_SIZE );
		CHECK_EQUAL( DZCOBS_RET_SUCCESS, ret );

		ctx.user6bits = TEST_USERBITS;

		ret = dzcobs_encode_inc( &ctx, decodeData, decodeDataSize );
		CHECK_EQUAL( DZCOBS_RET_SUCCESS, ret );

		ret = dzcobs_encode_inc_end( &ctx, &encodedLen );
		CHECK_EQUAL( DZCOBS_RET_SUCCESS, ret );

		CHECK_EQUAL( encodedDataSize, encodedLen );
		CHECK_EQUAL( 0, memcmp( encodedData, buffer + UTEST_GUARD_SIZE, encodedDataSize ) );

		CHECK_EQUAL( 0, memcmp( buffer, &guard, UTEST_GUARD_SIZE ) );
		CHECK_EQUAL( 0, memcmp( buffer + UTEST_GUARD_SIZE + encodedDataSize, &guard, UTEST_GUARD_SIZE ) );
		idx++;
	}
}

// NOLINTBEGIN
TEST( DZCOBS, EncodeDictionaryManual )
// NOLINTEND
{
	size_t idx									 = 0;
	const uint8_t *pDatatest		 = s_dzcobs_datatest_dictionary;
	const uint8_t *pDatatest_end = s_dzcobs_datatest_dictionary + sizeof( s_dzcobs_datatest_dictionary );

	sDICT_ctx dictCtx;

	eDICT_ret ret = dzcobs_dictionary_init( &dictCtx, s_TEST_Dictionary1, s_TEST_Dictionary1_size );
	CHECK_EQUAL( DICT_RET_SUCCESS, ret );

	sDZCOBS_ctx ctx;
	memset( &ctx, 0, sizeof( sDZCOBS_ctx ) );

	dzcobs_encode_set_dictionary( &ctx, &dictCtx, DZCOBS_USING_DICT_1 );
	ctx.user6bits = TEST_USERBITS;

	while( pDatatest < pDatatest_end )
	{
		const uint8_t decodeDataSize = *pDatatest++;
		const uint8_t *decodeData		 = pDatatest;
		pDatatest += decodeDataSize;

		const uint8_t encodedDataSize = *pDatatest++;
		const uint8_t *encodedData		= pDatatest;
		pDatatest += encodedDataSize;

		static const uint32_t guard = ( UTEST_GUARD_BYTE << 24 ) | ( UTEST_GUARD_BYTE << 16 ) | ( UTEST_GUARD_BYTE << 8 ) |
																	( UTEST_GUARD_BYTE << 0 );

		memset( buffer, UTEST_GUARD_BYTE, UTEST_ENCODED_DECODED_DATA_MAX_SIZE + UTEST_GUARD_SIZE * 2 );

		CHECK_TRUE( encodedDataSize <= DZCOBS_MAX_ENCODED_SIZE( decodeDataSize ) + DZCOBS_FRAME_HEADER_SIZE );

		eDZCOBS_ret ret		= DZCOBS_RET_SUCCESS;
		size_t encodedLen = 0;

		ret = dzcobs_encode_inc_begin(
		 &ctx,
		 DZCOBS_USING_DICT_1,
		 buffer + UTEST_GUARD_SIZE,
		 std::max<uint8_t>( DZCOBS_MAX_ENCODED_SIZE( decodeDataSize ) + DZCOBS_FRAME_HEADER_SIZE,
												4 ) // Used to test the limit
		);
		CHECK_EQUAL( DZCOBS_RET_SUCCESS, ret );

		ret = dzcobs_encode_inc( &ctx, decodeData, decodeDataSize );
		CHECK_EQUAL( DZCOBS_RET_SUCCESS, ret );

		ret = dzcobs_encode_inc_end( &ctx, &encodedLen );
		CHECK_EQUAL( DZCOBS_RET_SUCCESS, ret );

		printf( "idx: %lu, encodedDataSize: %u, encodedLen: %lu\n", idx, encodedDataSize, encodedLen );
		printf( "deco:" );

		for( size_t i = 0; i < decodeDataSize; i++ )
		{
			printf( " 0x%02X, ", decodeData[i] );
		}
		printf( "\n" );
		printf( "good:" );

		for( size_t i = 0; i < encodedDataSize; i++ )
		{
			printf( " 0x%02X, ", encodedData[i] );
		}
		printf( "\n" );
		printf( "goot:" );

		for( size_t i = 0; i < encodedLen; i++ )
		{
			printf( " 0x%02X, ", buffer[i + UTEST_GUARD_SIZE] );
		}
		printf( "\n" );

		CHECK_EQUAL( encodedDataSize, encodedLen );
		CHECK_EQUAL( 0, memcmp( encodedData, buffer + UTEST_GUARD_SIZE, encodedDataSize ) );

		CHECK_EQUAL( 0, memcmp( buffer, &guard, UTEST_GUARD_SIZE ) );
		CHECK_EQUAL( 0, memcmp( buffer + UTEST_GUARD_SIZE + encodedDataSize, &guard, UTEST_GUARD_SIZE ) );
		idx++;
	}
}

// NOLINTBEGIN
TEST( DZCOBS, DecodedictionaryManual )
// NOLINTEND
{
	sDICT_ctx dictCtx;

	eDICT_ret ret = dzcobs_dictionary_init( &dictCtx, s_TEST_Dictionary1, s_TEST_Dictionary1_size );
	CHECK_EQUAL( DICT_RET_SUCCESS, ret );

	size_t idx									 = 0;
	const uint8_t *pDatatest		 = s_dzcobs_datatest_dictionary;
	const uint8_t *pDatatest_end = s_dzcobs_datatest_dictionary + sizeof( s_dzcobs_datatest_dictionary );

	while( pDatatest < pDatatest_end )
	{
		const uint8_t decodeDataSize = *pDatatest++;
		const uint8_t *decodeData		 = pDatatest;
		pDatatest += decodeDataSize;

		const uint8_t encodedDataSize = *pDatatest++;
		const uint8_t *encodedData		= pDatatest;
		pDatatest += encodedDataSize;

		eDZCOBS_ret ret		= DZCOBS_RET_SUCCESS;
		size_t decodedLen = 0;

		static const uint32_t guard = ( UTEST_GUARD_BYTE << 24 ) | ( UTEST_GUARD_BYTE << 16 ) | ( UTEST_GUARD_BYTE << 8 ) |
																	( UTEST_GUARD_BYTE << 0 );

		memset( buffer, UTEST_GUARD_BYTE, UTEST_ENCODED_DECODED_DATA_MAX_SIZE + UTEST_GUARD_SIZE * 2 );

		sDZCOBS_decodectx decodeCtx;
		decodeCtx.srcBufEncoded			= encodedData;
		decodeCtx.srcBufEncodedLen	= encodedDataSize;
		decodeCtx.dstBufDecoded			= buffer + UTEST_GUARD_SIZE;
		decodeCtx.dstBufDecodedSize = decodeDataSize; // used to test limit of the buffer
		decodeCtx.pDict[0]					= &dictCtx;

		uint8_t user6bitDataRightAlgn = 0;

		ret = dzcobs_decode( &decodeCtx, &decodedLen, &user6bitDataRightAlgn );

		CHECK_EQUAL( DZCOBS_RET_SUCCESS, ret );
		CHECK_EQUAL( 0x3F, user6bitDataRightAlgn );
		CHECK_EQUAL( 0, memcmp( buffer, &guard, UTEST_GUARD_SIZE ) );
		CHECK_EQUAL( 0,
								 memcmp( buffer + UTEST_GUARD_SIZE + UTEST_ENCODED_DECODED_DATA_MAX_SIZE, &guard, UTEST_GUARD_SIZE ) );
		CHECK_EQUAL( decodeDataSize, decodedLen );
		CHECK_EQUAL( 0, memcmp( decodeData, decodeCtx.dstBufDecoded, decodedLen ) );
		idx++;
	}
}

// NOLINTBEGIN
TEST( DZCOBS, DecodeInvalidArgs )
// NOLINTEND
{
	eDZCOBS_ret ret		= DZCOBS_RET_SUCCESS;
	size_t decodedLen = 0;

	uint8_t user6bitDataRightAlgn = 0;

	sDZCOBS_decodectx decodeCtx;
	decodeCtx.srcBufEncoded			= buffer;
	decodeCtx.srcBufEncodedLen	= 1;
	decodeCtx.dstBufDecoded			= buffer + UTEST_GUARD_SIZE;
	decodeCtx.dstBufDecodedSize = 1;

	ret = dzcobs_decode( &decodeCtx, &decodedLen, nullptr );
	CHECK_EQUAL( DZCOBS_RET_ERR_BAD_ARG, ret );

	ret = dzcobs_decode( &decodeCtx, &decodedLen, &user6bitDataRightAlgn );
	CHECK_EQUAL( DZCOBS_RET_ERR_BAD_ARG, ret );

	ret = dzcobs_decode( &decodeCtx, nullptr, &user6bitDataRightAlgn );
	CHECK_EQUAL( DZCOBS_RET_ERR_BAD_ARG, ret );

	ret = dzcobs_decode( nullptr, &decodedLen, &user6bitDataRightAlgn );
	CHECK_EQUAL( DZCOBS_RET_ERR_BAD_ARG, ret );
}

// NOLINTBEGIN
TEST( DZCOBS, EncodeBeginInvalidArgs )
// NOLINTEND
{
	eDZCOBS_ret ret = DZCOBS_RET_SUCCESS;
	sDZCOBS_ctx ctx;

	ret = dzcobs_encode_inc_begin( &ctx, DZCOBS_PLAIN, buffer, 0 );
	CHECK_EQUAL_TEXT( DZCOBS_RET_ERR_BAD_ARG, ret, "buffer length <2 must fail" );

	ret = dzcobs_encode_inc_begin( &ctx, DZCOBS_PLAIN, buffer, 1 );
	CHECK_EQUAL_TEXT( DZCOBS_RET_ERR_BAD_ARG, ret, "buffer length <2 must fail" );

	ret = dzcobs_encode_inc_begin( &ctx, DZCOBS_PLAIN, nullptr, 3 );
	CHECK_EQUAL_TEXT( DZCOBS_RET_ERR_BAD_ARG, ret, "NULL input must fail" );

	ret = dzcobs_encode_inc_begin( nullptr, DZCOBS_PLAIN, buffer, 3 );
	CHECK_EQUAL_TEXT( DZCOBS_RET_ERR_BAD_ARG, ret, "NULL input must fail" );
}

// NOLINTBEGIN
TEST( DZCOBS, EncodeIncInvalidArgs )
// NOLINTEND
{
	eDZCOBS_ret ret = DZCOBS_RET_SUCCESS;
	sDZCOBS_ctx ctx;

	ret = dzcobs_encode_inc( nullptr, buffer, 1 );
	CHECK_EQUAL_TEXT( DZCOBS_RET_ERR_BAD_ARG, ret, "NULL input must fail" );

	ret = dzcobs_encode_inc( &ctx, nullptr, 1 );
	CHECK_EQUAL_TEXT( DZCOBS_RET_ERR_BAD_ARG, ret, "NULL input must fail" );
}

// NOLINTBEGIN
TEST( DZCOBS, EncodeEndInvalidArgs )
// NOLINTEND
{
	eDZCOBS_ret ret = DZCOBS_RET_SUCCESS;
	sDZCOBS_ctx ctx;
	size_t sizeEncoded = 0;

	ret = dzcobs_encode_inc_end( &ctx, nullptr );
	CHECK_EQUAL_TEXT( DZCOBS_RET_ERR_BAD_ARG, ret, "NULL input must fail" );

	ret = dzcobs_encode_inc_end( nullptr, &sizeEncoded );
	CHECK_EQUAL_TEXT( DZCOBS_RET_ERR_BAD_ARG, ret, "NULL input must fail" );
}

// NOLINTBEGIN
TEST( DZCOBS, EncodeDecodeLongSequentialPlain )
// NOLINTEND
{
	static constexpr size_t MaxDecodedDataSize = UTEST_ENCODED_DECODED_DATA_MAX_SIZE;

	// for( size_t decodedDataSize = 2; decodedDataSize < MaxDecodedDataSize;
	// decodedDataSize++ )
	size_t decodedDataSize = 255;
	if( 1 )
	{
		// Prepare
		uint8_t *decodedData = new uint8_t[decodedDataSize];

		for( size_t i = 0; i < decodedDataSize; i++ )
		{
			decodedData[i] = (uint8_t)( i & 0xFF );
		}

		memset( buffer, UTEST_GUARD_BYTE, UTEST_ENCODED_DECODED_DATA_MAX_SIZE + UTEST_GUARD_SIZE * 2 );

		sDZCOBS_ctx ctx;

		eDZCOBS_ret ret = DZCOBS_RET_SUCCESS;

		ret = dzcobs_encode_inc_begin( &ctx,
																	 DZCOBS_PLAIN,
																	 buffer + UTEST_GUARD_SIZE,
																	 DZCOBS_MAX_ENCODED_SIZE( decodedDataSize ) + DZCOBS_FRAME_HEADER_SIZE );
		CHECK_EQUAL( DZCOBS_RET_SUCCESS, ret );

		ctx.user6bits = TEST_USERBITS;

		ret = dzcobs_encode_inc( &ctx, decodedData, decodedDataSize );
		CHECK_EQUAL( DZCOBS_RET_SUCCESS, ret );

		size_t encodedLen = 0;

		ret = dzcobs_encode_inc_end( &ctx, &encodedLen );
		CHECK_EQUAL( DZCOBS_RET_SUCCESS, ret );

		size_t decodedLen = 0;

		static const uint32_t guard = ( UTEST_GUARD_BYTE << 24 ) | ( UTEST_GUARD_BYTE << 16 ) | ( UTEST_GUARD_BYTE << 8 ) |
																	( UTEST_GUARD_BYTE << 0 );

		uint8_t *decoded_new = new uint8_t[UTEST_GUARD_SIZE + decodedDataSize + UTEST_GUARD_SIZE];

		memset( decoded_new, UTEST_GUARD_BYTE, UTEST_GUARD_SIZE + decodedDataSize + UTEST_GUARD_SIZE );

		sDZCOBS_decodectx decodeCtx;
		decodeCtx.srcBufEncoded			= buffer + UTEST_GUARD_SIZE;
		decodeCtx.srcBufEncodedLen	= encodedLen;
		decodeCtx.dstBufDecoded			= decoded_new + UTEST_GUARD_SIZE;
		decodeCtx.dstBufDecodedSize = decodedDataSize;

		uint8_t user6bitDataRightAlgn = 0;

		ret = dzcobs_decode( &decodeCtx, &decodedLen, &user6bitDataRightAlgn );

		if( ret != DZCOBS_RET_SUCCESS )
		{
			printf( "bufferEncoded " );
			debug_dump_buffer( buffer + UTEST_GUARD_SIZE, encodedLen );

			printf( "decodedData " );
			debug_dump_buffer( decodedData, decodedDataSize );

			printf( "dstBufDecoded " );
			debug_dump_buffer( decodeCtx.dstBufDecoded, decodedLen );
		}

		CHECK_EQUAL( DZCOBS_RET_SUCCESS, ret );
		CHECK_EQUAL( TEST_USERBITS, user6bitDataRightAlgn );
		CHECK_EQUAL( 0, memcmp( buffer, &guard, UTEST_GUARD_SIZE ) );
		CHECK_EQUAL( 0,
								 memcmp( buffer + UTEST_GUARD_SIZE + UTEST_ENCODED_DECODED_DATA_MAX_SIZE, &guard, UTEST_GUARD_SIZE ) );

		if( decodedDataSize != decodedLen )
		{
			printf( "bufferEncoded " );
			debug_dump_buffer( buffer + UTEST_GUARD_SIZE, encodedLen );

			printf( "decodedData " );
			debug_dump_buffer( decodedData, decodedDataSize );

			printf( "dstBufDecoded " );
			debug_dump_buffer( decodeCtx.dstBufDecoded, decodedLen );
		}

		CHECK_EQUAL( decodedDataSize, decodedLen );
		CHECK_EQUAL( 0, memcmp( decodedData, decodeCtx.dstBufDecoded, decodedLen ) );

		delete[] decoded_new;
		delete[] decodedData;
	}
}

// NOLINTBEGIN
TEST( DZCOBS, EncodeDecodeWithJumpPlain )
// NOLINTEND
{
	// Prepare
	static constexpr size_t decodedDataSize = 512;
	uint8_t decodedData[decodedDataSize];

	for( size_t i = 0; i < decodedDataSize; i++ )
	{
		decodedData[i] = (uint8_t)( i & 0xFF );
	}

	memset( buffer, UTEST_GUARD_BYTE, UTEST_ENCODED_DECODED_DATA_MAX_SIZE + UTEST_GUARD_SIZE * 2 );

	sDZCOBS_ctx ctx;

	eDZCOBS_ret ret = DZCOBS_RET_SUCCESS;

	ret = dzcobs_encode_inc_begin( &ctx,
																 DZCOBS_PLAIN,
																 buffer + UTEST_GUARD_SIZE,
																 DZCOBS_MAX_ENCODED_SIZE( decodedDataSize ) + DZCOBS_FRAME_HEADER_SIZE );
	CHECK_EQUAL( DZCOBS_RET_SUCCESS, ret );

	ctx.user6bits = TEST_USERBITS;

	ret = dzcobs_encode_inc( &ctx, decodedData, decodedDataSize );
	CHECK_EQUAL( DZCOBS_RET_SUCCESS, ret );

	size_t encodedLen = 0;

	ret = dzcobs_encode_inc_end( &ctx, &encodedLen );
	CHECK_EQUAL( DZCOBS_RET_SUCCESS, ret );

	size_t decodedLen = 0;

	static const uint32_t guard = ( UTEST_GUARD_BYTE << 24 ) | ( UTEST_GUARD_BYTE << 16 ) | ( UTEST_GUARD_BYTE << 8 ) |
																( UTEST_GUARD_BYTE << 0 );

	uint8_t decoded_new[UTEST_GUARD_SIZE + decodedDataSize + UTEST_GUARD_SIZE];

	memset( decoded_new, UTEST_GUARD_BYTE, sizeof( decoded_new ) );

	sDZCOBS_decodectx decodeCtx;
	decodeCtx.srcBufEncoded			= buffer + UTEST_GUARD_SIZE;
	decodeCtx.srcBufEncodedLen	= encodedLen;
	decodeCtx.dstBufDecoded			= decoded_new + UTEST_GUARD_SIZE;
	decodeCtx.dstBufDecodedSize = decodedDataSize;

	uint8_t user6bitDataRightAlgn = 0;

	ret = dzcobs_decode( &decodeCtx, &decodedLen, &user6bitDataRightAlgn );

	if( ret != DZCOBS_RET_SUCCESS )
	{
		printf( "bufferEncoded " );
		debug_dump_buffer( buffer + UTEST_GUARD_SIZE, encodedLen );

		printf( "decodedData " );
		debug_dump_buffer( decodedData, decodedDataSize );

		printf( "dstBufDecoded " );
		debug_dump_buffer( decodeCtx.dstBufDecoded, decodedLen );
	}

	CHECK_EQUAL( DZCOBS_RET_SUCCESS, ret );
	CHECK_EQUAL( TEST_USERBITS, user6bitDataRightAlgn );
	CHECK_EQUAL( 0, memcmp( buffer, &guard, UTEST_GUARD_SIZE ) );
	CHECK_EQUAL( 0, memcmp( buffer + UTEST_GUARD_SIZE + UTEST_ENCODED_DECODED_DATA_MAX_SIZE, &guard, UTEST_GUARD_SIZE ) );

	// if( decodedDataSize != decodedLen )
	{
		printf( "bufferEncoded " );
		debug_dump_buffer( buffer + UTEST_GUARD_SIZE, encodedLen );

		printf( "decodedData " );
		debug_dump_buffer( decodedData, decodedDataSize );

		printf( "dstBufDecoded " );
		debug_dump_buffer( decodeCtx.dstBufDecoded, decodedLen );
	}

	CHECK_EQUAL( decodedDataSize, decodedLen );
	CHECK_EQUAL( 0, memcmp( decodedData, decodeCtx.dstBufDecoded, decodedLen ) );
}

// NOLINTBEGIN
TEST( DZCOBS, EncodeDecodeLong0Plain )
// NOLINTEND
{
	static constexpr size_t MaxDecodedDataSize = ( 256 * 3 ) + 128;

	for( size_t decodedDataSize = 2; decodedDataSize < MaxDecodedDataSize; decodedDataSize++ )
	{
		// Prepare
		uint8_t *decodedData = new uint8_t[decodedDataSize];
		for( size_t i = 0; i < decodedDataSize; i++ )
		{
			decodedData[i] = 0x00;
		}

		memset( buffer, UTEST_GUARD_BYTE, UTEST_ENCODED_DECODED_DATA_MAX_SIZE + ( UTEST_GUARD_SIZE * 2 ) );

		sDZCOBS_ctx ctx;

		eDZCOBS_ret ret = DZCOBS_RET_SUCCESS;

		ret = dzcobs_encode_inc_begin( &ctx,
																	 DZCOBS_PLAIN,
																	 buffer + UTEST_GUARD_SIZE,
																	 DZCOBS_MAX_ENCODED_SIZE( decodedDataSize ) + DZCOBS_FRAME_HEADER_SIZE );
		CHECK_EQUAL( DZCOBS_RET_SUCCESS, ret );

		ctx.user6bits = TEST_USERBITS;

		ret = dzcobs_encode_inc( &ctx, decodedData, decodedDataSize );
		CHECK_EQUAL( DZCOBS_RET_SUCCESS, ret );

		size_t encodedLen = 0;

		ret = dzcobs_encode_inc_end( &ctx, &encodedLen );
		CHECK_EQUAL( DZCOBS_RET_SUCCESS, ret );

		size_t decodedLen = 0;

		static const uint32_t guard = ( UTEST_GUARD_BYTE << 24 ) | ( UTEST_GUARD_BYTE << 16 ) | ( UTEST_GUARD_BYTE << 8 ) |
																	( UTEST_GUARD_BYTE << 0 );

		CHECK_EQUAL( 0, memcmp( buffer, &guard, UTEST_GUARD_SIZE ) );
		CHECK_EQUAL( 0,
								 memcmp( buffer + UTEST_GUARD_SIZE + UTEST_ENCODED_DECODED_DATA_MAX_SIZE, &guard, UTEST_GUARD_SIZE ) );

		uint8_t *decoded_new = new uint8_t[UTEST_GUARD_SIZE + decodedDataSize + UTEST_GUARD_SIZE];

		memset( decoded_new, UTEST_GUARD_BYTE, UTEST_GUARD_SIZE + decodedDataSize + UTEST_GUARD_SIZE );

		sDZCOBS_decodectx decodeCtx;
		decodeCtx.srcBufEncoded			= buffer + UTEST_GUARD_SIZE;
		decodeCtx.srcBufEncodedLen	= encodedLen;
		decodeCtx.dstBufDecoded			= decoded_new + UTEST_GUARD_SIZE;
		decodeCtx.dstBufDecodedSize = decodedDataSize;

		uint8_t user6bitDataRightAlgn = 0;

		ret = dzcobs_decode( &decodeCtx, &decodedLen, &user6bitDataRightAlgn );

		CHECK_EQUAL( DZCOBS_RET_SUCCESS, ret );
		CHECK_EQUAL( TEST_USERBITS, user6bitDataRightAlgn );

		if( decodedDataSize != decodedLen )
		{
			printf( "bufferEncoded " );
			debug_dump_buffer( buffer + UTEST_GUARD_SIZE, encodedLen );

			printf( "decodedData " );
			debug_dump_buffer( decodedData, decodedDataSize );

			printf( "dstBufDecoded " );
			debug_dump_buffer( decodeCtx.dstBufDecoded, decodedLen );
		}

		CHECK_EQUAL( decodedDataSize, decodedLen );
		CHECK_EQUAL( 0, memcmp( decodedData, decodeCtx.dstBufDecoded, decodedLen ) );
		delete[] decoded_new;
		delete[] decodedData;
	}
}

// NOLINTBEGIN
TEST( DZCOBS, EncodeDecodeLong0Dictionary )
// NOLINTEND
{
	sDICT_ctx dictCtx;

	eDICT_ret dict_ret = dzcobs_dictionary_init( &dictCtx, s_TEST_Dictionary1, s_TEST_Dictionary1_size );
	CHECK_EQUAL( DICT_RET_SUCCESS, dict_ret );

	static constexpr size_t MaxDecodedDataSize = ( 256 * 3 ) + 128;

	for( size_t decodedDataSize = 2; decodedDataSize < MaxDecodedDataSize; decodedDataSize++ )
	{
		// Prepare
		uint8_t *decodedData = new uint8_t[decodedDataSize];
		for( size_t i = 0; i < decodedDataSize; i++ )
		{
			decodedData[i] = 0x00;
		}

		memset( buffer, UTEST_GUARD_BYTE, UTEST_ENCODED_DECODED_DATA_MAX_SIZE + ( UTEST_GUARD_SIZE * 2 ) );

		sDZCOBS_ctx ctx;

		eDZCOBS_ret ret = DZCOBS_RET_SUCCESS;

		dzcobs_encode_set_dictionary( &ctx, &dictCtx, DZCOBS_USING_DICT_1 );

		ret = dzcobs_encode_inc_begin( &ctx,
																	 DZCOBS_USING_DICT_1,
																	 buffer + UTEST_GUARD_SIZE,
																	 DZCOBS_MAX_ENCODED_SIZE( decodedDataSize ) + DZCOBS_FRAME_HEADER_SIZE );
		CHECK_EQUAL( DZCOBS_RET_SUCCESS, ret );

		ctx.user6bits = TEST_USERBITS;

		ret = dzcobs_encode_inc( &ctx, decodedData, decodedDataSize );
		CHECK_EQUAL( DZCOBS_RET_SUCCESS, ret );

		size_t encodedLen = 0;

		ret = dzcobs_encode_inc_end( &ctx, &encodedLen );
		CHECK_EQUAL( DZCOBS_RET_SUCCESS, ret );

		size_t decodedLen = 0;

		static const uint32_t guard = ( UTEST_GUARD_BYTE << 24 ) | ( UTEST_GUARD_BYTE << 16 ) | ( UTEST_GUARD_BYTE << 8 ) |
																	( UTEST_GUARD_BYTE << 0 );

		CHECK_EQUAL( 0, memcmp( buffer, &guard, UTEST_GUARD_SIZE ) );
		CHECK_EQUAL( 0,
								 memcmp( buffer + UTEST_GUARD_SIZE + UTEST_ENCODED_DECODED_DATA_MAX_SIZE, &guard, UTEST_GUARD_SIZE ) );

		uint8_t *decoded_new = new uint8_t[UTEST_GUARD_SIZE + decodedDataSize + UTEST_GUARD_SIZE];

		memset( decoded_new, UTEST_GUARD_BYTE, UTEST_GUARD_SIZE + decodedDataSize + UTEST_GUARD_SIZE );

		sDZCOBS_decodectx decodeCtx;
		decodeCtx.srcBufEncoded			= buffer + UTEST_GUARD_SIZE;
		decodeCtx.srcBufEncodedLen	= encodedLen;
		decodeCtx.dstBufDecoded			= decoded_new + UTEST_GUARD_SIZE;
		decodeCtx.dstBufDecodedSize = decodedDataSize;
		decodeCtx.pDict[0]					= &dictCtx;

		uint8_t user6bitDataRightAlgn = 0;

		ret = dzcobs_decode( &decodeCtx, &decodedLen, &user6bitDataRightAlgn );

		CHECK_EQUAL( DZCOBS_RET_SUCCESS, ret );
		CHECK_EQUAL( TEST_USERBITS, user6bitDataRightAlgn );

		if( decodedDataSize != decodedLen )
		{
			printf( "bufferEncoded " );
			debug_dump_buffer( buffer + UTEST_GUARD_SIZE, encodedLen );

			printf( "decodedData " );
			debug_dump_buffer( decodedData, decodedDataSize );

			printf( "dstBufDecoded " );
			debug_dump_buffer( decodeCtx.dstBufDecoded, decodedLen );
		}

		CHECK_EQUAL( decodedDataSize, decodedLen );
		CHECK_EQUAL( 0, memcmp( decodedData, decodeCtx.dstBufDecoded, decodedLen ) );
		delete[] decoded_new;
		delete[] decodedData;
	}
}

// NOLINTBEGIN
TEST( DZCOBS, EncodeDecode0xFFJumpPlain )
// NOLINTEND
{
	// Prepare
	static constexpr size_t decodedDataSize = 512;
	uint8_t decodedData[decodedDataSize];

	for( size_t i = 0; i < decodedDataSize; i++ )
	{
		decodedData[i] = 0xFF;
	}

	memset( buffer, UTEST_GUARD_BYTE, UTEST_ENCODED_DECODED_DATA_MAX_SIZE + UTEST_GUARD_SIZE * 2 );

	sDZCOBS_ctx ctx;

	eDZCOBS_ret ret = DZCOBS_RET_SUCCESS;

	ret = dzcobs_encode_inc_begin( &ctx,
																 DZCOBS_PLAIN,
																 buffer + UTEST_GUARD_SIZE,
																 DZCOBS_MAX_ENCODED_SIZE( decodedDataSize ) + DZCOBS_FRAME_HEADER_SIZE );
	CHECK_EQUAL( DZCOBS_RET_SUCCESS, ret );

	ctx.user6bits = TEST_USERBITS;

	ret = dzcobs_encode_inc( &ctx, decodedData, decodedDataSize );
	CHECK_EQUAL( DZCOBS_RET_SUCCESS, ret );

	size_t encodedLen = 0;

	ret = dzcobs_encode_inc_end( &ctx, &encodedLen );
	CHECK_EQUAL( DZCOBS_RET_SUCCESS, ret );

	size_t decodedLen = 0;

	static const uint32_t guard = ( UTEST_GUARD_BYTE << 24 ) | ( UTEST_GUARD_BYTE << 16 ) | ( UTEST_GUARD_BYTE << 8 ) |
																( UTEST_GUARD_BYTE << 0 );

	uint8_t decoded_new[UTEST_GUARD_SIZE + decodedDataSize + UTEST_GUARD_SIZE];

	memset( decoded_new, UTEST_GUARD_BYTE, sizeof( decoded_new ) );

	sDZCOBS_decodectx decodeCtx;
	decodeCtx.srcBufEncoded			= buffer + UTEST_GUARD_SIZE;
	decodeCtx.srcBufEncodedLen	= encodedLen;
	decodeCtx.dstBufDecoded			= decoded_new + UTEST_GUARD_SIZE;
	decodeCtx.dstBufDecodedSize = decodedDataSize;

	uint8_t user6bitDataRightAlgn = 0;

	ret = dzcobs_decode( &decodeCtx, &decodedLen, &user6bitDataRightAlgn );

	if( ret != DZCOBS_RET_SUCCESS )
	{
		printf( "bufferEncoded " );
		debug_dump_buffer( buffer + UTEST_GUARD_SIZE, encodedLen );

		printf( "decodedData " );
		debug_dump_buffer( decodedData, decodedDataSize );

		printf( "dstBufDecoded " );
		debug_dump_buffer( decodeCtx.dstBufDecoded, decodedLen );
	}

	CHECK_EQUAL( DZCOBS_RET_SUCCESS, ret );
	CHECK_EQUAL( TEST_USERBITS, user6bitDataRightAlgn );
	CHECK_EQUAL( 0, memcmp( buffer, &guard, UTEST_GUARD_SIZE ) );
	CHECK_EQUAL( 0, memcmp( buffer + UTEST_GUARD_SIZE + UTEST_ENCODED_DECODED_DATA_MAX_SIZE, &guard, UTEST_GUARD_SIZE ) );

	if( decodedDataSize != decodedLen )
	{
		printf( "bufferEncoded " );
		debug_dump_buffer( buffer + UTEST_GUARD_SIZE, encodedLen );

		printf( "decodedData " );
		debug_dump_buffer( decodedData, decodedDataSize );

		printf( "dstBufDecoded " );
		debug_dump_buffer( decodeCtx.dstBufDecoded, decodedLen );
	}

	CHECK_EQUAL( decodedDataSize, decodedLen );
	CHECK_EQUAL( 0, memcmp( decodedData, decodeCtx.dstBufDecoded, decodedLen ) );
}

// NOLINTBEGIN
TEST( DZCOBS, EncodeDecode0xFFJumpDictionary )
// NOLINTEND
{
	sDICT_ctx dictCtx;

	eDICT_ret dict_ret = dzcobs_dictionary_init( &dictCtx, s_TEST_Dictionary1, s_TEST_Dictionary1_size );
	CHECK_EQUAL( DICT_RET_SUCCESS, dict_ret );

	// Prepare
	static constexpr size_t decodedDataSize = 512;
	uint8_t decodedData[decodedDataSize];

	for( size_t i = 0; i < decodedDataSize; i++ )
	{
		decodedData[i] = 0xFF;
	}

	memset( buffer, UTEST_GUARD_BYTE, UTEST_ENCODED_DECODED_DATA_MAX_SIZE + UTEST_GUARD_SIZE * 2 );

	sDZCOBS_ctx ctx;

	eDZCOBS_ret ret = DZCOBS_RET_SUCCESS;

	dzcobs_encode_set_dictionary( &ctx, &dictCtx, DZCOBS_USING_DICT_1 );

	ret = dzcobs_encode_inc_begin( &ctx,
																 DZCOBS_USING_DICT_1,
																 buffer + UTEST_GUARD_SIZE,
																 DZCOBS_MAX_ENCODED_SIZE( decodedDataSize ) + DZCOBS_FRAME_HEADER_SIZE );
	CHECK_EQUAL( DZCOBS_RET_SUCCESS, ret );

	ctx.user6bits = TEST_USERBITS;

	ret = dzcobs_encode_inc( &ctx, decodedData, decodedDataSize );
	CHECK_EQUAL( DZCOBS_RET_SUCCESS, ret );

	size_t encodedLen = 0;

	ret = dzcobs_encode_inc_end( &ctx, &encodedLen );
	CHECK_EQUAL( DZCOBS_RET_SUCCESS, ret );

	size_t decodedLen = 0;

	static const uint32_t guard = ( UTEST_GUARD_BYTE << 24 ) | ( UTEST_GUARD_BYTE << 16 ) | ( UTEST_GUARD_BYTE << 8 ) |
																( UTEST_GUARD_BYTE << 0 );

	uint8_t decoded_new[UTEST_GUARD_SIZE + decodedDataSize + UTEST_GUARD_SIZE];

	memset( decoded_new, UTEST_GUARD_BYTE, sizeof( decoded_new ) );

	sDZCOBS_decodectx decodeCtx;
	decodeCtx.srcBufEncoded			= buffer + UTEST_GUARD_SIZE;
	decodeCtx.srcBufEncodedLen	= encodedLen;
	decodeCtx.dstBufDecoded			= decoded_new + UTEST_GUARD_SIZE;
	decodeCtx.dstBufDecodedSize = decodedDataSize;

	uint8_t user6bitDataRightAlgn = 0;

	ret = dzcobs_decode( &decodeCtx, &decodedLen, &user6bitDataRightAlgn );

	if( ret != DZCOBS_RET_SUCCESS )
	{
		printf( "bufferEncoded " );
		debug_dump_buffer( buffer + UTEST_GUARD_SIZE, encodedLen );

		printf( "decodedData " );
		debug_dump_buffer( decodedData, decodedDataSize );

		printf( "dstBufDecoded " );
		debug_dump_buffer( decodeCtx.dstBufDecoded, decodedLen );
	}

	CHECK_EQUAL( DZCOBS_RET_SUCCESS, ret );
	CHECK_EQUAL( TEST_USERBITS, user6bitDataRightAlgn );
	CHECK_EQUAL( 0, memcmp( buffer, &guard, UTEST_GUARD_SIZE ) );
	CHECK_EQUAL( 0, memcmp( buffer + UTEST_GUARD_SIZE + UTEST_ENCODED_DECODED_DATA_MAX_SIZE, &guard, UTEST_GUARD_SIZE ) );

	if( decodedDataSize != decodedLen )
	{
		printf( "bufferEncoded " );
		debug_dump_buffer( buffer + UTEST_GUARD_SIZE, encodedLen );

		printf( "decodedData " );
		debug_dump_buffer( decodedData, decodedDataSize );

		printf( "dstBufDecoded " );
		debug_dump_buffer( decodeCtx.dstBufDecoded, decodedLen );
	}

	CHECK_EQUAL( decodedDataSize, decodedLen );
	CHECK_EQUAL( 0, memcmp( decodedData, decodeCtx.dstBufDecoded, decodedLen ) );
}

// NOLINTBEGIN
TEST( DZCOBS, EncodeDecodeLongDecrementSeqPlain )
// NOLINTEND
{
	// Prepare
	uint8_t decodedData[512];
	const size_t decodedDataSize = sizeof( decodedData );
	for( size_t i = decodedDataSize; i > 0; i-- )
	{
		decodedData[i] = (uint8_t)( i & 0xFF );
	}

	memset( buffer, UTEST_GUARD_BYTE, UTEST_ENCODED_DECODED_DATA_MAX_SIZE + UTEST_GUARD_SIZE * 2 );

	sDZCOBS_ctx ctx;

	eDZCOBS_ret ret = DZCOBS_RET_SUCCESS;

	ret = dzcobs_encode_inc_begin( &ctx,
																 DZCOBS_PLAIN,
																 buffer + UTEST_GUARD_SIZE,
																 DZCOBS_MAX_ENCODED_SIZE( decodedDataSize ) + DZCOBS_FRAME_HEADER_SIZE );
	CHECK_EQUAL( DZCOBS_RET_SUCCESS, ret );

	ctx.user6bits = TEST_USERBITS;

	ret = dzcobs_encode_inc( &ctx, decodedData, decodedDataSize );
	CHECK_EQUAL( DZCOBS_RET_SUCCESS, ret );

	size_t encodedLen = 0;

	ret = dzcobs_encode_inc_end( &ctx, &encodedLen );
	CHECK_EQUAL( DZCOBS_RET_SUCCESS, ret );

	size_t decodedLen		= 0;
	uint8_t *decodedPos = nullptr;

	static const uint32_t guard = ( UTEST_GUARD_BYTE << 24 ) | ( UTEST_GUARD_BYTE << 16 ) | ( UTEST_GUARD_BYTE << 8 ) |
																( UTEST_GUARD_BYTE << 0 );

	uint8_t decoded_new[UTEST_GUARD_SIZE + 512 + UTEST_GUARD_SIZE];

	memset( decoded_new, UTEST_GUARD_BYTE, sizeof( decoded_new ) );

	sDZCOBS_decodectx decodeCtx;
	decodeCtx.srcBufEncoded			= buffer + UTEST_GUARD_SIZE;
	decodeCtx.srcBufEncodedLen	= encodedLen;
	decodeCtx.dstBufDecoded			= decoded_new + UTEST_GUARD_SIZE;
	decodeCtx.dstBufDecodedSize = 512;

	uint8_t user6bitDataRightAlgn = 0;

	ret = dzcobs_decode( &decodeCtx, &decodedLen, &user6bitDataRightAlgn );

	CHECK_EQUAL( DZCOBS_RET_SUCCESS, ret );
	CHECK_EQUAL( TEST_USERBITS, user6bitDataRightAlgn );
	CHECK_EQUAL( 0, memcmp( buffer, &guard, UTEST_GUARD_SIZE ) );
	CHECK_EQUAL( 0, memcmp( buffer + UTEST_GUARD_SIZE + UTEST_ENCODED_DECODED_DATA_MAX_SIZE, &guard, UTEST_GUARD_SIZE ) );
	CHECK_EQUAL( decodedDataSize, decodedLen );
	CHECK_EQUAL( 0, memcmp( decodedData, decodeCtx.dstBufDecoded, decodedLen ) );
}

// NOLINTBEGIN
TEST( DZCOBS, EncodeDecodeLongRandomPlain )
// NOLINTEND
{
	static constexpr size_t decodedDataSize = 512;

	// Prepare
	uint8_t decodedData[decodedDataSize];

	for( size_t i = 0; i < decodedDataSize; i++ )
	{
		decodedData[i] = (uint8_t)( rand() & 0xFF );
	}

	memset( buffer, UTEST_GUARD_BYTE, UTEST_ENCODED_DECODED_DATA_MAX_SIZE + UTEST_GUARD_SIZE * 2 );

	sDZCOBS_ctx ctx;

	eDZCOBS_ret ret = DZCOBS_RET_SUCCESS;

	ret = dzcobs_encode_inc_begin( &ctx,
																 DZCOBS_PLAIN,
																 buffer + UTEST_GUARD_SIZE,
																 DZCOBS_MAX_ENCODED_SIZE( decodedDataSize ) + DZCOBS_FRAME_HEADER_SIZE );
	CHECK_EQUAL( DZCOBS_RET_SUCCESS, ret );

	ctx.user6bits = TEST_USERBITS;

	ret = dzcobs_encode_inc( &ctx, decodedData, decodedDataSize );
	CHECK_EQUAL( DZCOBS_RET_SUCCESS, ret );

	size_t encodedLen = 0;

	ret = dzcobs_encode_inc_end( &ctx, &encodedLen );
	CHECK_EQUAL( DZCOBS_RET_SUCCESS, ret );

	size_t decodedLen = 0;

	static const uint32_t guard = ( UTEST_GUARD_BYTE << 24 ) | ( UTEST_GUARD_BYTE << 16 ) | ( UTEST_GUARD_BYTE << 8 ) |
																( UTEST_GUARD_BYTE << 0 );

	uint8_t decoded_new[UTEST_GUARD_SIZE + decodedDataSize + UTEST_GUARD_SIZE];

	memset( decoded_new, UTEST_GUARD_BYTE, sizeof( decoded_new ) );

	sDZCOBS_decodectx decodeCtx;
	decodeCtx.srcBufEncoded			= buffer + UTEST_GUARD_SIZE;
	decodeCtx.srcBufEncodedLen	= encodedLen;
	decodeCtx.dstBufDecoded			= decoded_new + UTEST_GUARD_SIZE;
	decodeCtx.dstBufDecodedSize = decodedDataSize;

	uint8_t user6bitDataRightAlgn = 0;

	ret = dzcobs_decode( &decodeCtx, &decodedLen, &user6bitDataRightAlgn );

	if( ret != DZCOBS_RET_SUCCESS )
	{
		printf( "decodedData " );
		debug_dump_buffer( decodedData, decodedDataSize );

		printf( "dstBufDecoded " );
		debug_dump_buffer( decodeCtx.dstBufDecoded, decodedLen );
	}

	CHECK_EQUAL( DZCOBS_RET_SUCCESS, ret );
	CHECK_EQUAL( TEST_USERBITS, user6bitDataRightAlgn );
	CHECK_EQUAL( 0, memcmp( buffer, &guard, UTEST_GUARD_SIZE ) );
	CHECK_EQUAL( 0, memcmp( buffer + UTEST_GUARD_SIZE + UTEST_ENCODED_DECODED_DATA_MAX_SIZE, &guard, UTEST_GUARD_SIZE ) );

	CHECK_EQUAL( 0, memcmp( decoded_new, &guard, UTEST_GUARD_SIZE ) );
	CHECK_EQUAL( 0, memcmp( decoded_new + UTEST_GUARD_SIZE + decodedDataSize, &guard, UTEST_GUARD_SIZE ) );

	if( decodedDataSize != decodedLen )
	{
		printf( "decodedData " );
		debug_dump_buffer( decodedData, decodedDataSize );

		printf( "dstBufDecoded " );
		debug_dump_buffer( decodeCtx.dstBufDecoded, decodedLen );
	}

	CHECK_EQUAL( decodedDataSize, decodedLen );
	CHECK_EQUAL( 0, memcmp( decodedData, decodeCtx.dstBufDecoded, decodedLen ) );
}

// NOLINTBEGIN
TEST( DZCOBS, EncodeDecodeLongRandomDictionary )
// NOLINTEND
{
	sDICT_ctx dictCtx;

	eDICT_ret dict_ret = dzcobs_dictionary_init( &dictCtx, s_TEST_Dictionary1, s_TEST_Dictionary1_size );
	CHECK_EQUAL( DICT_RET_SUCCESS, dict_ret );

	// Prepare
	uint8_t decodedData[512];
	const size_t decodedDataSize = sizeof( decodedData );
	for( size_t i = 0; i < decodedDataSize; i++ )
	{
		decodedData[i] = (uint8_t)( rand() & 0xFF );
	}

	memset( buffer, UTEST_GUARD_BYTE, UTEST_ENCODED_DECODED_DATA_MAX_SIZE + UTEST_GUARD_SIZE * 2 );

	sDZCOBS_ctx ctx;

	eDZCOBS_ret ret = DZCOBS_RET_SUCCESS;

	dzcobs_encode_set_dictionary( &ctx, &dictCtx, DZCOBS_USING_DICT_1 );

	ret = dzcobs_encode_inc_begin( &ctx,
																 DZCOBS_USING_DICT_1,
																 buffer + UTEST_GUARD_SIZE,
																 DZCOBS_MAX_ENCODED_SIZE( decodedDataSize ) + DZCOBS_FRAME_HEADER_SIZE );
	CHECK_EQUAL( DZCOBS_RET_SUCCESS, ret );

	ctx.user6bits = TEST_USERBITS;

	ret = dzcobs_encode_inc( &ctx, decodedData, decodedDataSize );
	CHECK_EQUAL( DZCOBS_RET_SUCCESS, ret );

	size_t encodedLen = 0;

	ret = dzcobs_encode_inc_end( &ctx, &encodedLen );
	CHECK_EQUAL( DZCOBS_RET_SUCCESS, ret );

	printf( "decodedDataSize: %lu, encodedLen: %lu, maxExpected: %lu\n",
					decodedDataSize,
					encodedLen,
					DZCOBS_MAX_ENCODED_SIZE( decodedDataSize ) + DZCOBS_FRAME_HEADER_SIZE );

	size_t decodedLen = 0;

	static const uint32_t guard = ( UTEST_GUARD_BYTE << 24 ) | ( UTEST_GUARD_BYTE << 16 ) | ( UTEST_GUARD_BYTE << 8 ) |
																( UTEST_GUARD_BYTE << 0 );

	uint8_t decoded_new[UTEST_GUARD_SIZE + 512 + UTEST_GUARD_SIZE];

	memset( decoded_new, UTEST_GUARD_BYTE, sizeof( decoded_new ) );

	sDZCOBS_decodectx decodeCtx;
	decodeCtx.srcBufEncoded			= buffer + UTEST_GUARD_SIZE;
	decodeCtx.srcBufEncodedLen	= encodedLen;
	decodeCtx.dstBufDecoded			= decoded_new + UTEST_GUARD_SIZE;
	decodeCtx.dstBufDecodedSize = 512;
	decodeCtx.pDict[0]					= &dictCtx;

	uint8_t user6bitDataRightAlgn = 0;

	ret = dzcobs_decode( &decodeCtx, &decodedLen, &user6bitDataRightAlgn );

	if( ret != DZCOBS_RET_SUCCESS )
	{
		printf( "decodedData " );
		debug_dump_buffer( decodedData, decodedDataSize );

		printf( "srcBufEncoded " );
		debug_dump_buffer( decodeCtx.srcBufEncoded, encodedLen );

		printf( "dstBufDecoded " );
		debug_dump_buffer( decodeCtx.dstBufDecoded, encodedLen );
	}

	CHECK_EQUAL( DZCOBS_RET_SUCCESS, ret );
	CHECK_EQUAL( TEST_USERBITS, user6bitDataRightAlgn );
	CHECK_EQUAL( 0, memcmp( buffer, &guard, UTEST_GUARD_SIZE ) );
	CHECK_EQUAL( 0, memcmp( buffer + UTEST_GUARD_SIZE + UTEST_ENCODED_DECODED_DATA_MAX_SIZE, &guard, UTEST_GUARD_SIZE ) );

	if( decodedDataSize != decodedLen )
	{
		printf( "decodedData " );
		debug_dump_buffer( decodedData, decodedDataSize );

		printf( "dstBufDecoded " );
		debug_dump_buffer( decodeCtx.dstBufDecoded, decodedLen );
	}

	CHECK_EQUAL( decodedDataSize, decodedLen );
	CHECK_EQUAL( 0, memcmp( decodedData, decodeCtx.dstBufDecoded, decodedLen ) );
}

// EOF
// /////////////////////////////////////////////////////////////////////////////
