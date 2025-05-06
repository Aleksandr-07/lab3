#include <boost/asio.hpp>
#include <iostream>
#include <string>

using namespace boost::asio;
using namespace boost::asio::ip;

int main() {
    try {
        // Инициализация контекста ввода-вывода 
        io_context io;
        
        // Создание акцептора для прослушивания порта 12345 на всех IPv4-интерфейсах
        tcp::acceptor acceptor(io, tcp::endpoint(tcp::v4(), 12345));
        std::cout << "Сервер запущен. Ожидание подключений..." << std::endl;

        // Бесконечный цикл обработки подключений
        for (;;) {
            // Создание сокета для нового подключения
            tcp::socket socket(io);
            
            // Блокирующее ожидание нового подключения
            acceptor.accept(socket);
            std::cout << "Новое подключение установлено" << std::endl;

            // Чтение данных от клиента
            streambuf buffer;  // Буфер для накопления данных
            
            // Чтение данных до символа перевода строки (включая его)
            read_until(socket, buffer, '\n');
            
            // Преобразование буфера в поток для удобного чтения
            std::istream stream(&buffer);
            std::string message;
            std::getline(stream, message);  // Извлечение строки из потока

            // Формирование и отправка ответа
            std::string response = "Сообщение получено: " + message + "\n";
            
            // Отправка ответа через сокет (синхронная запись)
            write(socket, boost::asio::buffer(response));
            
            // Логирование в консоль сервера
            std::cout << "Клиент отправил: " << message << std::endl;
            

        }
    } catch (std::exception& e) {
        std::cerr << "Ошибка сервера: " << e.what() << std::endl;
    }
    return 0;
}
