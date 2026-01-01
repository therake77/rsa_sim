#pragma once

#include <iostream>
#include <vector>
#include <cstdint>


//COMMANDS
#define REQUEST_KEYS_STR "REQK"
#define SEND_MSG_STR "SNDM"
#define STOP_SERVER_STR "STOP"
#define SEND_PROOF_OF_ID_STR "SNDP"
#define OK_STR "OK"
#define BAD_CMD_STR "BAD"
#define SHUTDOWN_STR "SHDW"


//COMMANDS CODES
#define STOP 4
#define REQUEST_KEYS 0
#define SEND_MSG 5
#define SEND_PROOF_OF_ID 3
#define OK 2
#define BAD -1


bool parse_cmd(std::string,int&,std::string&);
bool parse_keystring(std::string,uint64_t&,uint64_t&);
std::vector<std::string> parse_response(std::string,int& code);
std::vector<std::string> toblocks(std::string,int);
uint64_t stouint64(std::string);
std::string uint64tos(uint64_t);
void c_print(std::string s);

