#pragma once
/* Copyright (C) Ralf Kubis */

#include <deque>
#include <optional>
#include <atomic>
#include <memory>
#include <algorithm>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <functional>

#include "r_base/vector.h"
#include "r_base/language_tools.h"
#include "r_base/Log.h"


namespace nsBase::concurrent
{


/** This implementation is inspired by GoLang-Channels.

    A channel is a queue-like container where elements can be stuffed into or
    getting pulled out.
    Both operations are thread-safe.
    A channel is thereby a tool for thread-safe communication between concurrenty
    executing parts of the program.

    A Channel can be of unlimeded or limited capacity.
    If a size-limited channel reached its capacity, the send()-operation blocks
    until some recv()-operation pulled an element.

    A Channel can be closed by calling drain().
    send()-Operations on a closed channel pose a no-op.

    One can register a handler that is called each time an element gets queued.

    There can be multiple consumer-threads. This allows to scale incoming
    compute jobs to a collection of consumers.

    Example:

        // Thread A - the owner

            auto c = channel<Event>{47}; // limited capacity of 47

        // Thread B - an emitter

            c.send(Event{...});
            c.send(Event{...});

        // Thread C - a consumer

            while (true)
            {
                ...

                if (auto e = channel.recv(::nsBase::time::now() + 10ms)) // timeout of 10ms
                {
                    consume_event(e);
                }
                else
                {
                    if (!channel.is_open())
                        break;
                }
            }

        // Thread D - the closer

            c.drain();
            thread_c.join(); // wait until thread C has processed all remaining elements
*/
template<typename Element>
class channel
{
    R_DTOR(channel) = default;
  //R_CTOR(channel); defined below
    R_CCPY(channel) = delete;
    R_CMOV(channel) = delete;
    R_COPY(channel) = delete;
    R_MOVE(channel) = delete;

    public  : using element_t  = Element;
    public  : using queue_t    = ::std::deque<element_t>;

    private : using mutex_t    = ::std::mutex;
    private : using guard_t    = ::std::unique_lock<mutex_t>;
    private : using cond_var_t = ::std::condition_variable;

    public :
        channel(
                ::std::optional<int> max_size
            )
            :   m_max_size {max_size}
            {
            }

    public :
        channel()
            :   channel {::std::optional<int>{}}
            {
            }


/** \name Locking
@{*/
    private : mutex_t
        m_mutex;

    // waking this means that the next entry can be pushed (due to a size limit)
    private : cond_var_t
        m_cv_pushable;

    // waking this means that there is an entry to pop
    private : cond_var_t
        m_cv_popable;

    private : guard_t
        lock()
            {
                return guard_t{m_mutex};
            }
//@}


/** \name Queue size limit
@{*/
    private : ::std::optional<int>
        m_max_size;

    public : ::std::optional<int>
        max_size()
            {
                auto l = lock();
                return m_max_size;
            }

    public : void
        max_size_assign(
                ::std::optional<int> x
            )
            {
                auto l = lock();

                if (x==m_max_size)
                    return;

                auto old = m_max_size;

                m_max_size = x;

                if (auto got_larger = !x.has_value() || old.has_value() && x>old)
                    m_cv_pushable.notify_one();
            }
//@}


/** \name Life Cycle Control
@{*/
    private : ::std::atomic_bool
        m_is_open {true};

    /** Test if the channel is still open. If not, there might still be objects in the channel.
    */
    public : bool
        is_open()
            {
                return m_is_open;
            }

    /** If this function returns TRUE, no object will appear in the channel again.
    */
    public : bool
        is_drained()
            {
                return !m_is_open && empty();
            }

    public : void
        drain()
            {
                auto l = lock();

                if (m_is_open)
                {
                    m_is_open = false;

                    m_cv_pushable.notify_all();
                    m_cv_popable.notify_all();
                }
            }
//@}


/** \name Queue operations
@{*/
    private : queue_t
        m_queue;


    public : bool
        empty()
            {
                auto l = lock();
                return m_queue.empty();
            }

    public : ::std::size_t
        size()
            {
                auto l = lock();
                return m_queue.size();
            }

    /** Send an element trough the channel.

        \param try_until_time_point
                If EMPTY, the call blocks until the element was sent or the channel got drained.
                Otherwise an initial attempt to send the element is performed and if unsuccessful
                re-performed until the element got sent, the try_until_time_point was reached
                or the channel got drained.

        \return TRUE if the element was sent. In this case the value was moved and is no longer usable.
                FALSE if the try_until_time_point was reached or the channel got drained.
                In this case the value was not moved and remains usable.
    */
    public : bool
        send(
                element_t && val_
            ,   ::std::optional<::std::chrono::system_clock::time_point> const & try_until_time_point = {}
            )
            {
                auto val = ::std::move(val_);
                auto l = lock();

                while (true)
                {
                    if (!m_is_open)
                        return false;

                    if (!m_max_size || m_max_size > m_queue.size())
                    {
                        m_queue.push_back(::std::move(val));

                        // entry can be popped
                        m_cv_popable.notify_one();

                        l.unlock();

                        if (handler)
                            handler();

                        return true;
                    }

                    if (    try_until_time_point
                        &&  try_until_time_point < ::std::chrono::system_clock::now()
                    )
                        return false;

                    if (try_until_time_point)
                        m_cv_pushable.wait_until(l, *try_until_time_point);
                    else
                        m_cv_pushable.wait(l);
                }
            }

    public : bool
        try_send(
                element_t && val
            )
            {
                return send(::std::move(val), ::std::chrono::system_clock::time_point{});
            }


    /** Pop the next element off the channel.

        \param try_until_time_point
                If EMPTY, the call blocks until the element was popped or the channel got drained and ran empty.
                Otherwise an initial attempt to pop an element is performed and if unsuccessful
                re-performed until the element got popped or the try_until_time_point was reached
                or the channel got drained and ran empty.

        \param wait_only
                If TRUE, the internal state remains unchanged an no element is returned.
    */
    public : ::std::optional<element_t>
        recv(
                ::std::optional<::std::chrono::system_clock::time_point> try_until_time_point = {}
            ,   bool                                                     wait_only = {}
            )
            {
                ::std::optional<element_t>
                    ret;

                auto l = lock();

                while (true)
                {
                    if (!m_is_open && m_queue.empty())
                        return ret;

                    if (!m_queue.empty())
                    {
                        if (wait_only)
                            break;

                        ret.emplace(::std::move(m_queue.front()));
                        m_queue.pop_front();
                        m_cv_pushable.notify_one();

                        // next entry can be popped
                        if (!m_queue.empty())
                            m_cv_popable.notify_one();

                        break;
                    }

                    if (try_until_time_point)
                    {
                        if (try_until_time_point < ::std::chrono::system_clock::now())
                            return ret;

                        m_cv_popable.wait_until(l, *try_until_time_point);
                    }
                    else
                    {
                        m_cv_popable.wait(l);
                    }
                }

                return ret;
            }

    /** Wait for a channel event.
        Calls recv(try_until_time_point, true).
    */
    public : void
        wait(
                ::std::optional<::std::chrono::system_clock::time_point> try_until_time_point = {}
            )
            {
                recv(try_until_time_point, true);
            }

    public : template<typename COLLECTION>
        void
        send_all(
                COLLECTION && eee_
            )
            {
                auto eee = ::std::move(eee_);

                if (m_max_size) "5d68a804-0e1a-4537-905e-fd28d4317870"_log().throw_UNIMPLEMENTED();
                if (handler)    "32647eaf-9e85-481f-846b-4a13803d0b66"_log().throw_UNIMPLEMENTED();

                if (eee.empty())
                    return;

                auto l = lock();

                for (auto & e : eee)
                    m_queue.push_back(::std::move(e));

                m_cv_popable.notify_one();
            }

    public : queue_t
        recv_all()
            {
                auto l = lock();

                queue_t ret;

                if (!m_queue.empty())
                {
                    m_queue.swap(ret);
                    m_queue.clear();
                    m_cv_pushable.notify_all();
                }

                return ret;
            }
//@}


/** \name Handler
    The handler is called each time an element gets queued.
    The handler is called in the thread that queued the element.
    Typical handlers post a signal to a target thread to trigger element consumption.
@{*/
    public : using
        handler_t = ::std::function<void()>;

    private : handler_t
        handler;

    public : void
        handler_assign(
                handler_t && h
            )
            {
                auto l = lock();

                if (handler)
                    "90a0fc96-46fd-48ec-b23b-0ad01bfee179"_log("once assigned, the handler is immutable").throw_error();

                handler = ::std::move(h);
            }
//@}
};


/** A multiplexer can be used to multicast Elements in a thread-safe way to
    multiple subscribed clients.

    When a client calls subscribe() a new Channel is emplaced for this client.
    The multiplexer maintains a collection of channels, one for each subscriber.

    When an element is send() into the multiplexer, a copy of the element is send()
    through the channels. One copy for each channel. The last channel gets a
    move-constructed instance.

    Like a channel, a multiplexer can be drained, which drains all its channels.

    One can register a handler that is called when the set of subscriptions has
    changed.
*/
template<typename Element>
class multiplexer
{
    private : using mutex_t = ::std::mutex;
    private : using guard_t = ::std::lock_guard<mutex_t>;

    private : mutable mutex_t       m_mutex;
    private : ::std::atomic_bool    m_is_open {true};

    private : guard_t
        lock() const
            {
                return guard_t{m_mutex};
            }

    public : using channel_t     = channel<Element>;
    public : using channel_ref_t = ::std::shared_ptr<channel_t>;

    private : ::std::vector<channel_ref_t>
        subscribers;

    public : bool
        has_subscribers() const
            {
                auto l = lock();
                return !subscribers.empty();
            }


    public : void
        send(
                Element && val_
            )
            {
                auto val = ::std::move(val_);

                auto l = lock();

                bool have_dangling_channel = false;

                for (auto i=subscribers.size(); i--;)
                {
                    auto & c = subscribers[i];

                    if (c.use_count()==1)
                    {
                        have_dangling_channel = true;
                    }
                    else
                    {
                        if (i)
                            c->send(Element{val}); // copy
                        else
                            c->send(::std::move(val)); // move (into the last channel)
                    }
                }

                if (have_dangling_channel)
                {
                    remove_if_and_erase(
                            subscribers
                        ,   [](auto & c){return c.use_count()==1;}
                        );
                }
            }


    public : void
        send(
                Element const & val
            )
            {
                send(Element{val});
            }

    public : template<typename COLLECTION>
        void
        send_all(
                COLLECTION && vvv_
            )
            {
                auto vvv = ::std::move(vvv_);

                auto l = lock();

                bool have_dangling_channel = false;

                for (auto i=subscribers.size(); i--;)
                {
                    auto & c = subscribers[i];

                    if (c.use_count()==1)
                    {
                        have_dangling_channel = true;
                    }
                    else
                    {
                        if (i)
                            c->send_all(COLLECTION(vvv)); // copy
                        else
                            c->send_all(::std::move(vvv)); // move (into the last channel)
                    }
                }

                if (have_dangling_channel)
                {
                    remove_if_and_erase(
                            subscribers
                        ,   [](auto & c){return c.use_count()==1;}
                        );
                }
            }


    public : bool
        is_open()
            {
                return m_is_open;
            }

    public : void
        drain()
            {
                auto l = lock();

                m_is_open = false;

                for (auto & c : subscribers)
                    c->drain();
            }


    /** Always retuns a channel.
        If the multiplexer was closed, the returned channel is initially closed.
        If the caller drops its reference to the channel,
            the channel gets deleted by the multiplexer on the next invocation of send*().
    */
    public : channel_ref_t
        subscribe()
            {
                channel_ref_t c;
                handler_t   * handler_to_call {};

                {
                    auto l = lock();

                    c = subscribers.emplace_back(::std::make_shared<channel_t>());

                    if (!m_is_open)
                        c->drain();

                    if (handler)
                        handler_to_call = &handler;
                }

                // handler is called in unlocked state
                if (handler_to_call && *handler_to_call)
                    (*handler_to_call)();

                return c;
            }

    /** Insert an existing channel into the collection of channels.
        If the multiplexer was already closed, the channel gets drained.
        If the caller drops its reference to the channel,
            the channel gets deleted by the multiplexer on the next invocation of send*().
    */
    public : void
        subscribe(
                channel_ref_t const & channel
            )
            {
                if (!channel) "de31b802-4a3c-4cd7-907a-5ba155c1368d"_log().throw_INTERNAL();

                handler_t * handler_to_call {};

                {
                    auto l = lock();

                    subscribers.emplace_back(channel);

                    if (!m_is_open)
                        channel->drain();

                    if (handler)
                        handler_to_call = &handler;
                }

                // handler is called in unlocked state
                if (handler_to_call && *handler_to_call)
                    (*handler_to_call)();
            }


/** \name Handler
    The handler is called if the set of subscriptions has changed.
    The handler is called in the thread that changed the set of subscriptions.
@{*/
    public : using
        handler_t = ::std::function<void()>;

    private : handler_t
        handler;

    public : void
        handler_assign(
                handler_t && h
            )
            {
                auto l = lock();

                if (handler)
                    "514ea964-26b8-41bc-a4af-bf810a1111b2"_log("once assigned, the handler is immutable").throw_error();

                handler = ::std::move(h);
            }
//@}
};

}
