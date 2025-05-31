// System HEADERS
#include <arpa/inet.h>
#include <cerrno>
#include <chrono>
#include <csignal>
#include <ctime>
#include <filesystem>
#include <functional>
#include <string>
#include <string_view>
#include <sys/select.h>
#include <unistd.h>
#include <utility>
#include <vector>
#include <sys/wait.h>

// Project HEADERS
#include "Exceptions.h"
#include "HttpHandler.h"
#include "HttpMessage.h"
#include "Log.h"
#include "ProtocolHandler.h"
#include "Router.h"
#include "SandMethod.h"
#include "Server.h"
#include "SocketIOHandler.h"
#include "Utils.h"

// Not sure if this is a good strategy
std::atomic<bool> gotSigInt{ true };

//-----------------------------------------------------------------------------
void sigIntHandler( int )
{
   gotSigInt = false;
   SLOG_WARN( "SIGINT received, shutting down gracefully..." );
}

// General Helper functions
namespace SandServer
{

//-----------------------------------------------------------------------------
void sigchld_handler( int s )
{
   // waitpid() might overwrite errno, so we save and restore it:
   int saved_errno = errno;
   while ( waitpid( -1, nullptr, WNOHANG ) > 0 )
      ;

   errno = saved_errno;
}
};   // namespace SandServer

// Main implementations
namespace SandServer
{

//-----------------------------------------------------------------------------
Server_t::Server_t() : Server_t( "config/config.toml" ) {}

//-----------------------------------------------------------------------------
Server_t::Server_t( std::string configPath ) : config{ configPath }
{
   config.parse();
   config.dump();

   threadPool.init( config.num_workers );

   // Creating callback handler for child proceses
   // Reap dead childs... poor childs :(
   // When a child process terminates, the kernel sends a SIGCHLD signal to the
   // parent
   sa.sa_handler = sigchld_handler;
   sigemptyset( &sa.sa_mask );
   sa.sa_flags = SA_RESTART;
   if ( sigaction( SIGCHLD, &sa, NULL ) == -1 )
   {
      perror( "sigaction" );
      exit( 1 );
   }

   saInter.sa_handler = sigIntHandler;
   saInter.sa_flags   = 0;   // or SA_RESTART
   sigemptyset(&saInter.sa_mask);

   sigset_t mask;
   sigemptyset( &mask );
   sigaddset( &mask, SIGINT );
   pthread_sigmask( SIG_BLOCK, &mask, nullptr );
}

//-----------------------------------------------------------------------------
Server_t::~Server_t()
{
   threadPool.stop = true;
}

//-----------------------------------------------------------------------------
void Server_t::serveStaticFiles( std::string_view filePath,
                                 std::string_view urlPrefix )
{
   // We need to know the endpoint where static files are served
   if ( !urlPrefix.empty() && urlPrefix.front() == '/' )
   {
      urlPrefix.remove_prefix( 1 );
   }

   router.staticFilesUrlPrefix = urlPrefix;

   auto handler = [ this, filePath ]( const HTTPRequest_t& request,
                                      HTTPResponse_t&      response )
   {
      response = serveFile( filePath, request.getUrlParts() );
   };   // TODO: filePath -> dir or servingDir

   router.staticFileHandler = handler;

   // Serve files
   /// Content type based of file extension
   ///
}

//-----------------------------------------------------------------------------
HTTPResponse_t Server_t::serveFile( const fs::path&                 servingDir,
                                    const std::vector<std::string>& urlParts )
{
   // Get filepath
   // Append the requested path to the root directory
   HTTPResponse_t response;
   if ( urlParts.size() < 2 )
   {
      // No file provided
      response.notFound();
      return response;
   }

   std::string filePathPart;
   fs::path    filePath = servingDir;
   for ( std::string_view part : urlParts )
   {
      if ( part.front() == '/' )
      {
         part.remove_prefix( 1 );
      }

      // TODO: This check is shit
      if ( part == "static" )
      {
         continue;
      }

      filePath = filePath / fs::path( part );
   }

   // Check if the file exists
   if ( !fs::exists( filePath ) || fs::is_directory( filePath ) )
   {
      if ( fs::is_directory( filePath ) )
      {
         // TODO: Check if this really is logged if we access a folder only
         SLOG_WARN( "Requested path is a directory. We are returning 404" );
      }

      response.notFound();
      return response;
   }

   // Open the file in read-only mode
   std::ifstream fileFD{ filePath, std::ios::binary };

   if ( fileFD.is_open() )
   {
      std::string line;
      int32_t     idx = 1;
      while ( std::getline( fileFD, line ) )
      {
         // TODO: Create a function to only append body or create a
         // temprorary string variable and only call set body once
         response.setBody( response.getBody() + line );
      }
   }

   // Get the file size
   auto size = fs::file_size( filePath );

   // Set the Content-Type header based on the file's extension
   std::string content_type = "text/plain";
   if ( filePath.extension() == ".html" )
   {
      content_type = "text/html";
   }
   else if ( filePath.extension() == ".css" )
   {
      content_type = "text/css";
   }
   else if ( filePath.extension() == ".js" )
   {
      content_type = "application/javascript";
   }
   else if ( filePath.extension() == ".wasm" )
   {
      content_type = "application/wasm";
   }

   // Send the response headers
   response.setHeader( "Content-Type", content_type );
   response.setHeader( "Content-Length", std::to_string( size ) );

   // RVO should kick in right??? right????
   // We need to read the file
   return response;
}

//-----------------------------------------------------------------------------
bool Server_t::start( int32_t port_ )
{
   std::string port = ( port_ == 0 ) ? config.port : std::to_string( port_ );

   // Initialize the socketHandler (bind to all interfaces by passing an empty
   // address string)
   try
   {
      socketHandler.init( "", std::stoi( port ), config.back_log );
   }
   catch ( const std::exception& ex )
   {
      SLOG_ERROR( "SocketHandler initialization failed: {0}", ex.what() );
      return false;
   }

   // start listnener thread here for incoming connections
   SLOG_WARN( "Sarting server on port: {0}", port );
   listenAndAccept();

   return true;
}

//-----------------------------------------------------------------------------
void Server_t::listenAndAccept()
{
   while ( gotSigInt )
   {
      auto newSocketFD = socketHandler.acceptConnection();
      if ( gotSigInt == 0 )
      {
         threadPool.stop = true;
         break;
      }

      if ( newSocketFD == -1 )
      {
         continue;
      }

      threadPool.enqueue(
          std::bind( &Server_t::processWorkerEvents, this, newSocketFD ) );
   }

   SLOG_INFO( "Shutting down server..." );
   socketHandler.closeSocket();
}

//-----------------------------------------------------------------------------
ProtocolType_t Server_t::detectProtocol( const std::string& data )
{
   // Checking uppercase letters because 100% no one will ever use this server
   // and also no one ever will send lower case method names...
   if ( data.rfind( "GET " ) || data.rfind( "POST " ) || data.rfind( "PUT " ) ||
        data.rfind( "DELETE " ) )
   {
      return ProtocolType_t::HTTP;
   }
   else if ( data.find( "Upgrade: websocket" ) !=
             std::string::npos )   // TODO: This can be wrong if the cases of
                                   // the letters are different. But this will
                                   // be a topic when implementing websockets
   {
      return ProtocolType_t::WEBSOCKET;
   }
   else if ( data.size() > 2 && ( static_cast<uint8_t>( data[ 0 ] ) == 0x10 ) )
   {
      // MQTT Connect Packet starts with 0x10
      return ProtocolType_t::MQTT;
   }

   return ProtocolType_t::UNKNOWN;
}

//-----------------------------------------------------------------------------
void Server_t::processWorkerEvents( int32_t newSocketFD )
{
   SLOG_INFO( "\n\n------ BEGIN: Got a message on the socket to read "
              "------\n\n" );

   Connection_t conn{ newSocketFD };
   socketHandler.readFromSocket( conn );
   ProtocolType_t protocol = detectProtocol( conn.persistentBuffer );

   // Dispatcher
   switch ( protocol )
   {
   case ProtocolType_t::HTTP:
   {
      HttpHandler_t httpHandler;
      httpHandler.handleConnection( conn, router );
      break;
   }
   case ProtocolType_t::WEBSOCKET:
   {
      // WebSocketHandler wsHandler;
      // wsHandler.handleConnection(clientSocket);
      break;
   }
   case ProtocolType_t::MQTT:
   {
      // MqttHandler mqttHandler;
      // mqttHandler.handleConnection(clientSocket);
      break;
   }
   default:
      SLOG_ERROR( "Unknown protocol. Closing socket {0}", conn.socketFD );
      close( conn.socketFD );
      break;
   }
}

//-----------------------------------------------------------------------------
void Server_t::addRoute( std::string&& route, const SAND_METHOD& method,
                         HandlerFunc handler )
{
   router.addRoute( route, handler, method );
}

}   // namespace SandServer
