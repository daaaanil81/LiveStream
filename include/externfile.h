#include <iostream>
#include <map>
#include <string>
#include "ace/Thread.h"
#include "ace/Thread_Manager.h"

extern std::map<ACE_thread_t, bool> thread_ids;
extern std::map<std::string, uint16_t> camera_ports;
extern bool DEBUG;
