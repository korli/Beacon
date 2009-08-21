/*
 * Copyright 2009 Haiku, Inc.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Ankur Sethi (get.me.ankur@gmail.com)
 */

#ifndef _SUPPORT_H
#define _SUPPORT_H

#include "Logger.h"

#include <cstring>
#include <cstdlib>

#include <Message.h>

enum {
	BEACON_UPDATE_INDEX = 'updt',
	BEACON_DELETE_ENTRY = 'dlte'
} ;

enum ErrorCode {
	BEACON_NOT_SUPPORTED = 'nspt',
	BEACON_FILE_EXCLUDED = 'excl',
	BEACON_FIRST_RUN = 'frst'
} ;

extern Logger *logger ;

status_t load_settings(BMessage *message) ;
status_t save_settings(BMessage *message) ;
Logger* open_log(DebugLevel level, bool replace) ;
wchar_t* to_wchar(const char *str) ;
bool is_hidden(entry_ref *ref) ;

#endif /* _SUPPORT_H */
