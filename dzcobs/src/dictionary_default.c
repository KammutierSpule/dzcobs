// /////////////////////////////////////////////////////////////////////////////
///	@file dictionary_default.c
///	@brief This is an example of a dictionary.
/// Optimized for embedded systems word context, json common tags and sequences,
/// English letter frequencies based on the following sites:
/// https://mathcenter.oxford.emory.edu/site/math125/englishLetterFreqs/
/// http://norvig.com/mayzner.html
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
#include <dzcobs/dzcobs_dictionary.h>

// clang-format off

/// Dictionary string, descendent order, null terminated
const char G_DZCOBS_DefaultDictionary[] =
	DICT_ADD_WORD(2, "\x00\x00")
	DICT_ADD_WORD(2, "\x00\x01")
	DICT_ADD_WORD(2, "\x01\x00")
	DICT_ADD_WORD(2, "\x0D\x0A")
	DICT_ADD_WORD(3, "\x00\x00\x00")
	DICT_ADD_WORD(3, "\x00\x00\x01")
	DICT_ADD_WORD(3, "\x00\x01\x00")
	DICT_ADD_WORD(3, "\x01\x00\x00")
;

// clang-format on

const size_t G_DZCOBS_DefaultDictionary_size = sizeof( G_DZCOBS_DefaultDictionary );

// EOF
// /////////////////////////////////////////////////////////////////////////////
