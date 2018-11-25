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
#include "fileop.hpp"
#include "msg.hpp"
#include "msg_response.hpp"

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
        FileOp op("./nfs_root");
        auto unserial_reader = [&](char* buf, size_t size) {
            int off = 0;
            int left = (int)size;
            while (left > 0)
            {
                int read_size = this->socket().receiveBytes(buf + off, left);
                if (read_size == 0)
                {
                    // client closed (for unknown reason)
                    throw std::runtime_error(
                        "client closed with out say bye-bye. rude");
                }
                assert(read_size <= left);
                left -= read_size;
                off += read_size;
            }
        };

        auto serial_writer = [&](const char* buf, size_t size) {
            int off = 0;
            int left = (int)size;
            while (left > 0)
            {
                int sent_size = this->socket().sendBytes(buf + off, left);
                if (sent_size <= 0)
                {
                    throw std::runtime_error(
                        "error happened while sending Msg to client");
                }
                assert(sent_size <= left);
                left -= sent_size;
                off += sent_size;
            }
        };

        std::cout << "client " << this->count << " connected." << std::endl;
        while (true)
        {
            auto msg = unserializeMsg(unserial_reader);
            auto responder = responder_map.at(msg->type);
            auto resp_msg = responder(*msg, op);
            if (resp_msg)
            {
                serializeMsg(*resp_msg, serial_writer);
            }
        }
    }
    catch (std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
}

unsigned long StorageServerConnection::next_count = 1;

Poco::Timestamp StorageServerConnection::firstRequestTime = Poco::Timestamp();
