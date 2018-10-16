#include <stdint.h>
#include <string>
#include <vector>

enum class MsgType
{
    Open = 0,
    OpenResp,
    Read,
    ReadResp,
    GetAttr,
    GetAttrResp
};

struct MsgOpen
{
    int32_t id;
    int32_t flag;
    std::string filename;
};

struct MsgOpenResp
{
    int32_t id;
    int32_t retcode;
};

// write the byte representation of msg to buf
void serialize(const MsgOpen& msg, std::vector<char>& buf);
// reconstruct MsgOpen from its byte representation
MsgOpen parse(const std::vector<char>& buf);
