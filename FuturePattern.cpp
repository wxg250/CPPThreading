#include <iostream>
#include <thread>
#include <future>

int main() {
    // Step 1: Create a promise<int>
    std::promise<int> promise;

    // Step 2: Get the future associated with it
    std::future<int> future = promise.get_future();

    // Step 3: Hand the promise to a producer thread
    std::thread producer([&promise] {
        try {
            int result = do_long_work();  // some computed value
            promise.set_value(result);  // put the result into the promise
        } catch (...) {
            promise.set_exception(std::current_exception()); // if error
        }
    });

    // Step 4: Consumer waits for the result
    int value = future.get();  // blocks until promise.set_value is called
    std::cout << "Result = " << value << "\n";å

    producer.join();
}

int main(){
    auto future = std::async(std::launch::async, [] { return do_long_work(); });
    auto result = future.get();
    std::cout << "Result = " << result << "\n";
}
