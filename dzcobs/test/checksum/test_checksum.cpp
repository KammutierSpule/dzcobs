// /////////////////////////////////////////////////////////////////////////////
///	@file test_crc.cpp
///	@brief Tests for chekcsum
/// Consider that it was choose an algorithm that allows independent order add
/// of the data.
///
/// Statistics (probability of undetected errors)
///	3 byte message:
///              Multi        XOR        Checksum
/// 1-bit error  0.00%      0.00%    0.00%
/// 2-bit error  4.69%      12.81%   8.77%
/// 3-bit error  0.21%      0.00%    0.36%
/// 4-bit error  0.69%      4.12%    2.05%
///
///	4 byte message:
///              Multi        XOR        Checksum
/// 1-bit error  0.00%      0.00%    0.00%
/// 2-bit error  4.29%      12.72%   8.95%
/// 3-bit error  0.35%      0.00%    0.88%
/// 4-bit error  0.86%      4.26%    1.65%
///
///	16 byte message:
///              Multi        XOR        Checksum
/// 1-bit error  0.00%      0.00%    0.00%
/// 2-bit error  1.56%      12.53%   7.48%
/// 3-bit error  0.45%      0.00%    1.08%
/// 4-bit error  0.45%      4.28%    1.65%
///
///	16 byte message:
///              Multi        XOR        Checksum
/// 1-bit error  0.00%      0.00%    0.00%
/// 2-bit error  0.88%      12.51%   7.07%
/// 3-bit error  0.44%      0.00%    1.18%
/// 4-bit error  0.42%      4.27%    1.64%
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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> // for memcpy
#include <time.h>

// Definitions
// /////////////////////////////////////////////////////////////////////////////

// Setup
// /////////////////////////////////////////////////////////////////////////////

// clang-format off
// NOLINTBEGIN
TEST_GROUP( DZCOBS_CHECKSUM ){
	void setup()
	{
	}

	void teardown()
	{
	}
};
// NOLINTEND
// clang-format on

// Tests
// /////////////////////////////////////////////////////////////////////////////

// Simple 8-bit hash function for a byte (can be improved)
uint8_t hash8( uint8_t byte )
{
	// A very basic hash: mixing bits non-linearly
	byte ^= ( byte >> 3 );
	byte *= 167; // Multiply by a prime number
	byte ^= ( byte << 1 );

	return byte;
}

// Order-independent multiset hash (sum of hash8 values mod 256)
uint8_t multiset_hash( uint8_t *data, size_t len )
{
	uint16_t sum = 0;
	for( size_t i = 0; i < len; i++ )
	{
		sum += hash8( data[i] );
	}
	return (uint8_t)( sum & 0xFF );
}

// Compute XOR checksum
uint8_t xor_checksum( uint8_t *data, size_t len )
{
	uint8_t result = 0;
	for( size_t i = 0; i < len; i++ )
	{
		result ^= data[i];
	}
	return result;
}

// Compute modular sum checksum (mod 256)
uint8_t sum_checksum( uint8_t *data, size_t len )
{
	uint16_t sum = 0;
	for( size_t i = 0; i < len; i++ )
	{
		sum += data[i];
	}
	return (uint8_t)( sum & 0xFF );
}

// Flip a random bit in the data
void flip_random_bit( uint8_t *data, size_t len )
{
	size_t byte_index = rand() % len;
	uint8_t bit_index = rand() % 8;
	data[byte_index] ^= ( 1 << bit_index );
}

#define N_ERROR_BITS 4
#define MESSAGE_LENGTH 128 // Length of the random message
#define ITERATIONS 1000000 // Number of random bit flips to test

// NOLINTBEGIN
TEST( DZCOBS_CHECKSUM, TEST_CHECKSUM )
// NOLINTEND
{
	srand( (unsigned int)time( NULL ) ); // Seed the RNG

	uint8_t message[MESSAGE_LENGTH];

	// Generate random message
	for( size_t i = 0; i < MESSAGE_LENGTH; i++ )
	{
		message[i] = rand() % 256;
	}

	// Compute original checksums
	uint8_t original_xor			= xor_checksum( message, MESSAGE_LENGTH );
	uint8_t original_sum			= sum_checksum( message, MESSAGE_LENGTH );
	uint8_t original_multiset = multiset_hash( message, MESSAGE_LENGTH );

	size_t xor_failures			 = 0;
	size_t sum_failures			 = 0;
	size_t multiset_failures = 0;

	uint8_t test_message[MESSAGE_LENGTH];

	for( size_t i = 0; i < ITERATIONS; i++ )
	{
		// Copy original message using memcpy
		memcpy( test_message, message, MESSAGE_LENGTH );

		for( size_t n_errors = 0; n_errors < N_ERROR_BITS; n_errors++ )
		{
			flip_random_bit( test_message, MESSAGE_LENGTH );
		}

		// Recompute checksums
		uint8_t new_xor			 = xor_checksum( test_message, MESSAGE_LENGTH );
		uint8_t new_sum			 = sum_checksum( test_message, MESSAGE_LENGTH );
		uint8_t new_multiset = multiset_hash( test_message, MESSAGE_LENGTH );

		// Check for detection
		if( new_xor == original_xor )
			xor_failures++;
		if( new_sum == original_sum )
			sum_failures++;
		if( new_multiset == original_multiset )
			multiset_failures++;
	}

	printf( "Total iterations: %d (%d-bit error each)\n", ITERATIONS, N_ERROR_BITS );
	printf( "XOR failed to detect: %zu times (%.4f%%)\n", xor_failures, ( 100.0 * xor_failures / ITERATIONS ) );
	printf( "SUM failed to detect: %zu times (%.4f%%)\n", sum_failures, ( 100.0 * sum_failures / ITERATIONS ) );
	printf( "Multiset failed to detect: %zu times (%.4f%%)\n",
					multiset_failures,
					( 100.0 * multiset_failures / ITERATIONS ) );
}

// EOF
// /////////////////////////////////////////////////////////////////////////////
