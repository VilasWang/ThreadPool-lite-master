#include "threadpool.h"
#include <iostream>
#include <windows.h>


void fun1(int msc)
{
	printf("  hello, fun1 !  %d\n", std::this_thread::get_id());
	if (msc > 0)
	{
		printf(" ======= fun1 sleep %d  =========  %d\n", msc, std::this_thread::get_id());
		std::this_thread::sleep_for(std::chrono::milliseconds(msc));
		//Sleep(slp );
	}
}

struct STfun
{
	int operator()(int n)
	{
		printf("%d  hello, STfun !  %d\n", n, std::this_thread::get_id());
		return 42;
	}
};

class A
{
public:
	static std::string fun(int n, std::string str, char c)
	{
		std::cout << n << "  hello, A::fun !  " << str.c_str() << "  " << (int)c << "  " << std::this_thread::get_id() << std::endl;
		return str;
	}
};


template<class F, class... Args>
auto funcWN(F&& f, Args&&... args) ->decltype(f(args...))
{
	return f(args...);
}

int main()
{
	try
	{
#if 1
		using FUN = std::function<void(int)>;
		auto fun2 = [](int msc) ->void {
			printf("  hello, fun2 !  %d\n", std::this_thread::get_id()); 
			if (msc > 0)
			{
				printf(" ======= fun2 sleep %d  =========  %d\n", msc, std::this_thread::get_id());
				std::this_thread::sleep_for(std::chrono::milliseconds(msc));
			}
		};

		//funcWN<FUN, int>(fun2, 500);

		std::threadpool pool(16);
		std::future<void> ff = pool.start(fun2, 500);
		std::future<int> fg = pool.start(STfun(), 0);
		std::future<std::string> gh = pool.start(A::fun, 9998, "multi args", 123);
		std::future<std::string> fh = pool.start([]()->std::string { std::cout << "hello, fh !  " << std::this_thread::get_id() << std::endl; return "hello,fh ret !"; });

		std::cout << " =======  sleep 3 s  ========= " << std::this_thread::get_id() << std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(3));

		std::cout << fg.get() << "  " << fh.get().c_str() << "  " << std::this_thread::get_id() << std::endl;

		std::cout << " =======  sleep 3 s  ========= " << std::this_thread::get_id() << std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(3));

		std::cout << gh.get().c_str();

		std::cout << "end... " << std::this_thread::get_id() << std::endl;
#else

		std::cout << " =======  commit all begin ========= " << std::this_thread::get_id() << std::endl;
		std::threadpool pool2(4);
		std::vector< std::future<int> > results;
		for (int i = 0; i < 50; ++i)
		{
			results.emplace_back(
				pool2.start([i] {
				std::cout << "hello " << i << std::endl;
				std::this_thread::sleep_for(std::chrono::milliseconds(30));
				std::cout << "world " << i << std::endl;
				return i*i;
			})
			);
		}
		std::cout << " =======  commit all end ========= " << std::this_thread::get_id() << std::endl;

		for (auto && result : results)
		{
			std::cout << "result: " << result.get() << std::endl;
		}
#endif

		system("pause");
		return 0;
	}
	catch (std::exception& e)
	{
		std::cout << "some exception happened...  " << std::this_thread::get_id() << e.what() << std::endl;
	}
}
