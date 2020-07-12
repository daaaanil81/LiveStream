#include "externfile.h"

std::map<ACE_thread_t, bool> thread_ids = {};
std::map<std::string, uint16_t> camera_ports = {};
bool DEBUG = false;
