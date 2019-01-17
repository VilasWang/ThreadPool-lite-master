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
	_task_cv.notify_all();		// ���������߳�ִ��
	waitForDone();
}

void threadpool::waitForDone()
{
	for (thread& thread : _pool)
	{
		if (thread.joinable())
			thread.join();		// �ȴ���������� ǰ�᣺�߳�һ����ִ����
	}
}

void threadpool::detachAll()
{
	for (thread& thread : _pool)
	{
		thread.detach();		// ���̡߳���������
	}
}

void threadpool::addThread(unsigned short size)
{
	//�����߳�����, �������� Ԥ�������� THREADPOOL_MAX_NUM
	for (; _pool.size() < THREADPOOL_MAX_NUM && size > 0; --size)
	{
		//�̺߳���
		auto threadFunc = [this] {
			while (_running)
			{
				Task task; // ��ȡһ����ִ�е� task
				{
					// wait ֱ���� task
					// unique_lock ��� lock_guard �ĺô��ǣ�������ʱ unlock() �� lock()
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
				task();//ִ������
				_idlThrNum++;
			}
		};

		_pool.emplace_back(move(threadFunc));
		_idlThrNum++;
	}
}