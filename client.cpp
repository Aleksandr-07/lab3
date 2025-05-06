#include <boost/asio.hpp>
#include <iostream>
#include <string>

using namespace boost::asio;
using namespace boost::asio::ip;

int main() {
    try {
        io_context io;
        
        // Создание TCP-сокета и резолвера для преобразования адресов
        tcp::socket socket(io);
        tcp::resolver resolver(io);

        connect(socket, resolver.resolve("127.0.0.1", "12345"));
        std::cout << "Успешно подключено к серверу" << std::endl;

        // Отправка сообщения
        std::string message;
        std::cout << "Введите сообщение: ";
        std::getline(std::cin, message);  // Чтение всей строки
        message += "\n";  // Добавление разделителя сообщений
        
        // Синхронная отправка данных через сокет
        write(socket, buffer(message));

        // Получение ответа 
        streambuf response;  // Буфер для приема данных
        read_until(socket, response, '\n');  // Чтение до разделителя
        
        // Преобразование буфера в строку
        std::istream stream(&response);
        std::string reply;
        std::getline(stream, reply);  // Извлечение строки без символа '\n'

        std::cout << "Ответ сервера: " << reply << std::endl;

    } catch (std::exception& e) {
        std::cerr << "Ошибка клиента: " << e.what() << std::endl;
    }
    return 0;
}
