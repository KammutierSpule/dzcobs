// /////////////////////////////////////////////////////////////////////////////
///	@file dzcobs_dictionary.h
///	@brief Definitions for dictionary setup and use
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
#ifndef _DZCOBS_DICTIONARY_H_
#define _DZCOBS_DICTIONARY_H_

// Includes
// /////////////////////////////////////////////////////////////////////////////
#include <stddef.h>
#include <stdint.h>

// clang-format off
#ifdef __cplusplus
extern "C" {
#endif
// clang-format on

// Definitions
// /////////////////////////////////////////////////////////////////////////////

// Helper to construct a entry on the dictionary
#define DICT_ADD_WORD( len, word ) #len word

typedef enum e_DICTVALID_ret
{
	DICT_IS_VALID = 0,
	DICT_INVALID_NOT_SORTED,
	DICT_INVALID_OUTOFBOUNDS,
	DICT_INVALID_WORDCOUNTING,
	DICT_INVALID_WORDSIZE,
	DICT_INVALID_EARLIER_END,
	DICT_INVALID_NUMBER_OF_WORDSIZES,
} eDICTVALID_ret;

enum
{
	DICT_MAX_DIFFERENTWORDSIZES = ( 4 )
};

/// Dictionary entry for different word sizes
typedef struct s_DICT_wordentry
{
	const uint8_t *dictionaryBegin; ///< Origin pointer to the dictionary entry. The first byte is the size (+'0')
	uint8_t nEntries;								///< Number of entries
	uint8_t lastIndex;							///< Number of entries -1
	uint8_t globalIndex;						///< Start index for this dictionary entry on the global dictionary. Starts at 1.
	uint8_t strideSize;							///< word size + 1, that is the size of each word entry
} sDICT_wordentry;

typedef struct s_DICT_ctx
{
	sDICT_wordentry wordSizeTable[DICT_MAX_DIFFERENTWORDSIZES];
	uint8_t minWordSize;
	uint8_t maxWordSize;
} sDICT_ctx;

typedef enum e_DICT_ret
{
	DICT_RET_SUCCESS = 0,
	DICT_RET_ERR_BAD_ARG,
	DICT_RET_ERR_INVALID,
} eDICT_ret;

enum
{
	DZCOBS_DICT_N = ( 2 )
};

// Declarations
// /////////////////////////////////////////////////////////////////////////////

/**
 * @brief Perform some verifications on dictionary to evaluate if it is valid.
 *        Checks size validity of the content.
 *
 * @param aDictionary
 * @param aDictionarySize
 * @retval DICT_IS_VALID Dictionary is well formed.
 * @retval DICT_INVALID_NOT_SORTED Is not sorted
 * @retval DICT_INVALID_OUTOFBOUNDS Out of bounds
 *         (internal word sizes does not match the dictionary size)
 * @retval DICT_INVALID_WORDCOUNTING Word counting exceeded.
 * @retval DICT_INVALID_WORDSIZE If there is an invalid word size
 */
eDICTVALID_ret dzcobs_dictionary_isvalid( const char *aDictionary, size_t aDictionarySize );

/**
 * @brief Initialize a dictionary context
 *
 * @param aCtx The context to store this dictionary session
 * @param aDictionary Pointer to the dictionary string array that will be used
 * @param aDictionarySize Size of dictionary string
 * @return eDICT_ret DICT_RET_SUCCESS if all good with parameters
 */
eDICT_ret dzcobs_dictionary_init( sDICT_ctx *aCtx, const char *aDictionary, size_t aDictionarySize );

/**
 * @brief Search for a Key in the dictionary
 *
 * @param aCtx The context to be used
 * @param aSearchKey The key buffer data
 * @param aSearchKeySize The key buffer size
 * @param aOutKeySizeFound The output with the key size found (2..5)
 * @return uint8_t 0 not found, 1..126 index of the key found (1 index based)
 */
uint8_t dzcobs_dictionary_search( const sDICT_ctx *aCtx,
																	const uint8_t *aSearchKey,
																	size_t aSearchKeySize,
																	size_t *aOutKeySizeFound );

/**
 * @brief Gets a word pointer and size, based on aIndex
 *
 * @param aCtx The context to be used
 * @param aIndex 0..125 index (0 index based)
 * @param aOutWordSize pointer to store the size in bytes of the word
 * @return uint8_t* A pointer to the word, NULL if invalid aIndex is givin
 */
const uint8_t *dzcobs_dictionary_get( const sDICT_ctx *aCtx, uint8_t aIndex, uint8_t *aOutWordSize );

// External declaration of default dictionary
extern const char G_DZCOBS_DefaultDictionary[];
extern const size_t G_DZCOBS_DefaultDictionary_size;

#ifdef __cplusplus
}
#endif

#endif

// EOF
// /////////////////////////////////////////////////////////////////////////////
