#ifndef _LOG_H_
#define _LOG_H_

#include <ace/Log_Msg.h>
#include <iostream>

#define LOG_DEBUG      LM_DEBUG,     ACE_TEXT ("%s:\033[1;36mDEBUG\033[0m:%d:%s -> %s\n")
#define LOG_INFO       LM_INFO,      ACE_TEXT ("%s:\033[1;32mINFO\033[0m:%d:%s -> %s\n")
#define LOG_NOTICE     LM_NOTICE,    ACE_TEXT ("%s:\033[1;32mNOTICE\033[0m:%d:%s -> %s\n")
#define LOG_WARNING    LM_WARNING,   ACE_TEXT ("%s:\033[1;31mWARNING\033[0m:%d:%s -> %s\n")
#define LOG_ERROR      LM_ERROR,     ACE_TEXT ("%s:\033[1;31mERROR\033[0m:%d:%s -> %s\n")
#define LOG_CRITICAL   LM_CRITICAL,  ACE_TEXT ("%s:CRITICAL:%d:%s -> %s\n")
#define LOG_ALERT      LM_ALERT,     ACE_TEXT ("%s:ALERT:%d:%s -> %s\n")
#define LOG_EMERGENCY  LM_EMERGENCY, ACE_TEXT ("%s:EMERGENCY:%d:%s -> %s\n")
#define LOG( LM, FMT )                                                  \
    do { ACE_DEBUG((LM, __TIME__, __LINE__, ACE_TEXT (__FUNCTION__), ACE_TEXT(FMT)));} while (0)

#endif
