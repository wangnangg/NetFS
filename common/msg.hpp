#include <stdint.h>
#include <string.h>
#include <memory>
#include <string>
#include <vector>

void serialize(uint32_t val, std::vector<char>& buf)
{
    auto start = (const char*)&val;
    auto end = (const char*)&val + sizeof(val);
    buf.insert(buf.end(), start, end);
}
void serialize(int32_t val, std::vector<char>& buf)
{
    auto start = (const char*)&val;
    auto end = (const char*)&val + sizeof(val);
    buf.insert(buf.end(), start, end);
}

void serialize(const std::string& str, std::vector<char>& buf)
{
    serialize((uint32_t)str.size(), buf);
    buf.insert(buf.end(), str.begin(), str.end());
}

class Msg
{
public:
    enum Type
    {
        Open = 0,
        OpenResp,
        Read,
        ReadResp,
        GetAttr,
        GetAttrResp
    };
    const int header_size = 8;  // 4 bytes for type, 4 bytes for
private:
    Type type;

protected:
    Msg(Type type) : type(type) {}
    virtual void serializeBody(std::vector<char>& buf) const = 0;

public:
    void serialize(std::vector<char>& buf) const
    {
        ::serialize((uint32_t)type, buf);
        serializeBody(buf);
    }
    static std::unique_ptr<Msg> unserialize(std::vector<char>& buf);
};

// a Msg object is constructed from buf, and buf is advanced to the next
// unused byte
// std::invalid_argument will be thrown if serialization is not possible

class MsgOpen : public Msg
{
    int32_t id;
    int32_t flag;
    std::string filename;

public:
    MsgOpen(int32_t id, int32_t flag, std::string filename)
        : Msg(Msg::Open), id(id), flag(flag), filename(filename)
    {
    }

protected:
    virtual void serializeBody(std::vector<char>& buf) const
    {
        ::serialize(id, buf);
        ::serialize(flag, buf);
        ::serialize(filename, buf);
    }
    static std::unique_ptr<MsgOpen> unserialize(std::vector<char>& buf);
};
