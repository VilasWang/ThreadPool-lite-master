#include "threadpool.h"

using namespace std;

threadpool::threadpool(unsigned short size)
	: _running(true)
	, _idlThrNum(0)
{
	addThread(size);
}

threadpool::~threadpool()
{
	_running = false;
	_task_cv.notify_all();		// 唤醒所有线程执行
	waitForDone();
}

void threadpool::waitForDone()
{
	for (thread& thread : _pool)
	{
		if (thread.joinable())
			thread.join();		// 等待任务结束， 前提：线程一定会执行完
	}
}

void threadpool::detachAll()
{
	for (thread& thread : _pool)
	{
		thread.detach();		// 让线程“自生自灭”
	}
}

void threadpool::addThread(unsigned short size)
{
	//增加线程数量, 但不超过 预定义数量 THREADPOOL_MAX_NUM
	for (; _pool.size() < THREADPOOL_MAX_NUM && size > 0; --size)
	{
		//线程函数
		auto threadFunc = [this] {
			while (_running)
			{
				Task task; // 获取一个待执行的 task
				{
					// wait 直到有 task
					// unique_lock 相比 lock_guard 的好处是：可以随时 unlock() 和 lock()
					unique_lock<mutex> lock(_lock);
					_task_cv.wait(lock, [this] {
						return !_running || !_tasks.empty();
					});
					if (!_running && _tasks.empty())
						return;
					task = move(_tasks.front());
					_tasks.pop();
				}
				_idlThrNum--;
				task();//执行任务
				_idlThrNum++;
			}
		};

		_pool.emplace_back(move(threadFunc));
		_idlThrNum++;
	}
}