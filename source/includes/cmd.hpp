#include <iostream>
#include <vector>

#define CREATE_KEYS_STR "CRTK"
#define SENDMSG_STR "SEND"
#define START_COMMS_STR "STRC"
#define RECEIVED_STR "RCVD"
#define READY_STR "REDY"
#define STOP_STR "STOP"

#define STOP 4
#define CREATE_KEYS 0
#define SENDMSG 5
#define START_COMMS 1
#define RECEIVED 2
#define READY 3
#define ERROR -1

int parsecmd(std::string);

std::vector<std::string> toblocks(std::string,int);