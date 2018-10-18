#include "StorageServer.h"

#define MAX_QUEUE 128
#define MAX_THREADS 4096
#define MIN_THREADS 8
#define PORT_NUM 55555

StorageServer::StorageServer(StorageServerConnectionFactory::Ptr cFactory,
			       Poco::ThreadPool& serverThreadPool,
			       const Poco::Net::ServerSocket& socket,
			       StorageServerParams::Ptr params):
  Poco::Net::TCPServer(cFactory, serverThreadPool, socket, params),
  StorageServerStartTime(Poco::Timestamp()){
  // Timestamp made to measure run time for request handling    
  
  
  std::cout << "**** StorageServer Constructed ****" << std::endl; 
}


StorageServer::~StorageServer(){
  std::cout << "**** StorageServer Destroyed ****" << std::endl;  
}



int StorageServerApp::main(const std::vector<std::string> &){

  // args: min capacity, max capacity, idle timeout, initial stack size
  Poco::ThreadPool storageThreadPool(MIN_THREADS, MAX_THREADS, 60, 0);

  // The storage server will handle all incoming connections and assigns a new
  // or existing thread to each connection (threads are managed by the thread pool)

  StorageServerParams * serverParams = new StorageServerParams();
  serverParams->setMaxQueued(MAX_QUEUE);
  
  Poco::Net::ServerSocket storageSocket(PORT_NUM);
  storageSocket.setKeepAlive(true);

  StorageServer StorageServer(new StorageServerConnectionFactory(),
				storageThreadPool,
				storageSocket,
				serverParams);

  StorageServer.start();

  std::cout << "@@@ Storage Server Started @@@" << std::endl;
  std::cout << "@@@ Listening on port " << PORT_NUM << " @@@" <<std::endl;
  std::cout << "@@@ Current thread count: " << StorageServer.currentThreads() << " @@@" << std::endl;
  std::cout << "@@@ Max thread count set at: " << StorageServer.maxThreads() << " @@@" << std::endl;

  Poco::Util::ServerApplication::waitForTerminationRequest();
  
  Poco::Timestamp::TimeDiff runtime = StorageServer.StorageServerStartTime.elapsed();

  StorageServer.stop();

  storageThreadPool.joinAll();
  
  std::cout << "@@@ Storage server shutting down...  @@@" << std::endl;
  std::cout << "@@@ Total up time (us) = " << runtime << " @@@" << std::endl;
  std::cout << "@@@ Total up time (seconds) = "
	    << ((double)runtime/(double)1000000) << " @@@" << std::endl;
  
  return Poco::Util::Application::EXIT_OK;
}



/* Main function to run the storage Matching Application + Server */
int main(int argc, char *argv[]){
  StorageServerApp app;
  return app.run(argc, argv);
}





