/*
 * log.h
 * Logging functions
 *
 *
 */



void strlog(const char *restrict mod, const char *restrict msg);
void fstrlog(const char *restrict mod, const char *restrict msg, ...);
void early_strlog(const char *restrict mod, const char *restrict msg);
void early_fstrlog(const char *restrict mod, const char *restrict msg, ...);
