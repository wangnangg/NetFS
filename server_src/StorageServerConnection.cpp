#include "StorageServerConnection.h"
#include <Poco/Exception.h>
#include <Poco/FIFOBuffer.h>
#include <Poco/Net/NetException.h>
#include <Poco/Net/SocketStream.h>
#include <Poco/Process.h>
#include <Poco/StreamCopier.h>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>

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

void StorageServerConnection::run()
{
    // For measuring execution time
    if (this->count == 1)
    {
        this->firstRequestTime = Poco::Timestamp();
    }

    std::string response =
        " * Storage Server received message from address: " +
        this->socket().peerAddress().toString() + "\r\n" +
        " * This StorageServer will reply with the requested file... "
        "eventually \r\n" +
        " *\r\n" + " *\r\n" + " * Seeks are bad.... \r\n";

    try
    {
        // this->socket().setKeepAlive(true);

        char Buffer[BUFF_SIZE];

        size_t readBytes = this->socket().receiveBytes(Buffer, BUFF_SIZE);

        std::cout << "Received message from address: "
                  << this->socket().peerAddress().toString() << std::endl;
        std::cout << readBytes << " bytes read into buffer of size "
                  << BUFF_SIZE << std::endl;
        std::cout << "======================================================"
                  << std::endl;
        std::cout << "Received message: " << std::endl;
        std::cout << Buffer << std::endl;
        std::cout << "======================================================"
                  << std::endl;
    }
    catch (const Poco::Net::NetException& pne)
    {
        std::cerr << pne.what() << std::endl;
    }

    // const char* to_client = response.c_str();

    // size_t sentBytes = this->socket().sendBytes(to_client,
    // strlen(to_client) + 1);
}

unsigned long StorageServerConnection::next_count = 1;

Poco::Timestamp StorageServerConnection::firstRequestTime = Poco::Timestamp();
