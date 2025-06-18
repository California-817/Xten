#ifndef __XTEN_SAFEQUEUE_H__
#define __XTEN_SAFEQUEUE_H__
#include <memory>
#include"mutex.h"
#include"nocopyable.hpp"
namespace Xten
{
    // 封装双向链表组织的线程安全队列
    template <class T>
    class ThreadSafeQueue : public NoCopyable
    {
    private:
        // 链表节点
        struct Node
        {
            std::shared_ptr<T> data; // 数据
            std::unique_ptr<Node> next; //后一个节点(用智能指针 该节点删除 下一个节点引用计数-1)
            Node* prev=nullptr; //前一个节点(用原始指针 防止前一个节点删除 但是引用计数仍+1)
        };
        std::unique_ptr<Node> pop_head()
        {
            std::unique_ptr<Node> old_head(std::move(_head));
            _head=std::move(old_head->next);
            return std::move(old_head);
        }
        std::unique_ptr<Node> try_pop_head(T& value)
        {
            SpinLock::Lock lock(_head_mutex);
            if(_head.get()==_tail)
            {
                //链表为空
                return std::unique_ptr<Node>();
            }
            value=std::move(*(_head->data));
            return pop_head();
        }
    public:
        //初始化一个空节点
        ThreadSafeQueue()
        :_head(new Node),_tail(_head.get())
        {}
        //放入数据
        void Push(T new_value)
        {
            std::shared_ptr<T> new_data=std::make_shared<T>(std::move(new_value));
            std::unique_ptr<Node> p(new Node);  //作为新尾节点
            {
                SpinLock::Lock lock(_tail_mutex);
                _tail->data=new_data;
                Node* const new_tail=p.get();
                new_tail->prev=_tail;
                _tail->next=std::move(p);
                _tail=new_tail;
            }
        }
        //尝试获取 不阻塞
        bool TryPop(T& value)
        {
            std::unique_ptr<Node> old_head=try_pop_head(value);
            if(old_head)
            {
                return true;
            }
            return false;
        }
        //尝试窃取任务 不阻塞
        bool TrySteal(T& value)
        {
            //可能涉及到前后访问到同一个节点
            SpinLock::Lock hlock(_head_mutex);
            SpinLock::Lock tlock(_tail_mutex);
            if(_head.get()==_tail)
            {
                return false;
            }
            Node* prev=_tail->prev;
            value=std::move(*(prev->data));
            _tail=prev;
            //不能删除节点 因为前一个节点的next是这个节点的智能指针（自动删除节点）
            _tail->prev=nullptr; //删除节点 （智能指针对象销毁）
            return true;

        }
        ~ThreadSafeQueue()=default;
    private:
        SpinLock::Lock _head_mutex; //头互斥量
        SpinLock::Lock _tail_mutex; //尾互斥量
        std::unique_ptr<Node> _head; //头指针  每一个头节点没删除的时候都有该指针指向--不会销毁
        Node* _tail=nullptr; //尾指针  中间及尾部节点都有前一个节点的next智能指针指向节点 ---不会销毁
    };
}
#endif