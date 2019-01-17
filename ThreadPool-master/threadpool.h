#pragma once
#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>
#include <queue>
#include <atomic>
#include <future>
//#include <condition_variable>
//#include <thread>
//#include <functional>
#include <stdexcept>

namespace std
{
#define  THREADPOOL_MAX_NUM 16
	//#define  THREADPOOL_AUTO_GROW

	//线程池 (可以提交变参函数或lambda表达式的匿名函数执行, 可以阻塞获取返回值)
	class threadpool
	{
	public:
		threadpool(unsigned short size = 4);
		~threadpool();

	public:
		// 开始一个任务（若有空闲线程，立即执行，否则，加入等待队列）
		// 返回值类型std::future<T>
		// 调用std::future<T>.get()阻塞等待任务执行完获取返回值
		// 有两种方法可以实现调用类成员:
		// 一种是使用	std::bind： std::threadpool.start(std::bind(&Dog::sayHello, &dog));
		// 一种是用   std::mem_fn： std::threadpool.start(std::mem_fn(&Dog::sayHello), this)
		template<class F, class... Args>
		auto start(F&& f, Args&&... args) ->future<decltype(f(args...))>
		{
			if (!_running)    // stoped ??
				throw runtime_error("commit on ThreadPool is stopped.");

			using RetType = decltype(f(args...)); // typename std::result_of<F(Args...)>::type
			auto task = make_shared<packaged_task<RetType()>>(
				bind(forward<F>(f), forward<Args>(args)...)
				);

			//std::cout << typeid(task).name() << std::endl;

			future<RetType> future = task->get_future();
			{
				lock_guard<mutex> lock(_lock);
				_tasks.emplace([task]() {
					(*task)();
				});
			}
#ifdef THREADPOOL_AUTO_GROW
			if (_idlThrNum < 1 && _pool.size() < THREADPOOL_MAX_NUM)
				addThread(1);
#endif // !THREADPOOL_AUTO_GROW

			_task_cv.notify_one();

			return future;
		}

		void waitForDone();
		void detachAll();

		int idleCount() { return _idlThrNum; }
		int threadCount() { return _pool.size(); }

#ifndef THREADPOOL_AUTO_GROW
	private:
#endif // !THREADPOOL_AUTO_GROW
		void addThread(unsigned short size);

	private:
		using Task = function<void()>;
		vector<thread> _pool;
		queue<Task> _tasks;			//任务队列
		mutex _lock;
		condition_variable _task_cv;//条件阻塞
		atomic<bool> _running;		//线程池是否执行
		atomic<int>  _idlThrNum;	//空闲线程数量
	};
}

#endif  //https://github.com/lzpong/