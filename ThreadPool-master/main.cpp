#include "threadpool.h"
#include <iostream>
#include <windows.h>


void fun1(int slp)
{
	printf("  hello, fun1 !  %d\n", std::this_thread::get_id());
	if (slp > 0)
	{
		printf(" ======= fun1 sleep %d  =========  %d\n", slp, std::this_thread::get_id());
		std::this_thread::sleep_for(std::chrono::milliseconds(slp));
		//Sleep(slp );
	}
}

struct gfun
{
	int operator()(int n)
	{
		printf("%d  hello, gfun !  %d\n", n, std::this_thread::get_id());
		return 42;
	}
};

class A
{    //函数必须是 static 的才能使用线程池
public:
	static int Afun(int n = 0)
	{
		std::cout << n << "  hello, Afun !  " << std::this_thread::get_id() << std::endl;
		return n;
	}

	static std::string Bfun(int n, std::string str, char c)
	{
		std::cout << n << "  hello, Bfun !  " << str.c_str() << "  " << (int)c << "  " << std::this_thread::get_id() << std::endl;
		return str;
	}
};

int main()
{
	try
	{
#ifdef Test
		std::threadpool pool(16);
		std::future<void> ff = pool.start(fun1, 0);
		std::future<int> fg = pool.start(gfun(), 0);
		std::future<std::string> gh = pool.start(A::Bfun, 9998, "multi args", 123);
		std::future<std::string> fh = pool.start([]()->std::string { std::cout << "hello, fh !  " << std::this_thread::get_id() << std::endl; return "hello,fh ret !"; });

		std::cout << " =======  sleep ========= " << std::this_thread::get_id() << std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(3));

		std::cout << fg.get() << "  " << fh.get().c_str() << "  " << std::this_thread::get_id() << std::endl;

		std::cout << " =======  sleep ========= " << std::this_thread::get_id() << std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(3));

		std::cout << " =======  fun1,55 ========= " << std::this_thread::get_id() << std::endl;
		pool.start(fun1, 55).get();    //调用.get()获取返回值会等待线程执行完

		std::cout << "end... " << std::this_thread::get_id() << std::endl;
#endif

#ifdef Test
		std::cout << " =======  sleep ========= " << std::this_thread::get_id() << std::endl;
		std::this_thread::sleep_for(std::chrono::microseconds(500));

		std::cout << " =======  commit all 1 begin ========= " << std::this_thread::get_id() << std::endl;
		for (int i = 0; i < 50; i++)
		{
			pool.start(fun1, i * 100);
		}
		std::cout << " =======  commit all 1 end ========= " << std::this_thread::get_id() << " idlesize = " << pool.idleCount() << std::endl;
#endif

		std::cout << " =======  commit all 2 begin ========= " << std::this_thread::get_id() << std::endl;
		std::threadpool pool2(4);
		std::vector< std::future<int> > results;
		for (int i = 0; i < 8; ++i)
		{
			results.emplace_back(
				pool2.start([i] {
				std::cout << "hello " << i << std::endl;
				std::this_thread::sleep_for(std::chrono::seconds(1));
				std::cout << "world " << i << std::endl;
				return i*i;
			})
			);
		}
		std::cout << " =======  commit all 2 end ========= " << std::this_thread::get_id() << std::endl;

		for (auto && result : results)
		{
			std::cout << "result: " << result.get() << std::endl;
		}


		system("pause");
		return 0;
	}
	catch (std::exception& e)
	{
		std::cout << "some exception happened...  " << std::this_thread::get_id() << e.what() << std::endl;
	}
}
