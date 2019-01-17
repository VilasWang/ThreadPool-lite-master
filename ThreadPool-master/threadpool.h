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

	//�̳߳� (�����ύ��κ�����lambda���ʽ����������ִ��, ����������ȡ����ֵ)
	class threadpool
	{
	public:
		threadpool(unsigned short size = 4);
		~threadpool();

	public:
		// ��ʼһ���������п����̣߳�����ִ�У����򣬼���ȴ����У�
		// ����ֵ����std::future<T>
		// ����std::future<T>.get()�����ȴ�����ִ�����ȡ����ֵ
		// �����ַ�������ʵ�ֵ������Ա:
		// һ����ʹ��	std::bind�� std::threadpool.start(std::bind(&Dog::sayHello, &dog));
		// һ������   std::mem_fn�� std::threadpool.start(std::mem_fn(&Dog::sayHello), this)
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
		queue<Task> _tasks;			//�������
		mutex _lock;
		condition_variable _task_cv;//��������
		atomic<bool> _running;		//�̳߳��Ƿ�ִ��
		atomic<int>  _idlThrNum;	//�����߳�����
	};
}

#endif  //https://github.com/lzpong/