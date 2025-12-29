#pragma once

#include <iostream>
#include <vector>
#include <cstdint>

#define REQUEST_KEYS_STR "REQK"
#define INVALIDATE_KEYS_STR "INVK"
#define OK_STR "OK"
#define SEND_NONENCRYPTED_MSG_STR "SNDM"
#define SEND_ENCRYPTED_MSG_STR "SNDE"
#define STOP_SERVER_MSG_STR "STOP"
#define BAD_CMD_STR "BAD"
#define SHUTDOWN_STR "SHDW"

#define STOP 4
#define REQUEST_KEYS 0
#define SEND_NONENCRYPTED_MSG 5
#define SEND_ENCRYPTED_MSG 1
#define OK 2
#define BAD -1
#define INVALIDATE_KEYS 6

int parsecmd(std::string);
std::vector<std::string> toblocks(std::string,int);
uint64_t stouint64(std::string);
std::string uint64tos(uint64_t);
void c_print(std::string s);