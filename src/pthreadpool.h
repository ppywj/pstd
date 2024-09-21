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
	//����������

	//�̳߳�ģʽ
	enum class   PThreadPoolMode {
		MODE_FIXED,//�̶��������߳�
		MODE_CACHED//�߳������ɶ�̬����
	};

	//�߳���
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
	//�̳߳���
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
			//std::cout << "�����������" << "��ȡ���������\n";
			cd_notEmpty_.notify_all();
			exitCond_.wait(locker, [&]() -> bool {return threadMap_.size() == 0; });
		}
		PThreadPool(const PThreadPool&) = delete;
		PThreadPool& operator=(const PThreadPool&) = delete;
		//�����̳߳�ģʽ
		void setMode(PThreadPoolMode mode)
		{
			if (checkState())
				return;
			this->mode_ = mode;
		}
		//����
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
				threadMap_[i]->start(); // ��Ҫȥִ��һ���̺߳���
				freeThreadNum++;//�����߳���������
			}
		}
		//��������������������
		void setTaskQueMaxSize(unsigned int size)
		{
			if (checkState())
				return;
			taskQueMaxSize_ = size;
		}
		//����cachedģʽ�µ��߳���������
		void setMinThreadSize(unsigned int minSize)
		{
			if (isPoolRunning)
				return;
			threadMinSize = minSize;
		}
		//����cachedģʽ�µ��߳���������
		void setMaxThreadSize(unsigned int maxSize)
		{
			if (checkState())
				return;
			threadMaxSize = maxSize;
		}
		//�ύ�����������
		template<typename Func, typename... Args>
		auto submitTask(Func&& func, Args&&... args) -> std::future<decltype(func(args...))>
		{
			using RType = decltype(func(args...));
			auto task = std::make_shared<std::packaged_task<RType()>>(
				std::bind(std::forward<Func>(func),std::forward<Args>(args)...));
			std::future<RType>result = task->get_future();
			//��ȡ��
			std::unique_lock<std::mutex>locker(taskMutex_);
			//�ж����� ������ȴ���������
			if (!cd_notFull_.wait_for(locker, std::chrono::seconds(1), [&]()->bool {return taskNum_ < taskQueMaxSize_; }))
			{
				auto task= std::make_shared<std::packaged_task<RType()>>([]()->RType { return RType(); });
				(*task)();
				return task->get_future();
			}
			//�������
			taskQue_.emplace([task]() {(*task)();});
			taskNum_++;
			//֪ͨ����
			cd_notEmpty_.notify_all();
			if (mode_ == pstd::PThreadPoolMode::MODE_CACHED
				&& threadNum_ < threadMaxSize
				&& taskNum_>freeThreadNum)
			{
				std::cout << "�´���һ���߳�" << std::endl;
				//�����߳� 
				auto ptr = std::make_unique<Thread>(std::bind(&PThreadPool::threadFunc, this, std::placeholders::_1));
				unsigned long long id = ptr->getThreadId();
				threadMap_.emplace(id, std::move(ptr));
				//�����߳�
				threadMap_[id]->start();
				//���¿����߳�����
				freeThreadNum++;
				//�����߳�����
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
			     //�Ȼ�ȡ��
				//	std::cout << std::this_thread::get_id() << "�̳߳���" << "��ȡ���������\n";
					std::unique_lock<std::mutex>locker(taskMutex_);
					//printf("%d�̻߳�ȡ�����������ȡ���������\n", std::this_thread::get_id());
					//�ȴ���������
					while (taskNum_ == 0)
					{

						if (!isPoolRunning)
						{
							threadMap_.erase(threadId); // std::this_thread::getid()
							std::cout << "threadid:" << std::this_thread::get_id() << " exit!"
								<< std::endl;
							exitCond_.notify_all();
							return; // �̺߳����������߳̽���
						}
						if (mode_ == pstd::PThreadPoolMode::MODE_CACHED)
						{
							//cachedģʽ��������߳��������ޣ��߳�60��û������ִ����ô��Ҫ����
							// ������������ʱ������
							if (std::cv_status::timeout ==
								cd_notEmpty_.wait_for(locker, std::chrono::seconds(1)))
							{
								auto now = std::chrono::high_resolution_clock().now();
								auto dur = std::chrono::duration_cast<std::chrono::seconds>(now - lastTime);
								if ((dur.count() >= 10) && threadNum_ > threadMinSize)
								{
									// ��ʼ���յ�ǰ�߳�
									// ��¼�߳���������ر�����ֵ�޸�
									// ���̶߳�����߳��б�������ɾ��   û�а취 threadFunc��=��thread����
									// threadid => thread���� => ɾ��
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
					//ȡ��һ������
					task = taskQue_.front();
					taskQue_.pop();
					//�����������-1
					taskNum_--;
					//���������֪ͨ����
					if (taskNum_ > 0)
						cd_notEmpty_.notify_all();
					//֪ͨ������в���
					cd_notFull_.notify_all();
				}//���������ͷ���

				//ִ���������
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
		//ʹ��unique_ptr�ܹ��Զ��ؿ��ٺ��ͷŶ��ڴ�
		//std::vector<std::unique_ptr<Thread>>threadVec_;//�̶߳���
		std::unordered_map<unsigned long long, std::unique_ptr<Thread>>threadMap_;//�̼߳���
		using Task = std::function<void()>;
		std::queue<Task>taskQue_;//�������
		unsigned int taskQueMaxSize_;//������пɳ��ܵ������������
		std::atomic_uint taskNum_;//��������
		std::atomic_uint threadNum_;//�߳�����
		std::atomic_uint threadMaxSize;//cachedģʽ�� �߳���������
		std::atomic_uint threadMinSize;//cachedģʽ�� �߳���������
		std::atomic_int freeThreadNum;//�����߳�����
		std::mutex taskMutex_;//������
		std::condition_variable cd_notFull_;//������������
		std::condition_variable cd_notEmpty_;//������������
		std::condition_variable exitCond_; // �ȵ��߳���Դȫ������
		PThreadPoolMode mode_;
		std::atomic_bool isPoolRunning;
	};
}
#endif
