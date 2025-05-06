// client.cpp
#include <boost/asio.hpp>
#include <iostream>
#include <string>

using namespace boost::asio;
using namespace boost::asio::ip;

int main() {
    try {
        io_context io;
        tcp::socket socket(io);
        tcp::resolver resolver(io);

        // Подключение к серверу
        connect(socket, resolver.resolve("127.0.0.1", "12345"));

        // Отправка сообщения
        std::string message;
        std::cout << "Введите сообщение: ";
        std::getline(std::cin, message);
        message += "\n"; // Добавляем разделитель
        write(socket, buffer(message));

        // Получение ответа
        streambuf response;
        read_until(socket, response, '\n');
        std::istream stream(&response);
        std::string reply;
        std::getline(stream, reply);

        std::cout << "Ответ сервера: " << reply << std::endl;
    } catch (std::exception& e) {
        std::cerr << "Ошибка клиента: " << e.what() << std::endl;
    }
    return 0;
}