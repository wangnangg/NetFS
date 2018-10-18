#include <Poco/Net/TCPServerConnectionFactory.h>
#include <Poco/SharedPtr.h>
#include <Poco/Net/StreamSocket.h>
#include <pqxx/pqxx>
#include <pqxx/except.hxx>
#include "StorageServerConnection.h"

class StorageServerConnectionFactory : public Poco::Net::TCPServerConnectionFactory{
private:

  
public:

  StorageServerConnectionFactory();

  ~StorageServerConnectionFactory();

  
  virtual Poco::Net::TCPServerConnection* createConnection(const Poco::Net::StreamSocket& socket);

};


