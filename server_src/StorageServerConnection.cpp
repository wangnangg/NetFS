#include "StorageServerConnection.h"
#include <Poco/Exception.h>
#include <Poco/FIFOBuffer.h>
#include <Poco/Net/NetException.h>
#include <Poco/Net/SocketStream.h>
#include <Poco/Process.h>
#include <Poco/StreamCopier.h>
#include <cassert>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include "msg.hpp"

#define BUFF_SIZE 4096
//#define NUM_REQ 16000 // for terminating server for scalability testing

StorageServerConnection::StorageServerConnection(
    const Poco::Net::StreamSocket& socket)
    : Poco::Net::TCPServerConnection(socket)
{
    this->count = this->next_count;
    this->next_count++;
}

StorageServerConnection::~StorageServerConnection() {}

/* connection should be persistent. An instance should serve a client until
 * the client shuts down.
 */
void StorageServerConnection::run()
{
    try
    {
        while (true)
        {
            auto msg = unserializeMsg([&](char* buf, size_t size) {
                int off = 0;
                int left = (int)size;
                while (left > 0)
                {
                    int read_size =
                        this->socket().receiveBytes(buf + off, left);
                    if (read_size == 0)
                    {
                        // client closed (for unknown reason)
                        throw Poco::Net::NetException(
                            "client closed with out say bye-bye. rude");
                    }
                    assert(read_size <= left);
                    left -= read_size;
                    off += read_size;
                }
            });
            switch (msg->type)
            {
                case Msg::Open:
                    std::cout << "MsgOpen" << std::endl;
                    break;
                case Msg::Stat:
                    std::cout << "MsgClose" << std::endl;
                    break;
                default:
                    std::cout << "unexpected Msg type:" << msg->type
                              << std::endl;
            }
        }
    }
    catch (const Poco::Net::NetException& pne)
    {
        std::cerr << pne.what() << std::endl;
    }
}

unsigned long StorageServerConnection::next_count = 1;

Poco::Timestamp StorageServerConnection::firstRequestTime = Poco::Timestamp();
