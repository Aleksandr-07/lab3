#include <boost/asio.hpp>
#include <iostream>

using boost::asio::ip::tcp;

int main() {
    try {
        boost::asio::io_context io;
        
        tcp::socket socket(io);
        tcp::resolver resolver(io);
        
        boost::asio::connect(
            socket, 
            resolver.resolve("127.0.0.1", "12345") // Разрешение адреса
        );
        std::cout << "Успешно подключено к серверу\n";

        // Ввод данных от пользователя
        std::cout << "Enter number: ";
        std::string input;
        std::getline(std::cin, input); 
        input += "\n"; // Добавление разделителя сообщений

        // Отправка данных на сервер
        boost::asio::write(
            socket, 
            boost::asio::buffer(input) // Безопасная передача данных без копирования
        );

        // Получение ответа от сервера
        boost::asio::streambuf response; // Динамический буфер для ответа
        boost::asio::read_until(
            socket, 
            response, 
            '\n' // Чтение до разделителя сообщений
        );

        // Извлечение данных из буфера
        std::istream is(&response);
        std::string result;
        std::getline(is, result); // Чтение строки (без символа '\n')
        
        std::cout << "Result: " << result << "\n";

    } catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
    return 0;
}
