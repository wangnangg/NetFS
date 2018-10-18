// Standard C++/C libraries 
#include <iostream>

// Poco-specific libraries
#include <Poco/Util/ServerApplication.h>
#include <Poco/Net/StreamSocket.h>
#include <Poco/Net/TCPServer.h>
#include <Poco/Timestamp.h>



#include "StorageServerConnectionFactory.h"
#include "StorageServerParams.h"


class StorageServer : public Poco::Net::TCPServer{
private:
 
  
public:
  
  StorageServer(StorageServerConnectionFactory::Ptr cFactory, Poco::ThreadPool& serverThreadPool,
		 const Poco::Net::ServerSocket& socket, StorageServerParams::Ptr params);
  
  ~StorageServer();
  
  Poco::Timestamp StorageServerStartTime;
  
};


class StorageServerApp : public Poco::Util::ServerApplication{

protected:
  int main(const std::vector<std::string> &);

};



