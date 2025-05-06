#include <boost/asio.hpp>
#include <iostream>

using boost::asio::ip::tcp;

int main() {
    try {
        boost::asio::io_context io;
        tcp::socket socket(io);
        tcp::resolver resolver(io);
        boost::asio::connect(socket, resolver.resolve("127.0.0.1", "12345"));
        
        std::cout << "Введите команду ('таймер N' или число): ";
        std::string input;
        std::getline(std::cin, input);
        input += "\n";
        
        boost::asio::write(socket, boost::asio::buffer(input));
        
        boost::asio::streambuf response;
        boost::asio::read_until(socket, response, '\n');
        
        std::istream is(&response);
        std::string result;
        std::getline(is, result);
        std::cout << "Ответ: " << result << "\n";
    } catch (std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << "\n";
    }
    return 0;
}
