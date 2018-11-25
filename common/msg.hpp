#pragma once
#include "msg_access.hpp"
#include "msg_base.hpp"
#include "msg_create.hpp"
#include "msg_mkdir.hpp"
#include "msg_read.hpp"
#include "msg_readdir.hpp"
#include "msg_rename.hpp"
#include "msg_rmdir.hpp"
#include "msg_stat.hpp"
#include "msg_statfs.hpp"
#include "msg_truncate.hpp"
#include "msg_unlink.hpp"
#include "msg_write.hpp"

void serializeMsg(const Msg& msg, const SWriter& sr);
std::unique_ptr<Msg> unserializeMsg(const SReader& rs);
