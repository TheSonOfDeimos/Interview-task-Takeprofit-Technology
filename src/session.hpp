#ifndef session_hpp
#define session_hpp

#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <thread>
#include <chrono>
#include <boost/thread/thread.hpp>
#include <boost/asio.hpp>
#include <boost/lockfree/queue.hpp>
#include <boost/function.hpp>
#include "threadSafeVector.hpp"

using boost::asio::ip::tcp;

class MessagePool
{
public:
    MessagePool(const int initial_msg);
    bool pop_msg(std::string* str);
    bool push_msg(std::string str);

private:
    std::atomic<int> counter_;
    boost::lockfree::queue<int> queue_;
    const int initial_msg_;
};

class MessageProcessor
{
public:

    void operator () (std::string str);
    std::vector<int>& getData();
    const int countMed();
    
private:
    SafeVector s_vec_;
};

class Session
{
public:
    Session(boost::shared_ptr<boost::asio::io_service> io_service, tcp::endpoint& ep);

    tcp::socket& socket();
    
    /*
     * Create async connection to srver
     * and call connect_handler
     */
    void start();
    void stop();
    void addMesgProcessor(boost::shared_ptr<MessageProcessor> proc);
    void setMsgPool(boost::shared_ptr<MessagePool> pool);
  
private:
    /*
     * Deletes all punct
     * Passes int value to thread safe countainer
     */
    void processMsg(std::string str);
    
    /*
     * Check if the connection succeded
     * Write data from counter to socket
     * Call hadle_write
     */
    void connect_handler(const boost::system::error_code& error);
    
    /*
     * Read data from socker until special symbol
     * Call handle_read
     */
    void handle_write(const boost::system::error_code& error);
    
    /*
     * Take data from sream buff and call processMsg
     */
    void handle_read(const boost::system::error_code& error, size_t bytes_transferred);
    void sendMsg();
    void handlError(const boost::system::error_code& error);
    
    
    tcp::socket socket_;
    boost::asio::streambuf buff_;
    boost::shared_ptr<boost::asio::io_service> io_service_;
    tcp::endpoint& ep_;
    boost::shared_ptr<MessagePool> msg_pool_;
    boost::shared_ptr<MessageProcessor> message_processor_;
    
    std::string current_task_;
};


#endif /* session_hpp */
