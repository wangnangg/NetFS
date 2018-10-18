#include <Poco/Net/TCPServerConnection.h>
#include <Poco/Net/StreamSocket.h>
#include <Poco/Timestamp.h>

class StorageServerConnection : public Poco::Net::TCPServerConnection{
private:

  static unsigned long next_count;
  
  unsigned long count;

  static Poco::Timestamp firstRequestTime;

 public:

  StorageServerConnection(const Poco::Net::StreamSocket& socket);
  
  ~StorageServerConnection();

  virtual void run();

  
};
