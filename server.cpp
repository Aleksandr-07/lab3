#include <boost/asio.hpp>
#include <iostream>
#include <string>

using namespace boost::asio;
using namespace boost::asio::ip;

int main() {
    try {
        io_context io;
        tcp::acceptor acceptor(io, tcp::endpoint(tcp::v4(), 12345));
        std::cout << "Сервер запущен. Ожидание подключений..." << std::endl;

        for (;;) {
            tcp::socket socket(io);
            acceptor.accept(socket); // Блокирующее ожидание клиента

            // Чтение сообщения от клиента
            streambuf buffer;
            read_until(socket, buffer, '\n');
            std::istream stream(&buffer);
            std::string message;
            std::getline(stream, message);

            // Формирование ответа
            std::string response = "Сообщение получено: " + message + "\n";
            write(socket, boost::asio::buffer(response));  // Отправка ответа

            std::cout << "Клиент отправил: " << message << std::endl;
        }
    } catch (std::exception& e) {
        std::cerr << "Ошибка сервера: " << e.what() << std::endl;
    }
    return 0;
}