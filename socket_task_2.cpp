#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <random>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>
#include <algorithm>
#include <future>
#include <memory>
#include <stdexcept>
#include <string>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>

class EventWorker;

class EventListener
{
public:
    ~EventListener();

    void setBacklog( int backlog );
    void setPort( int port );

    void close();
    void listen();
    void close( int fileDescriptor );

    template <class F> void onAccept( F&& f ) { m_handleAccept = f; }
    template <class F> void onRead( F&& f ) { m_handleRead = f; }

private:
    int m_backlog =  1;
    int m_port    = -1;
    int m_socket  = -1;

    std::function< void ( std::weak_ptr<EventWorker> worker ) > m_handleAccept;
    std::function< void ( std::weak_ptr<EventWorker> worker ) > m_handleRead;

    std::vector< std::shared_ptr<EventWorker> > m_eventWorkers;

    std::vector<int> m_staleFileDescriptors;
    std::mutex m_staleFileDescriptorsMutex;
};

class EventWorker
{
public:
    EventWorker( int fileDescriptor, EventListener& eventListener );
    ~EventWorker();

    int fileDescriptor() const;

    void close();
    void write( const std::string& data );

    std::string read();

    EventWorker( const EventWorker& )            = delete;
    EventWorker& operator=( const EventWorker& ) = delete;

private:
    int m_fileDescriptor = -1;
    EventListener& m_eventListener;
};

static std::fstream outFile;
static EventListener eventListener;
void handleExitSignal( int ) {
    eventListener.close();
    outFile.close();
}

int main()
{
    using namespace std::chrono_literals;
    using std::cout;
    using std::endl;

    cout << endl;
    cout << "Event Listener started on port: 3678" << endl;
    cout << "See 'log.txt' for details" << endl;

    signal( SIGINT, handleExitSignal );
    eventListener.setPort( 3678 );
    outFile.open("log.txt", std::fstream::out);

    std::mutex fileOutputMutex;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distr(1000, 5000);

    eventListener.onRead( [&] ( std::weak_ptr<EventWorker> eventWorker )
    {
        if( auto ew = eventWorker.lock() )
        {
            auto start = std::chrono::high_resolution_clock::now();
            std::this_thread::sleep_for(std::chrono::milliseconds{distr(gen)});
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> elapsed = end-start;

            auto data = ew->read();
            {
                std::lock_guard<std::mutex> lock( fileOutputMutex );
                outFile << "Worker: " << ew->fileDescriptor() << endl;
                outFile << "  content:" << endl;
                outFile << "\t" << data << endl;
                outFile << "  elapsed: " << elapsed.count() << " ms" << endl << endl;
            }
            ew->write( data ); // echo back
        }
    } );

    eventListener.listen();

    return 0;
}


/////////////////////////// EventListener class //////////////////////////////
EventListener::~EventListener()
{
    this->close();
}

void EventListener::setBacklog( int backlog )
{
    m_backlog = backlog;
}

void EventListener::setPort( int port )
{
    m_port = port;
}

void EventListener::close()
{
    if( m_socket )
        ::close( m_socket );

    for( auto&& eventWorker : m_eventWorkers )
        eventWorker->close();

    m_eventWorkers.clear();
}

void EventListener::listen()
{
    m_socket = socket( AF_INET, SOCK_STREAM, 0 );

    if( m_socket == -1 )
        throw std::runtime_error( std::string( strerror( errno ) ) );

    {
        int option = 1;

        setsockopt( m_socket,
                    SOL_SOCKET,
                    SO_REUSEADDR,
                    reinterpret_cast<const void*>( &option ),
                    sizeof( option ) );
    }

    sockaddr_in socketAddress;

    std::fill( reinterpret_cast<char*>( &socketAddress ),
               reinterpret_cast<char*>( &socketAddress ) + sizeof( socketAddress ),
               0 );

    socketAddress.sin_family      = AF_INET;
    socketAddress.sin_addr.s_addr = htonl( INADDR_ANY );
    socketAddress.sin_port        = htons( m_port );

    {
        auto result = bind( m_socket,
                            reinterpret_cast<const sockaddr*>( &socketAddress ),
                            sizeof( socketAddress ) );

        if( result == -1 )
            throw std::runtime_error( std::string( strerror( errno ) ) );
    }

    {
        int result = ::listen( m_socket, m_backlog );

        if( result == -1 )
            throw std::runtime_error( std::string( strerror( errno ) ) );
    }

    fd_set masterSocketSet;
    fd_set clientSocketSet;

    FD_ZERO( &masterSocketSet );
    FD_SET( m_socket, &masterSocketSet );

    int highestFileDescriptor = m_socket;

    while( 1 )
    {
        clientSocketSet = masterSocketSet;

        int numFileDescriptors = select( highestFileDescriptor + 1,
                                         &clientSocketSet,
                                         nullptr,
                                         nullptr,
                                         nullptr );

        if( numFileDescriptors == -1 )
            break;


        int newHighestFileDescriptor = highestFileDescriptor;

        for( int i = 0; i <= highestFileDescriptor; i++ )
        {
            if( !FD_ISSET( i, &clientSocketSet ) )
                continue;

            // Handle new client
            if( i == m_socket )
            {
                sockaddr_in clientAddress;
                auto clientAddressLength = sizeof(clientAddress);

                int clientFileDescriptor = accept( m_socket,
                                                   reinterpret_cast<sockaddr*>( &clientAddress ),
                                                   reinterpret_cast<socklen_t*>( &clientAddressLength ) );

                if( clientFileDescriptor == -1 )
                    break;

                FD_SET( clientFileDescriptor, &masterSocketSet );
                newHighestFileDescriptor = std::max( highestFileDescriptor, clientFileDescriptor );

                auto eventWorker = std::make_shared<EventWorker>( clientFileDescriptor, *this );

                if( m_handleAccept )
                    auto result = std::async( std::launch::async, m_handleAccept, eventWorker );

                m_eventWorkers.push_back( eventWorker );
            }

            // Known client socket
            else
            {
                char buffer[2] = {0,0};
                int result = recv( i, buffer, 1, MSG_PEEK );

                if( result <= 0 )
                {
                    this->close( i );
                }
                else
                {
                    auto itEventWorker = std::find_if( m_eventWorkers.begin(), m_eventWorkers.end(),
                                                       [&] ( std::shared_ptr<EventWorker> eventWorker )
                    { return eventWorker->fileDescriptor() == i; });

                    if( itEventWorker != m_eventWorkers.end() && m_handleRead )
                        auto result = std::async( std::launch::async, m_handleRead, *itEventWorker );
                }
            }
        }

        highestFileDescriptor = std::max( newHighestFileDescriptor, highestFileDescriptor );

        {
            std::lock_guard<std::mutex> lock( m_staleFileDescriptorsMutex );

            for( auto&& fileDescriptor : m_staleFileDescriptors )
            {
                FD_CLR( fileDescriptor, &masterSocketSet );
                ::close( fileDescriptor );
            }

            m_staleFileDescriptors.clear();
        }
    }
}

void EventListener::close( int fileDescriptor )
{
    std::lock_guard<std::mutex> lock( m_staleFileDescriptorsMutex );

    m_eventWorkers.erase( std::remove_if( m_eventWorkers.begin(), m_eventWorkers.end(),
                                          [&] ( std::shared_ptr<EventWorker> eventWorker )
                          {
                              return eventWorker->fileDescriptor() == fileDescriptor;
                          } ),
            m_eventWorkers.end() );

    m_staleFileDescriptors.push_back( fileDescriptor );
}


/////////////////////////// EventWorker class //////////////////////////////
EventWorker::EventWorker( int fileDescriptor, EventListener& eventListener )
    : m_fileDescriptor( fileDescriptor )
    , m_eventListener( eventListener )
{
}

EventWorker::~EventWorker()
{
}

int EventWorker::fileDescriptor() const
{
    return m_fileDescriptor;
}

void EventWorker::close()
{
    m_eventListener.close( m_fileDescriptor );
}

void EventWorker::write( const std::string& data )
{
    auto result = send( m_fileDescriptor,
                        reinterpret_cast<const void*>( data.c_str() ),
                        data.size(),
                        0 );

    if( result == -1 )
        throw std::runtime_error( std::string( strerror( errno ) ) );
}

std::string EventWorker::read()
{
    std::string message;

    char buffer[256] = { 0 };
    ssize_t numBytes = 0;

    while( ( numBytes = recv( m_fileDescriptor, buffer, sizeof(buffer), MSG_DONTWAIT ) ) > 0 )
    {
        buffer[numBytes]  = 0;
        message          += buffer;
    }

    return message;
}
