#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <semaphore>
#include <random>
#include <chrono>
#include <atomic>
#include <algorithm>

std::counting_semaphore<10> cranes(5); // Максимум 10 кранов, изначально 5
std::queue<int> truck_queue;
std::mutex queue_mutex;
std::mutex cout_mutex;
std::atomic<int> loaded_trucks(0);
std::atomic<bool> emergency_mode(false);
std::atomic<int> available_cranes(5); // Отслеживаем количество доступных кранов
std::atomic<bool> crane_added(false); // Флаг, что кран уже добавлен

void truck(int id) {
    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "Грузовик " << id << " прибыл в порт и ожидает загрузки.\n";
    }

    cranes.acquire();
    available_cranes--; // Уменьшаем количество доступных кранов

    {
        std::lock_guard<std::mutex> lock(queue_mutex);
        // Удаляем грузовик из очереди, когда он начинает загрузку
        std::queue<int> temp_queue;
        while (!truck_queue.empty()) {
            if (truck_queue.front() != id) {
                temp_queue.push(truck_queue.front());
            }
            truck_queue.pop();
        }
        truck_queue = temp_queue; // Заменяем очередь
    }

    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "Грузовик " << id << " начал загрузку.\n";
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> time_dist(3, 6);
    int load_time = emergency_mode ? time_dist(gen) / 2 : time_dist(gen);

    std::this_thread::sleep_for(std::chrono::seconds(load_time));

    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "Грузовик " << id << " завершил загрузку за " << load_time << " сек.\n";
    }

    loaded_trucks++;
    cranes.release();
    available_cranes++; // Увеличиваем количество доступных кранов
}

void monitor() {
    while (loaded_trucks < 10) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        {
            std::lock_guard<std::mutex> lock(queue_mutex);

            // Активируем резервный кран, если в очереди больше 5 и есть доступные и ещё не добавлен
            if (truck_queue.size() > 5 && available_cranes < 10 && !crane_added) {
                try {
                    //if (cranes.try_acquire_for(std::chrono::milliseconds(1))) {
                    //     cranes.release();  //вернули
                    //} else {
                    //    continue; //не получилось добавить
                    //}
                    cranes.release();  //добавили
                    available_cranes++; //Увеличили
                    crane_added = true; // Ставим флаг

                    std::lock_guard<std::mutex> lock_cout(cout_mutex);
                    std::cout << "Активирован резервный кран! Доступно: " << available_cranes.load() << " кранов.\n";
                } catch (const std::system_error& e) {
                    // Обработка ошибки, если не удалось добавить кран (например, достигнут максимум)
                    std::lock_guard<std::mutex> lock_cout(cout_mutex);
                    std::cerr << "Ошибка добавления крана: " << e.what() << std::endl;
                }
            }

            // Сбрасываем флаг, если очередь уменьшилась
            if (truck_queue.size() <= 5) {
                crane_added = false;
            }
        }

        if (loaded_trucks.load() < 3 && !emergency_mode) {
            emergency_mode = true;
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cout << "Аварийная загрузка активирована!\n";
        } else if (loaded_trucks.load() >= 3 && emergency_mode) {
            emergency_mode = false;
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cout << "Аварийная загрузка отключена\n";
        }
    }
}

int main() {
    std::thread monitoring_thread(monitor);

    std::vector<std::thread> trucks;
    for (int i = 1; i <= 10; ++i) {
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            truck_queue.push(i);
        }
        trucks.emplace_back(truck, i);
    }

    for (auto& t : trucks) t.join();
    monitoring_thread.join();

    return 0;
}