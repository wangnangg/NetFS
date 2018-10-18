#include "StorageServerConnectionFactory.h"
#include <iostream>


StorageServerConnectionFactory::StorageServerConnectionFactory(){
  //std::cout << "---- StorageServerConnectionFactory created ----" << std::endl;

}

StorageServerConnectionFactory::~StorageServerConnectionFactory(){
}


Poco::Net::TCPServerConnection* StorageServerConnectionFactory::createConnection(const Poco::Net::StreamSocket& socket){

  return new StorageServerConnection(socket);

}


  
