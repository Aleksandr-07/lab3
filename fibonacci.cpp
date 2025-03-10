#include <iostream>
#include <boost/thread.hpp>
#include <boost/chrono.hpp>

// Рекурсивная функция вычисления числа Фибоначчи
int fibonacci(int n) {
    if (n <= 1) return n;
    return fibonacci(n - 1) + fibonacci(n - 2);
}

// Функция для многопоточернр вычтсления
void thread_function(int m) {
    int result = fibonacci(m);
    std::cout << "Поток " << boost::this_thread::get_id() << " для числа Фибоначчи(" << m << ") = " << result << std::endl;
}

int main() {
    int N = 4; // Количество потоков
    int M = 40; // Номер числа Фибоначчи для вычисления

    boost::thread_group threads;

    // Запуск N потоков
    auto start_time = boost::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; ++i) {
        threads.create_thread(boost::bind(thread_function, M));
    }

    // Ожидание завершения всех потоков
    threads.join_all();
    auto end_time = boost::chrono::high_resolution_clock::now();

    auto duration = boost::chrono::duration_cast<boost::chrono::milliseconds>(end_time - start_time);
    std::cout << "Время выполнения многопоточного вычисления: " << duration.count() << " ms" << std::endl;

    // Последовательное вычисление
    start_time = boost::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; ++i) {
        int result = fibonacci(M);
        std::cout << "Последовательное вычисление числа Фибоначчи(" << M << ") = " << result << std::endl;
    }
    end_time = boost::chrono::high_resolution_clock::now();

    duration = boost::chrono::duration_cast<boost::chrono::milliseconds>(end_time - start_time);
    std::cout << "Время выполнения последовательного вычисления: " << duration.count() << " ms" << std::endl;

    return 0;
}