#include "../include/scheduler.h"
#include "../include/macro.h"
#include"../include/hook.h"
#include "log.h"
namespace Xten
{
	static Logger::ptr g_logger = XTEN_LOG_NAME("system");

	static thread_local Scheduler *t_scheduler = nullptr;	// 线程所属协程调度器
	static thread_local Fiber *t_scheduler_fiber = nullptr; // 线程的调度协程
	Scheduler::Scheduler(int threadNum, bool use_caller, const std::string &name)
		: _name(name), _threads_num(threadNum)
	{
		XTEN_ASSERTINFO(_threads_num > 0, "scheduler threads num<0");
		if (use_caller)
		{
			// 创建该调度器的线程也参与调度
			_threads_num--;
			_root_threadId = Xten::ThreadUtil::GetThreadId();
			// 创建默认主协程
			Xten::Fiber::GetThis();
			// 判断当前线程没有归属的协程调度器
			XTEN_ASSERT((GetThis() == nullptr));
			t_scheduler = this;
			// 创建调度子协程---自定义删除器
			_root_fiber.reset(NewFiber(0, std::bind(&Scheduler::Run, this), true), FreeFiber);
			t_scheduler_fiber = _root_fiber.get();
			Xten::Thread::SetName(_name);
			_thread_ids.push_back(_root_threadId);
			// Xten::set_hook_enable(true);
		}
	}
	Scheduler::~Scheduler()
	{
		XTEN_ASSERT((_stopping));
		if (GetThis() == this) // 这个执行析构的线程的所属协程调度器是该调度器
		{
			t_scheduler = nullptr; // 置空便于下次继续创建归属调度器
		}
	}
	// 启动
	void Scheduler::Start()
	{
		RWMutex::WriteLock lock(_mutex);
		if (!_stopping)
		{ // 已经启动
			return;
		}
		XTEN_ASSERT(_threads.empty());
		_stopping = false;
		_threads.resize(_threads_num);
		for (int i = 0; i < _threads_num; i++)
		{
			// 创建线程绑定run方法
			_threads[i] = std::make_shared<Xten::Thread>(std::bind(&Scheduler::Run, this),
														 _name + "_" + std::to_string(i));
			_thread_ids.push_back(_threads[i]->getId()); // 存放线程id
		}
	}
	// 运行函数
	void Scheduler::Run()
	{
		// 设置线程所属调度器
		SetThis();
		// 设置hook属性
		Xten::set_hook_enable(true);
		// 设置当前线程的调度协程
		if (Xten::ThreadUtil::GetThreadId() != _root_threadId)
		{
			// 调度器内工作线程的调度协程为默认主协程
			t_scheduler_fiber = Xten::Fiber::GetThis().get();
		}
		// 创建idle协程
		Fiber::ptr idle_fiber = std::shared_ptr<Fiber>(NewFiber(0, std::bind(&Scheduler::Idle, this), false), FreeFiber);
		// 任务协程
		Fiber::ptr cb_fiber;
		FuncOrFiber fcb; // Task
		while (true)
		{
			fcb.Reset();
			bool tickle_me = false;
			bool is_active = false;
			{
				RWMutex::WriteLock lock(_mutex); //加一把全局锁保证多线程访问任务队列的线程安全（锁的粒度是比较大的）
				auto iter = _fun_fibers.begin();
				while (iter != _fun_fibers.end())
				{
					if (iter->threadId != -1 && iter->threadId != Xten::ThreadUtil::GetThreadId())
					{
						// 指定了其他线程执行该任务
						iter++;
						tickle_me = true;
						continue;
					}
					// 可以被该线程执行
					XTEN_ASSERT((iter->fiber || iter->func));
					if (iter->fiber && iter->fiber->GetStatus() == Fiber::Status::EXEC)
					{
						// 协程任务并且处于执行状态
						iter++;
						continue;
					}
					fcb = *iter; // 赋值函数
					_fun_fibers.erase(iter++);
					_active_threadNum++;
					is_active = true;
					break;
				}
				// 不为空也通知
				tickle_me |= !_fun_fibers.empty();
			}
			if (tickle_me) // 通知
			{
				Tickle();
			}
			// 任务类型是fiber
			if (fcb.fiber && fcb.fiber->GetStatus() != Fiber::Status::EXCEPT &&
				fcb.fiber->GetStatus() != Fiber::Status::TERM)
			{
				fcb.fiber->SwapIn(); // 切入执行工作协程
				_active_threadNum--;
				if (fcb.fiber->GetStatus() == Fiber::Status::READY)
				{ // 执行完切出来后 协程状态为 准备执行状态 -----继续调度
					Schedule(fcb.fiber, fcb.threadId);
				}
				else if (fcb.fiber->GetStatus() != Fiber::Status::TERM &&
						 fcb.fiber->GetStatus() != Fiber::Status::EXCEPT)
				{
					// 状态设置成挂起状态 ----执行条件不满足
					fcb.fiber->_status = Fiber::Status::HOLD;
				}
				// 协程状态终止或者错误终止
				fcb.Reset();
			}
			// 任务类型是cb
			else if (fcb.func)
			{
				// 根据cb创建一个任务协程
				if (cb_fiber) // 使用上次回收协程
				{
					cb_fiber->Reset(fcb.func);
				}
				else
				{
					cb_fiber.reset(NewFiber(0, fcb.func, false), FreeFiber);
				}
				// 切入执行协程任务
				int tid = fcb.threadId;
				fcb.Reset();
				cb_fiber->SwapIn();
				_active_threadNum--;
				if (cb_fiber->GetStatus() == Fiber::Status::READY)
				{
					Schedule(cb_fiber, tid);
					// 智能指针置空 ---协程不能回收使用
					cb_fiber.reset();
				}
				else if (cb_fiber->GetStatus() == Fiber::Status::TERM ||
						 cb_fiber->GetStatus() == Fiber::Status::EXCEPT)
				{
					// 任务终止 ---协程可以回收使用
					cb_fiber->Reset(nullptr); // 指针并未置空---协程对象仍存在
				}
				else
				{
					// 任务处于条件不满足 --挂起状态  --不能回收使用协程
					cb_fiber->_status = Fiber::Status::HOLD;
					cb_fiber.reset();
				}
			}
			// 没有获取到任务---------执行idle协程
			else
			{
				if (is_active)
				{ // 基本不会走到这里
					_active_threadNum--;
					continue;
				}
				// 执行idle协程
				if (idle_fiber->GetStatus() == Fiber::Status::TERM ||
					idle_fiber->GetStatus() == Fiber::Status::EXCEPT)
				{ // idle协程的状态是终止状态或者异常退出
					XTEN_LOG_DEBUG(g_logger) << "idle fiber term";
					break; // 整个工作线程退出
				}
				_idle_threadNum++;
				idle_fiber->SwapIn();
        		XTEN_LOG_DEBUG(g_logger) << "idle out";
				_idle_threadNum--;
				if (idle_fiber->GetStatus() != Fiber::Status::TERM &&
					idle_fiber->GetStatus() != Fiber::Status::EXCEPT)
				{
					idle_fiber->_status = Fiber::Status::HOLD;
				}
				// 如果idle协程状态是term或者EXCEPT 下次再执行到这里也会会退出
			}
		}
		// 跳出循环----线程结束
	}
	// 获取name
	std::string Scheduler::GetName() const
	{
		return _name;
	}
	// 输出调度器状态信息
	std::ostream &Scheduler::dump(std::ostream &os) const
	{
		os << "[Scheduler name=" << _name
		   << " size=" << _threads_num
		   << " active_count=" << _active_threadNum
		   << " idle_count=" << _idle_threadNum
		   << " stopping=" << _stopping
		   << " ]" << std::endl
		   << "    ";
		for (size_t i = 0; i < _thread_ids.size(); ++i)
		{
			if (i)
			{
				os << ", ";
			}
			os << _thread_ids[i];
		}
		return os;
	}
	// 切换执行线程(或者调度器)
	void Scheduler::SwitchTo(int threadId)
	{
		XTEN_ASSERT((Scheduler::GetThis() != nullptr));
		if (Scheduler::GetThis() == this)
		{ // 只有 1.目标调度器是自身 && ( 2.目标线程id是自己 || 3.或者目标线程id是-1 ) ------->仍继续执行该协程
			if (threadId == -1 || threadId == Xten::ThreadUtil::GetThreadId())
			{
				return;
			}
		}
		Schedule(Xten::Fiber::GetThis(), threadId); // 重新放入调度队列中调度 ---不一定是本身调度器的调度队列
		Fiber::YieldToHold();						// 当前线程从协程中切出--不调度该协程
	}

	// 返回线程的当前协程调度器
	Scheduler *Scheduler::GetThis()
	{
		return t_scheduler;
	}
	// 返回当前线程的调度协程
	Fiber *Scheduler::GetScheduleFiber()
	{
		return t_scheduler_fiber;
	}

	// 通知线程有任务 ---子类实现
	void Scheduler::Tickle()
	{
		XTEN_LOG_INFO(g_logger) << "tickle";
	}

	// 返回是否可以终止 ---子类实现
	bool Scheduler::IsStopping()
	{
		RWMutex::ReadLock lock(_mutex);
		return _stopping && _auto_stopping &&
			   _fun_fibers.empty() && !_active_threadNum;
	}
	// 停止
	void Scheduler::Stop()
	{
		_auto_stopping = true;
		if (_root_fiber &&
			_threads_num == 0 &&
			(_root_fiber->GetStatus() == Fiber::Status::INIT ||
			 _root_fiber->GetStatus() == Fiber::Status::TERM))
		{
			_stopping = true;
			if (IsStopping())
			{
				return;
			}
		}
		if (_root_threadId != -1)
		{									  // 调度器有绑定线程
			XTEN_ASSERT((GetThis() == this)); // 创建时绑定的线程才有资格停止调度器
		}
		else
		{
			XTEN_ASSERT((GetThis() != this));
		}
		_stopping = true;
		// 唤醒工作线程--防止没任务一直处于休眠状态
		for (int i = 0; i < _threads_num; i++)
		{
			Tickle();
		}
		if (_root_fiber)
		{ // 创建线程的调度协程
			if (!IsStopping())
			{
				// 由默认主协程切换到该线程的调度协程
				_root_fiber->Call();
				// 这个线程的调度协程处理完任务之后返回
			}
		}
		// 等待工作线程退出
		std::vector<Thread::ptr> ths;
		{
			RWMutex::WriteLock lock(_mutex);
			ths.swap(_threads);
		}
		for (auto &th : ths)
		{
			th->join();
		}
	}
	// 线程无任务执行idle空闲协程  ---子类实现
	void Scheduler::Idle()
	{
		XTEN_LOG_INFO(g_logger) << "idle";
	}
	// 设置线程当前调度器
	void Scheduler::SetThis()
	{
		t_scheduler = this;
	}
	// 返回是否有空闲线程
	bool Scheduler::HasIdleThread()
	{
		return _idle_threadNum > 0;
	}

	// 协程任务切换器 --切换协程任务运行的调度器
	SwitchScheduler::SwitchScheduler(Scheduler *target)
	{
		// 保存当前调度器
		_caller = Xten::Scheduler::GetThis();
		if (target)
		{
			target->SwitchTo();
		}
	}
	SwitchScheduler::~SwitchScheduler()
	{
		// 切回原来调度器
		if (_caller)
		{
			_caller->SwitchTo();
		}
	}
}
std::ostream &operator<<(std::ostream &os, const Xten::Scheduler &scheduler)
{
	scheduler.dump(os);
	return os;
}
