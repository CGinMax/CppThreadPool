#include <iostream>
#include <chrono>
#include "ThreadPool.hpp"
#include "Cpp11ThreadPool.hpp"
#include <cstdio>

void Test()
{
	ThreadPool pool(2);
	for (int i = 0; i < 20; ++i) {
		pool.AddTask([i](){
				auto curId = std::this_thread::get_id();
				//std::cout << "Current Thread number: " << i << '\n';
				printf("Current THread ID: %d\n", i);
				//printf("Current Thread ID: %ld\n", curId);
				//std::cout << "Current Thread ID: " << curId << '\n';
				});
	}
	std::this_thread::sleep_for(std::chrono::seconds(2));
//	pool.Stop();
}

void TestCpp11ThreadPool()
{
	static Cpp11ThreadPool pool(6);

	auto res1 = pool.add([]{
		int sum = 0;
		for (int i = 0; i < 10000; i++) {
			sum += i;
		}
		return sum;
	});
	auto res2 = pool.add([]{
		int sum = 0;
		for (int i = 0; i < 10000; i++) {
			sum += i;
		}
		return sum;
	});
	auto res3 = pool.add([]{
		int sum = 0;
		for (int i = 0; i < 10000; i++) {
			sum += i;
		}
		return sum;
	});
	auto res4 = pool.add([]{
		int sum = 0;
		for (int i = 0; i < 10000; i++) {
			sum += i;
		}
		return sum;
	});

	std::cout <<  res1.get() << "\n";
	std::cout <<  res2.get() << "\n";
	std::cout <<  res3.get() << "\n";
	std::cout <<  res4.get() << "\n";

}

int main()
{
	// Test();
	TestCpp11ThreadPool();


	return 0;
}
