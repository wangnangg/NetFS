#pragma once
#include <unordered_map>
#include "fileop.hpp"
#include "msg.hpp"
extern std::unordered_map<Msg::Type, std::unique_ptr<Msg> (*)(const Msg& msg,
                                                              FileOp& op)>
    responder_map;
