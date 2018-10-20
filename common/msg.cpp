#include "msg.hpp"

std::unordered_map<Msg::Type, std::unique_ptr<Msg> (*)(const SReader& rs)>
    unserializorLookup = {
        {Msg::Open, MsgOpen::unserialize},
        {Msg::OpenResp, MsgOpenResp::unserialize},
        {Msg::Stat, MsgStat::unserialize},
        {Msg::StatResp, MsgStatResp::unserialize},
        {Msg::Readdir, MsgReaddir::unserialize},
        {Msg::ReaddirResp, MsgReaddirResp::unserialize},
        {Msg::Read, MsgRead::unserialize},
        {Msg::ReadResp, MsgReadResp::unserialize},
        {Msg::Write, MsgWrite::unserialize},
        {Msg::WriteResp, MsgWriteResp::unserialize},
};

void serializeMsg(const Msg& msg, const SWriter& sr) { msg.serialize(sr); }

std::unique_ptr<Msg> unserializeMsg(const SReader& rs)
{
    Msg::Type type = (Msg::Type)unserializePod<uint32_t>(rs);
    auto creator = unserializorLookup.at(type);
    return creator(rs);
}
