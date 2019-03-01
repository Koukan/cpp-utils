#ifndef _KOUKAN_THREADPOOL_HPP_
#define _KOUKAN_THREADPOOL_HPP_

#include <deque>
#include <mutex>
#include <thread>
#include <atomic>
#include <functional>
#include <condition_variable>

namespace koukan
{

/*!
 \brief Threadpool class
 */
class ThreadPool
{
  public:
	/*!
		\brief Construct a Threadpool
		\param nbThread Number of initial threads in threadpool
		\details if 0, then the number of the thread is equal to the number of logic processors on the computer
	*/
    ThreadPool(size_t nbThread = 0);
	/*!
		\brief Destructor of Threadpool
		\details call terminate method
	*/
    ~ThreadPool();
	/*!
		\brief Add a task to perform asynchronously 
		\param task Task to be executed
	*/
	void		pushTask(std::function<void()> task);
	/*!
		\brief Add a task to perform asynchronously in the first position
		\param task Task to be executed with a high priority
	*/
	void		pushPriorityTask(std::function<void()> task);
	/*!
		\brief Get the number of waiting tasks
		\return the number of waiting tasks
	*/
	size_t		size() const;
	/*!
		\brief	Add dynamically one or more threads to the threadpool 
		\details No task is lost
		\param nb Number of threads to add
	*/
	void		addThread(size_t nb = 1);
	/*!
		\brief Remove dynamically one or more threads to the threadpool
		\details No task is lost
		\details This function do not wait threads to terminate
		\param nb Numbers of thread to remove
	*/
	void		removeThread(size_t nb = 1); // non blocking
	/*!
		\return the number of threads
	*/
	size_t		getNbThread() const;
	/*!
		\brief Wait until all tasks are completed
	*/
	void		wait();
	/*!
		\brief Terminate the threadpool
		\details All tasks not performed are lost
		\details This function blocks until all thread return
	*/
	void		terminate();

  private:
	void		handleTask();

	std::deque<std::thread>				_threads;
	std::deque<std::function<void()>>	_tasks;
	std::condition_variable				_condition_variable;
	std::mutex							_mutex;
	std::condition_variable				_waiting_condition_variable;
	std::mutex							_waiting_mutex;
	std::atomic<int>					_working_thread_counter;
};



// Definitions
thread_local static bool gl_continue = true;

inline ThreadPool::ThreadPool(size_t nbThread)
{
	if (nbThread == 0)
	{
		nbThread = std::thread::hardware_concurrency();
		if (nbThread > 1)
			nbThread--;
	}
	this->addThread(nbThread);
}

inline ThreadPool::~ThreadPool()
{
	this->terminate();
}

inline void ThreadPool::pushTask(std::function<void()> task)
{
	std::lock_guard lock(_mutex);
	_tasks.emplace_back(task);
	_condition_variable.notify_one();
}

inline void ThreadPool::pushPriorityTask(std::function<void()> task)
{
	std::lock_guard lock(_mutex);
	_tasks.emplace_front(task);
	_condition_variable.notify_one();
}

inline size_t ThreadPool::size() const
{
	return _tasks.size();
}

inline void ThreadPool::addThread(size_t nb)
{
	for (; nb > 0; nb--)
		_threads.emplace_back([this] { this->handleTask(); });
}

inline void ThreadPool::removeThread(size_t nb)
{
	nb = (nb < _threads.size()) ? nb : _threads.size();
	for (size_t i = 0; i < nb; ++i)
		this->pushPriorityTask([] { gl_continue = false; });
}

inline size_t ThreadPool::getNbThread() const
{
	return _threads.size();
}

inline void ThreadPool::wait()
{
	std::unique_lock lock(_waiting_mutex);
	_waiting_condition_variable.wait(lock, [this] { return _tasks.empty(); });
}

inline void ThreadPool::terminate()
{
	_mutex.lock();
	_tasks.clear();
	_mutex.unlock();
	this->removeThread(_threads.size());
	for (auto& thread : _threads)
		thread.join();
	_threads.clear();
	_tasks.clear(); // now safe since all threads joined
}

inline void ThreadPool::handleTask()
{
	while (gl_continue)
	{
		std::unique_lock lock(_mutex);
		if (_tasks.empty() && _working_thread_counter == 0)
			_waiting_condition_variable.notify_all();
		_condition_variable.wait(lock, [this] { return !_tasks.empty(); });

		++_working_thread_counter;
		auto task = std::move(_tasks.front());
		_tasks.pop_front();
		lock.unlock();
		task();
		--_working_thread_counter;
	}
}

}

#endif // _KOUKAN_THREADPOOL_HPP_
