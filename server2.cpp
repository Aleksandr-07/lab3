#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <sstream>
#include <thread>

using boost::asio::ip::tcp;
namespace asio = boost::asio;

// Класс для обработки отдельного соединения с клиентом
class Session : public std::enable_shared_from_this<Session> {
public:
    Session(tcp::socket socket, asio::io_context& io)
        : socket_(std::move(socket)), io_(io) {}

    void start() {
        read(); // Запуск цикла чтения данных
    }

private:
    void read() {
        auto self(shared_from_this()); // Захват shared_ptr для продления времени жизни сессии
        asio::async_read_until(socket_, buffer_, '\n',
            [this, self](boost::system::error_code ec, size_t len) {
                if (!ec) {
                    // Преобразование буфера в строку
                    std::string data{
                        asio::buffers_begin(buffer_.data()),
                        asio::buffers_begin(buffer_.data()) + len
                    };
                    buffer_.consume(len); // Очистка обработанных данных из буфера
                    
                    // Асинхронная обработка в контексте ввода-вывода
                    asio::post(io_, [this, data]() {
                        try {
                            // Вычисление факториала
                            int num = std::stoi(data);
                            uint64_t fact = 1;
                            for (int i = 2; i <= num; ++i) fact *= i;
                            write("Factorial: " + std::to_string(fact) + "\n");
                        } catch (...) {
                            write("Error: Invalid input\n");
                        }
                    });
                }
            });
    }

    void write(const std::string& msg) {
        auto self(shared_from_this());
        // Асинхронная отправка ответа
        asio::async_write(socket_, asio::buffer(msg),
            [this, self](boost::system::error_code ec, size_t) {
                if (!ec) read(); // Повторное чтение после успешной отправки
            });
    }

    tcp::socket socket_;
    asio::io_context& io_;     // Референс на контекст ввода-вывода
    asio::streambuf buffer_;   // Буфер для хранения входных данных
};

// Класс TCP-сервера
class Server {
public:
    Server(asio::io_context& io, short port)
        : acceptor_(io, tcp::endpoint(tcp::v4(), port)) {
        accept(); // Запуск процесса принятия соединений
    }

private:
    void accept() {
        // Асинхронное принятие нового соединения
        acceptor_.async_accept(
            [this](boost::system::error_code ec, tcp::socket socket) {
                if (!ec) {
                    // Создание сессии для нового клиента
                    std::make_shared<Session>(std::move(socket), 
                        acceptor_.get_executor().context())->start();
                }
                accept(); // Рекурсивный вызов для принятия следующего соединения
            });
    }

    tcp::acceptor acceptor_; // Акцептор для входящих соединений
};

int main(int argc, char* argv[]) {
    try {
        asio::io_context io; // Главный контекст ввода-вывода
        
        Server server(io, 12345); // Создание сервера на порту 12345
        
        // Создание пула потоков для обработки задач
        size_t threads = std::thread::hardware_concurrency(); // Оптимальное число потоков
        std::vector<std::thread> pool;
        for (size_t i = 0; i < threads; ++i)
            pool.emplace_back([&io] { io.run(); }); // Запуск обработчиков в каждом потоке
        
        for (auto& t : pool) t.join(); // Ожидание завершения всех потоков
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
}
