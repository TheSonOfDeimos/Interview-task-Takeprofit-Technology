#include <exception>
#include "session.hpp"
#include "macro_definition.h"


MessagePool::MessagePool(const int initial_msg)
    : counter_(1),
    queue_(initial_msg),
    initial_msg_(initial_msg)
{
    for(int i = 1; i <= initial_msg_; i++)
    {
        queue_.push(i);
    }
}

bool MessagePool::pop_msg(std::string* str)
{
    int cnt = 0;
    queue_.pop(cnt);
    if (cnt < 1 || cnt > initial_msg_) {
        return false;
    }
    *str = std::string(std::to_string(cnt) + "\n");
    return true;
}

bool MessagePool::push_msg(std::string str)
{
    try
    {
        return queue_.push(std::stoi(str));
    }
    catch(std::invalid_argument& e)
    {
        throw std::invalid_argument("Invalid string! Msg: " + str);
    }
    catch(std::out_of_range& e)
    {
        throw std::out_of_range("Out of int base! Msg: " + str);
    }
}
    
void MessageProcessor::operator () (std::string str)
{
    str.erase(std::remove_if(str.begin(), str.end(), ::ispunct), str.end());
    try
    {
        s_vec_.push_back(std::stoi(str));
    }
    catch(std::invalid_argument& e)
    {
        throw std::invalid_argument("Invalid string! Msg: " + str);
    }
    catch(std::out_of_range& e)
    {
        throw std::out_of_range("Out of int base! Msg: " + str);
    }
}
    
std::vector<int>& MessageProcessor::getData()
{
    return s_vec_.toVector();
}

const int MessageProcessor::countMed()
{
    try
    {
        std::sort(s_vec_.begin(), s_vec_.end());
        if (s_vec_.size() % 2 == 0) {
            return (s_vec_.at(s_vec_.size() / 2) + s_vec_.at((s_vec_.size() / 2) + 1)) / 2;
        } else {
            return s_vec_.at((s_vec_.size() / 2) + 1);
        }
    }
    catch(std::out_of_range& e)
    {
        throw std::out_of_range("Error in counting median\n");
    }
}

        
Session::Session(boost::shared_ptr<boost::asio::io_service> io_service, tcp::endpoint& ep)
    : socket_(*io_service),
    ep_(ep),
    io_service_(io_service),
    current_task_("")
{
}

tcp::socket& Session::socket()
{
    return socket_;
}

void Session::start()
{
    socket_.async_connect(ep_, boost::bind(&Session::connect_handler, this,
    boost::asio::placeholders::error));
}


void Session::processMsg(std::string str)
{
    (*message_processor_)(str);
}
    
void Session::connect_handler(const boost::system::error_code& error)
{
    if (!error) {
        sendMsg();
    }
}
    
void Session::handle_read(const boost::system::error_code& error, size_t bytes_transferred)
{
    if (!error) {
        std::string str((std::istreambuf_iterator<char>(&buff_)), std::istreambuf_iterator<char>());
        if(message_processor_.get() != 0) {
            this->io_service_->post(boost::bind(&Session::processMsg, this, str));
        }
        else {
            std::cerr << "Result countainer isn't declared\n";
        }
        sendMsg();
    }
    else {
        handlError(error);
    }
}

void Session::handle_write(const boost::system::error_code& error)
{
    if (!error) {
        boost::asio::async_read_until(socket(), buff_, '\n', boost::bind(&Session::handle_read,
                                                                         this,
                                                                         boost::asio::placeholders::error,
                                                                         boost::asio::placeholders::bytes_transferred));
    }
    else {
        handlError(error);
    }
}

void Session::setMsgPool(boost::shared_ptr<MessagePool> pool)
{
    msg_pool_ = pool;
}

void Session::addMesgProcessor(boost::shared_ptr<MessageProcessor> proc)
{
    
    message_processor_ = proc;
}

void Session::sendMsg()
{
    if (msg_pool_.get() != 0) {
        std::string message_to_send;
        if (msg_pool_ -> pop_msg(&message_to_send)) {
            current_task_ = message_to_send;
            std::cout << "Counter: " << message_to_send;
            socket_.async_write_some(boost::asio::buffer(message_to_send),
                                     boost::bind(&Session::handle_write, this, boost::asio::placeholders::error));
        }
        else {
            stop();
        }
    }
    else {
        std::cerr << "Message pool isn't declared\n";
    }
}

void Session::stop()
{
    io_service_ -> post([this]() {
      socket_.close();
    });
}

void Session::handlError(const boost::system::error_code& error)
{
    std::cerr << "[Error occurred] ";
    if (boost::asio::error::connection_reset == error || boost::asio::error::eof == error) {
        std::cerr << "Connection lost!\n";
        
    }
    else {
        stop();
    }
    
    if (!current_task_.empty()) {
        msg_pool_ -> push_msg(current_task_);
        current_task_.clear();
    }
    start();
}
