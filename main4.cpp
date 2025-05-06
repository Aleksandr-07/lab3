#include "server.hpp"
#include <iostream>
#include <cstdlib>

int main(int argc, char* argv[]) {
    try {
        if (argc != 3) {
            std::cerr << "Использование: " << argv[0] << " <port> <threads>\n";
            return 1;
        }

        boost::asio::io_context io_context;
        Server s(io_context, std::atoi(argv[1]), std::atoi(argv[2]));
        s.run();
        
        std::cout << "Сервер запущен. Нажмите Enter для выхода...\n";
        std::cin.get();
        io_context.stop();
        
    } catch (std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << "\n";
    }
    return 0;
}
