#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>
#include <vector>
#include <chrono>

int counter = 0; // Общий счётчик
std::atomic<int> atomic_counter(0); // Атомарный счётчик
std::mutex mtx; // Мьютекс для синхронизации

// Функция для увеличения счётчика без синхронизации
void increment_unsafe(int iterations) {
    for (int i = 0; i < iterations; ++i) {
        ++counter;
    }
}

// Функция для увеличения атомарного счётчика
void increment_atomic(int iterations) {
    for (int i = 0; i < iterations; ++i) {
        ++atomic_counter;
    }
}

// Функция для увеличения счётчика с использованием мьютекса
void increment_mutex(int iterations) {
    for (int i = 0; i < iterations; ++i) {
        std::lock_guard<std::mutex> lock(mtx);
        ++counter;
    }
}

int main() {
    const int iterations = 1000000; // Количество итераций
    const int num_threads = 2; // Количество потоков

    // Без синхронизации
    auto start_time = std::chrono::high_resolution_clock::now();
    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(increment_unsafe, iterations);
    }
    for (auto& t : threads) {
        t.join();
    }
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    std::cout << "Счетчик без синхронизации: " << counter << ", время: " << duration.count() << " ms" << std::endl;

    // С использованием std::atomic
    counter = 0; // Сброс счётчика
    start_time = std::chrono::high_resolution_clock::now();
    threads.clear();
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(increment_atomic, iterations);
    }
    for (auto& t : threads) {
        t.join();
    }
    end_time = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    std::cout << "Счетчик с использованием std::atomic: " << atomic_counter << ", время: " << duration.count() << " ms" << std::endl;

    // С использованием std::mutex
    counter = 0; // Сброс счётчика
    start_time = std::chrono::high_resolution_clock::now();
    threads.clear();
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(increment_mutex, iterations);
    }
    for (auto& t : threads) {
        t.join();
    }
    end_time = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    std::cout << "Счетчик с использованием std::mutex: " << counter << ", время: " << duration.count() << " ms" << std::endl;

    return 0;
}