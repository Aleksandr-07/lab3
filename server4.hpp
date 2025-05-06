#pragma once
#include <boost/asio.hpp>
#include <vector>
#include <memory>
#include <thread>

using boost::asio::ip::tcp;

// Класс для обработки одного клиентского соединения
// Наследуемся от enable_shared_from_this для безопасного использования shared_ptr 
class Session : public std::enable_shared_from_this<Session> {
public:
    Session(tcp::socket socket, 
           boost::asio::strand<boost::asio::io_context::executor_type>& strand, 
           std::vector<std::string>& log);

    void start();

private:
    void do_read();
    
    // Обработка полученного запроса 
    void handle_request(const std::string& request);
    
    // Асинхронная отправка ответа клиенту
    void do_write(const std::string& response);
  
    void start_timer();

    // Сетевые элементы
    tcp::socket socket_;  // Клиентский сокет
    
    // Strand для синхронизации обработчиков 
    boost::asio::strand<boost::asio::io_context::executor_type>& strand_;
    
    // Таймер для отслеживания времени бездействия
    boost::asio::steady_timer timer_;
    
    // Ссылка на общий лог сервера (синхронизация через strand)
    std::vector<std::string>& log_;
    
    // Максимальный размер буфера для чтения
    enum { max_length = 1024 };
    char data_[max_length];  
};

// Основной класс сервера
class Server {
public:
    Server(boost::asio::io_context& io_context, 
          short port, 
          int thread_pool_size);
    
    void run();

private:
    void do_accept();

    boost::asio::io_context& io_context_;  
    tcp::acceptor acceptor_;               
    std::vector<std::string> log_;         
    
    // Главный strand для синхронизации критических операций
    boost::asio::strand<boost::asio::io_context::executor_type> strand_;
    

    int thread_pool_size_;           
    std::vector<std::thread> threads_; 
};
