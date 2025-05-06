#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <sstream>
#include <thread>

using boost::asio::ip::tcp;
namespace asio = boost::asio;

// Класс для обработки подключения клиента
class Session : public std::enable_shared_from_this<Session> {
public:
    Session(tcp::socket socket, asio::io_context& io)
        : socket_(std::move(socket)), 
          timer_(io),     
          io_(io) {}       

    void start() {
        read(); 
    }

private:
    void read() {
        auto self(shared_from_this()); // Захват shared_ptr 
        
        // Асинхронное чтение до символа новой строки
        asio::async_read_until(socket_, buffer_, '\n',
            [this, self](boost::system::error_code ec, size_t len) {
                if (!ec) {
                    // Преобразование буфера в строку
                    std::string data{
                        asio::buffers_begin(buffer_.data()),
                        asio::buffers_begin(buffer_.data()) + len
                    };
                    buffer_.consume(len); // Очистка буфера
                    
                    // Обработка команды таймера
                    if (data.rfind("таймер ", 0) == 0) { // Проверка префикса
                        std::stringstream ss(data.substr(7)); // Парсинг времени
                        int seconds;
                        if (ss >> seconds && seconds > 0) {
                            start_timer(seconds); 
                        } else {
                            write("Ошибка: Некорректное время\n");
                        }
                    } else {
                        // Обработка запроса факториала в пуле потоков
                        asio::post(io_, [this, data]() {
                            try {
                                int num = std::stoi(data);
                                uint64_t fact = 1;
                                for (int i = 2; i <= num; ++i) fact *= i; 
                                write("Факториал: " + std::to_string(fact) + "\n");
                            } catch (...) {
                                write("Ошибка: Введите число или 'таймер N'\n");
                            }
                        });
                    }
                }
            });
    }

    // Запуск таймера на указанное количество секунд
    void start_timer(int seconds) {
        timer_.expires_after(asio::chrono::seconds(seconds)); 
        auto self(shared_from_this());
        timer_.async_wait([this, self](const boost::system::error_code& ec) {
            if (!ec) {
                write("Прошло " + std::to_string(seconds) + " секунд!\n");
            }
        });
    }

    // Асинхронная отправка сообщения клиенту
    void write(const std::string& msg) {
        auto self(shared_from_this());
        asio::async_write(socket_, asio::buffer(msg),
            [this, self](boost::system::error_code ec, size_t) {
                if (!ec) read(); // Цикл чтения после успешной отправки
            });
    }

    tcp::socket socket_;
    asio::steady_timer timer_;  
    asio::io_context& io_;      
    asio::streambuf buffer_;    // Буфер для хранения входных данных
};

// Класс TCP-сервера
class Server {
public:
    Server(asio::io_context& io, short port)
        : acceptor_(io, tcp::endpoint(tcp::v4(), port)) { // Инициализация акцептора
        accept(); // Начало приема подключений
    }

private:
    // Асинхронный прием новых подключений
    void accept() {
        acceptor_.async_accept(
            [this](boost::system::error_code ec, tcp::socket socket) {
                if (!ec) {
                    // Создание сессии для нового клиента
                    std::make_shared<Session>(std::move(socket), 
                        acceptor_.get_executor().context())->start();
                }
                accept(); // Рекурсивный вызов для следующего подключения
            });
    }

    tcp::acceptor acceptor_; // Акцептор входящих подключений
};

int main() {
    try {
        asio::io_context io; 
        Server server(io, 12345); 
        
        // Пул из 4 потоков для обработки запросов
        std::vector<std::thread> threads;
        for (int i = 0; i < 4; ++i) {
            threads.emplace_back([&io] { 
                io.run(); // Запуск обработки событий в каждом потоке
            });
        }
        
        // Ожидание завершения всех потоков
        for (auto& t : threads) t.join();
    } catch (std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << "\n";
    }
}
