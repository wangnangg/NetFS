#include "msg.hpp"

std::unordered_map<Msg::Type, std::unique_ptr<Msg> (*)(const SReader& rs)>
    unserializorLookup = {
        {Msg::Access, MsgAccess::unserialize},
        {Msg::AccessResp, MsgAccessResp::unserialize},
        {Msg::Statfs, MsgStatfs::unserialize},
        {Msg::StatfsResp, MsgStatfsResp::unserialize},
        {Msg::Create, MsgCreate::unserialize},
        {Msg::CreateResp, MsgCreateResp::unserialize},
        {Msg::Stat, MsgStat::unserialize},
        {Msg::StatResp, MsgStatResp::unserialize},
        {Msg::Readdir, MsgReaddir::unserialize},
        {Msg::ReaddirResp, MsgReaddirResp::unserialize},
        {Msg::Read, MsgRead::unserialize},
        {Msg::ReadResp, MsgReadResp::unserialize},
        {Msg::Write, MsgWrite::unserialize},
        {Msg::WriteResp, MsgWriteResp::unserialize},
        {Msg::Truncate, MsgTruncate::unserialize},
        {Msg::TruncateResp, MsgTruncateResp::unserialize},
        {Msg::Unlink, MsgUnlink::unserialize},
        {Msg::UnlinkResp, MsgUnlinkResp::unserialize},
        {Msg::Rmdir, MsgRmdir::unserialize},
        {Msg::RmdirResp, MsgRmdirResp::unserialize},
        {Msg::Mkdir, MsgMkdir::unserialize},
        {Msg::MkdirResp, MsgMkdirResp::unserialize},
        {Msg::Rename, MsgRename::unserialize},
        {Msg::RenameResp, MsgRenameResp::unserialize},
};

void serializeMsg(const Msg& msg, const SWriter& sr) { msg.serialize(sr); }

std::unique_ptr<Msg> unserializeMsg(const SReader& rs)
{
    Msg::Type type = (Msg::Type)unserializePod<uint32_t>(rs);
    auto creator = unserializorLookup.at(type);
    return creator(rs);
}
