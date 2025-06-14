// /////////////////////////////////////////////////////////////////////////////
///	@file dzcobs_assert.h
///	@brief Assert declaration
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
#ifndef _DZCOBS_ASSERT_H_
#define _DZCOBS_ASSERT_H_

// Includes
// /////////////////////////////////////////////////////////////////////////////

#ifndef DZCOBS_ASSERT

#if DZCOBS_IS_DEBUG_BUILD == 1
#include <assert.h>
#define DZCOBS_ASSERT( a ) assert( a )
#else
#define DZCOBS_ASSERT( a )
#endif

#endif

#ifndef DZCOBS_RUN_ONDEBUG

#if DZCOBS_IS_DEBUG_BUILD == 1
#define DZCOBS_RUN_ONDEBUG( a ) ( a )
#else
#define DZCOBS_RUN_ONDEBUG( a )
#endif

#endif

#endif

// EOF
// /////////////////////////////////////////////////////////////////////////////
