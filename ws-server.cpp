#include "log.h"
#include <cstring>
#include <iostream>

int ACE_TMAIN (int, ACE_TCHAR *[])
{
    LOG(LOG_DEBUG, "Test logging system for LOG_DEBUG");
    LOG(LOG_INFO, "Test logging system for LOG_INFO");
    LOG(LOG_NOTICE, "Test logging system for LOG_NOTICE");
    LOG(LOG_WARNING, "Test logging system for LOG_WARNING");
    LOG(LOG_ERROR, "Test logging system for LOG_ERROR");
    return 0;
}
