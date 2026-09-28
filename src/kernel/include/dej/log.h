#pragma once
#include <dej/kernel.h>
#include <stdarg.h>
/*
 *
 * Log.h
 *
 *
 * Story: Back in the day when i had printf as my long function and shit was all bad cuz i forgot that percpu data wasnt setup yet and all that shit
 * so now i have a better way i make log.h and lowk just use my brain to make sure everything is being loaded at the right time also i lowk dont
 * want this to need to be inited so here we go
 */

void LogStr(const char * s);
void LogfStr(const char * s, ...);
void LogStrEarly(const char *s);
void LogfStrEarly(const char * s, ...);
void LogRaw(const char * s, size_t len);
