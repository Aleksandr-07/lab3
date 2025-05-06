#include "server.hpp"
#include <iostream>
#include <sstream>
#include <chrono>

// обработка одного клиентского соединения
Session::Session(tcp::socket socket, 
                boost::asio::strand<boost::asio::io_context::executor_type>& strand,
                std::vector<std::string>& log)
    : socket_(std::move(socket)),   // Переносим владение сокетом
      strand_(strand),              // Используем strand для синхронизации
      timer_(socket_.get_executor()), 
      log_(log)                     // Ссылка на общий лог сервера
{
    // Устанавливаем таймаут 10 секунд для операций
    timer_.expires_after(std::chrono::seconds(10));
}

void Session::start() {
    start_timer();  
    do_read();      
}

void Session::do_read() {
    auto self(shared_from_this());  // Захватываем shared_ptr для продления времени жизни
    
    // Асинхронное чтение из сокета с привязкой к strand
    socket_.async_read_some(
        boost::asio::buffer(data_, max_length),
        boost::asio::bind_executor(strand_, [this, self](boost::system::error_code ec, size_t length) {
            timer_.cancel();  // Отменяем таймер по успешному чтению
            
            if (!ec) {
                std::string request(data_, length);
                handle_request(request);  
            } else {
                // Логируем ошибку через strand для потокобезопасности
                boost::asio::post(strand_, [this, ec]() {
                    log_.push_back("Ошибка чтения: " + ec.message());
                });
            }
        }));
}

void Session::handle_request(const std::string& request) {
    try {
        int n = std::stoi(request);  // Парсим входные данные
        
        // Выполняем вычисление в strand для синхронизации доступа к ресурсам
        boost::asio::post(strand_, [this, n]() {
            uint64_t fact = 1;
            for (int i = 2; i <= n; ++i) fact *= i;
            
            // Формируем строку ответа и логируем
            std::ostringstream oss;
            oss << "Факториал " << n << " = " << fact;
            log_.push_back(oss.str());
            
            do_write(oss.str());  // Отправляем результат клиенту
        });
    } catch (...) {
        do_write("Ошибка: некорректный ввод");
    }
}

void Session::do_write(const std::string& response) {
    auto self(shared_from_this());
    
    // Асинхронная запись в сокет с привязкой к strand
    boost::asio::async_write(
        socket_, 
        boost::asio::buffer(response + "\n"),
        boost::asio::bind_executor(strand_, [this, self](boost::system::error_code ec, size_t) {
            if (!ec) {
                start_timer();  // Перезапускаем таймер после успешной записи
            }
        }));
}

void Session::start_timer() {
    // Асинхронное ожидание срабатывания таймера
    timer_.async_wait([this](boost::system::error_code ec) {
        if (!ec) {
            socket_.close();  // Закрываем соединение при таймауте
        }
    });
}

// Реализация класса Server (управление сервером)
Server::Server(boost::asio::io_context& io_context, 
              short port, 
              int thread_pool_size)
    : io_context_(io_context),
      acceptor_(io_context, tcp::endpoint(tcp::v4(), port)),  
      strand_(boost::asio::make_strand(io_context)),          
      thread_pool_size_(thread_pool_size)
{
    do_accept();  // Начинаем принимать соединения
}

void Server::run() {
    // Запускаем пул потоков для обработки операций ввода-вывода
    for (int i = 0; i < thread_pool_size_; ++i) {
        threads_.emplace_back([this]() { 
            io_context_.run();  
        });
    }
}

void Server::do_accept() {
    // Асинхронный прием новых соединений
    acceptor_.async_accept(
        boost::asio::make_strand(io_context_),  // Используем отдельный strand для accept
        [this](boost::system::error_code ec, tcp::socket socket) {
            if (!ec) {
                // Создаем сессию для нового клиента
                std::make_shared<Session>(std::move(socket), strand_, log_)->start();
            }
            do_accept();  
        });
}
