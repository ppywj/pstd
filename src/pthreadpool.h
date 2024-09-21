#ifndef PSTD_THREADPOOL
#define PSTD_THREADPOOL
#include<vector>
#include<queue>
#include<memory>
#include<atomic>
#include<thread>
#include<functional>
#include<chrono>
#include<condition_variable>
#include<unordered_map>
#include<iostream>
#include<future>
#define THREAD_MAX_IDLE_TIME 60;
const int defaultTaskQueMaxSize = 1024;
namespace pstd {
	//任务抽象基类

	//线程池模式
	enum class   PThreadPoolMode {
		MODE_FIXED,//固定数量的线程
		MODE_CACHED//线程数量可动态增长
	};

	//线程类
	class  Thread {
	public:
		using ThreadFunc = std::function<void(unsigned long long)>;
		Thread(ThreadFunc func) :func_(func), threadId(generateId++){ }
		~Thread() {}
		void start()
		{
			std::thread t(func_, threadId);
			t.detach();
		}
		unsigned long long getThreadId()
		{
			return threadId;
		}
	private:
		static unsigned long long generateId;
		unsigned long long  threadId;
		ThreadFunc func_;
	};
	unsigned long long Thread::generateId = 0;
	//线程池类
	class  PThreadPool
	{
	public:
		PThreadPool() :
			threadMinSize(0),
			threadMaxSize(0),
			taskQueMaxSize_(defaultTaskQueMaxSize),
			taskNum_(0),
			threadNum_(0),
			mode_(PThreadPoolMode::MODE_FIXED),
			isPoolRunning(false),
			freeThreadNum(0)
		{

		}
		~PThreadPool()
		{
			isPoolRunning = false;
			std::unique_lock<std::mutex>locker(taskMutex_);
			//std::cout << "析构函数获得" << "获取任务队列锁\n";
			cd_notEmpty_.notify_all();
			exitCond_.wait(locker, [&]() -> bool {return threadMap_.size() == 0; });
		}
		PThreadPool(const PThreadPool&) = delete;
		PThreadPool& operator=(const PThreadPool&) = delete;
		//设置线程池模式
		void setMode(PThreadPoolMode mode)
		{
			if (checkState())
				return;
			this->mode_ = mode;
		}
		//启动
		void start(int initThreadNum = std::thread::hardware_concurrency())
		{
			if (checkState())
				return;
			isPoolRunning = true;
			this->threadNum_ = initThreadNum;
			for (int i = 0; i < initThreadNum; i++)
			{
				auto ptr = std::make_unique<Thread>(std::bind(&PThreadPool::threadFunc, this, std::placeholders::_1));
				threadMap_.emplace(ptr->getThreadId(), std::move(ptr));
			}
			for (int i = 0; i < threadNum_; i++)
			{
				threadMap_[i]->start(); // 需要去执行一个线程函数
				freeThreadNum++;//空闲线程数量更新
			}
		}
		//设置任务队列最大任务数
		void setTaskQueMaxSize(unsigned int size)
		{
			if (checkState())
				return;
			taskQueMaxSize_ = size;
		}
		//设置cached模式下的线程数量下限
		void setMinThreadSize(unsigned int minSize)
		{
			if (isPoolRunning)
				return;
			threadMinSize = minSize;
		}
		//设置cached模式下的线程数量上限
		void setMaxThreadSize(unsigned int maxSize)
		{
			if (checkState())
				return;
			threadMaxSize = maxSize;
		}
		//提交任务到任务队列
		template<typename Func, typename... Args>
		auto submitTask(Func&& func, Args&&... args) -> std::future<decltype(func(args...))>
		{
			using RType = decltype(func(args...));
			auto task = std::make_shared<std::packaged_task<RType()>>(
				std::bind(std::forward<Func>(func),std::forward<Args>(args)...));
			std::future<RType>result = task->get_future();
			//获取锁
			std::unique_lock<std::mutex>locker(taskMutex_);
			//判断条件 若满则等待不满条件
			if (!cd_notFull_.wait_for(locker, std::chrono::seconds(1), [&]()->bool {return taskNum_ < taskQueMaxSize_; }))
			{
				auto task= std::make_shared<std::packaged_task<RType()>>([]()->RType { return RType(); });
				(*task)();
				return task->get_future();
			}
			//添加任务
			taskQue_.emplace([task]() {(*task)();});
			taskNum_++;
			//通知不空
			cd_notEmpty_.notify_all();
			if (mode_ == pstd::PThreadPoolMode::MODE_CACHED
				&& threadNum_ < threadMaxSize
				&& taskNum_>freeThreadNum)
			{
				std::cout << "新创建一个线程" << std::endl;
				//创建线程 
				auto ptr = std::make_unique<Thread>(std::bind(&PThreadPool::threadFunc, this, std::placeholders::_1));
				unsigned long long id = ptr->getThreadId();
				threadMap_.emplace(id, std::move(ptr));
				//开启线程
				threadMap_[id]->start();
				//更新空闲线程数量
				freeThreadNum++;
				//更新线程数量
				threadNum_++;
			}
			return result;
		}
	private:
		void threadFunc(unsigned long long threadId)
		{
			auto lastTime = std::chrono::high_resolution_clock::now();
			for (;;)
			{
				Task task;
				{
			     //先获取锁
				//	std::cout << std::this_thread::get_id() << "线程尝试" << "获取任务队列锁\n";
					std::unique_lock<std::mutex>locker(taskMutex_);
					//printf("%d线程获取任务队列锁获取任务队列锁\n", std::this_thread::get_id());
					//等待不空条件
					while (taskNum_ == 0)
					{

						if (!isPoolRunning)
						{
							threadMap_.erase(threadId); // std::this_thread::getid()
							std::cout << "threadid:" << std::this_thread::get_id() << " exit!"
								<< std::endl;
							exitCond_.notify_all();
							return; // 线程函数结束，线程结束
						}
						if (mode_ == pstd::PThreadPoolMode::MODE_CACHED)
						{
							//cached模式如果多于线程数量下限，线程60秒没有任务执行那么就要回收
							// 条件变量，超时返回了
							if (std::cv_status::timeout ==
								cd_notEmpty_.wait_for(locker, std::chrono::seconds(1)))
							{
								auto now = std::chrono::high_resolution_clock().now();
								auto dur = std::chrono::duration_cast<std::chrono::seconds>(now - lastTime);
								if ((dur.count() >= 10) && threadNum_ > threadMinSize)
								{
									// 开始回收当前线程
									// 记录线程数量的相关变量的值修改
									// 把线程对象从线程列表容器中删除   没有办法 threadFunc《=》thread对象
									// threadid => thread对象 => 删除
									threadMap_.erase(threadId); // std::this_thread::getid()
									threadNum_--;
									freeThreadNum--;;

									std::cout << "threadid:" << std::this_thread::get_id() << " exit!"
										<< std::endl;
									return;
								}
							}
						}
						else
						{
							cd_notEmpty_.wait(locker);
						}
					}
					freeThreadNum--;
					//取出一个任务
					task = taskQue_.front();
					taskQue_.pop();
					//任务队列数量-1
					taskNum_--;
					//如果不空则通知不空
					if (taskNum_ > 0)
						cd_notEmpty_.notify_all();
					//通知任务队列不满
					cd_notFull_.notify_all();
				}//出作用域释放锁

				//执行这个任务
				if (task != nullptr) {
					task();
					//task();
					freeThreadNum++;
				}
				lastTime = std::chrono::high_resolution_clock::now();
			}
		}
		bool checkState()
		{
			return isPoolRunning;
		}
		//使用unique_ptr能够自动地开辟和释放堆内存
		//std::vector<std::unique_ptr<Thread>>threadVec_;//线程队列
		std::unordered_map<unsigned long long, std::unique_ptr<Thread>>threadMap_;//线程集合
		using Task = std::function<void()>;
		std::queue<Task>taskQue_;//任务队列
		unsigned int taskQueMaxSize_;//任务队列可承受的最大任务数量
		std::atomic_uint taskNum_;//任务数量
		std::atomic_uint threadNum_;//线程数量
		std::atomic_uint threadMaxSize;//cached模式下 线程数量上限
		std::atomic_uint threadMinSize;//cached模式下 线程数量下限
		std::atomic_int freeThreadNum;//空闲线程数量
		std::mutex taskMutex_;//互斥锁
		std::condition_variable cd_notFull_;//不满条件变量
		std::condition_variable cd_notEmpty_;//不空条件变量
		std::condition_variable exitCond_; // 等到线程资源全部回收
		PThreadPoolMode mode_;
		std::atomic_bool isPoolRunning;
	};
}
#endif
