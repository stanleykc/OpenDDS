// -*- C++ -*-
// Definition for Win32 Export directives.
// This file is generated automatically by generate_export_file.pl HsdsCommon
// ------------------------------
#ifndef HSDSCOMMON_EXPORT_H
#define HSDSCOMMON_EXPORT_H

#include "ace/config-all.h"

#if defined (ACE_AS_STATIC_LIBS) && !defined (HSDSCOMMON_HAS_DLL)
#  define HSDSCOMMON_HAS_DLL 0
#endif /* ACE_AS_STATIC_LIBS && HSDSCOMMON_HAS_DLL */

#if !defined (HSDSCOMMON_HAS_DLL)
#  define HSDSCOMMON_HAS_DLL 1
#endif /* ! HSDSCOMMON_HAS_DLL */

#if defined (HSDSCOMMON_HAS_DLL) && (HSDSCOMMON_HAS_DLL == 1)
#  if defined (HSDSCOMMON_BUILD_DLL)
#    define HsdsCommon_Export ACE_Proper_Export_Flag
#    define HSDSCOMMON_SINGLETON_DECLARATION(T) ACE_EXPORT_SINGLETON_DECLARATION (T)
#    define HSDSCOMMON_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK) ACE_EXPORT_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#  else /* HSDSCOMMON_BUILD_DLL */
#    define HsdsCommon_Export ACE_Proper_Import_Flag
#    define HSDSCOMMON_SINGLETON_DECLARATION(T) ACE_IMPORT_SINGLETON_DECLARATION (T)
#    define HSDSCOMMON_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK) ACE_IMPORT_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#  endif /* HSDSCOMMON_BUILD_DLL */
#else /* HSDSCOMMON_HAS_DLL == 1 */
#  define HsdsCommon_Export
#  define HSDSCOMMON_SINGLETON_DECLARATION(T)
#  define HSDSCOMMON_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#endif /* HSDSCOMMON_HAS_DLL == 1 */

// Set HSDSCOMMON_NTRACE = 0 to turn on library specific tracing even if
// tracing is turned off for ACE.
#if !defined (HSDSCOMMON_NTRACE)
#  if (ACE_NTRACE == 1)
#    define HSDSCOMMON_NTRACE 1
#  else /* (ACE_NTRACE == 1) */
#    define HSDSCOMMON_NTRACE 0
#  endif /* (ACE_NTRACE == 1) */
#endif /* !HSDSCOMMON_NTRACE */

#if (HSDSCOMMON_NTRACE == 1)
#  define HSDSCOMMON_TRACE(X)
#else /* (HSDSCOMMON_NTRACE == 1) */
#  if !defined (ACE_HAS_TRACE)
#    define ACE_HAS_TRACE
#  endif /* ACE_HAS_TRACE */
#  define HSDSCOMMON_TRACE(X) ACE_TRACE_IMPL(X)
#  include "ace/Trace.h"
#endif /* (HSDSCOMMON_NTRACE == 1) */

#endif /* HSDSCOMMON_EXPORT_H */

// End of auto generated file.