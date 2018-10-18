#include <Poco/Net/TCPServerParams.h>
#include <Poco/SharedPtr.h>


// configures storage server 
class StorageServerParams : public Poco::Net::TCPServerParams{

public:
  //typedef Poco::SharedPtr<StorageServerParams> Ptr;

  StorageServerParams();

  ~StorageServerParams();
  

};



