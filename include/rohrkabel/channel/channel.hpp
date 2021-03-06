#pragma once
#include <map>
#include <queue>
#include <memory>
#include <variant>
#include <functional>
#include <condition_variable>

#include "../utils/annotations.hpp"
namespace pipewire
{
    class proxy;
    class main_loop;
    struct channel_state;

    class sender_impl
    {
      private:
        std::shared_ptr<channel_state> m_state;

      protected:
        void emit_signal();

      public:
        ~sender_impl();

      protected:
        sender_impl(sender_impl &&) noexcept;
        sender_impl(std::shared_ptr<channel_state>);

      public:
        sender_impl(const sender_impl &) = delete;
        sender_impl &operator=(const sender_impl &) = delete;
    };

    class receiver_impl
    {
      private:
        std::shared_ptr<channel_state> m_state;

      protected:
        std::function<void()> on_receive;

      protected:
        void attach(main_loop *);

      public:
        ~receiver_impl();

      protected:
        receiver_impl(receiver_impl &&) noexcept;
        receiver_impl(std::shared_ptr<channel_state>);

      public:
        receiver_impl(const sender_impl &) = delete;
        receiver_impl &operator=(const sender_impl &) = delete;
    };

    template <typename... Messages> class sender : public sender_impl
    {
        using variant_t = std::variant<Messages...>;
        static_assert((!std::is_base_of_v<proxy, std::remove_pointer_t<std::remove_cv_t<std::decay_t<Messages>>>> && ...), "pipewire objects may not be exchanged");

      private:
        std::shared_ptr<std::mutex> m_mutex;
        std::shared_ptr<std::queue<variant_t>> m_queue;

      public:
        sender(sender &&) noexcept;
        sender(std::shared_ptr<channel_state>, decltype(m_mutex), decltype(m_queue));

      public:
        template <typename T> //
        [[thread_safe]] void send(T && = {});
    };

    template <typename... Messages> class receiver : public receiver_impl
    {
        using variant_t = std::variant<Messages...>;
        static_assert((!std::is_base_of_v<proxy, std::remove_pointer_t<std::remove_cv_t<std::decay_t<Messages>>>> && ...), "pipewire objects may not be exchanged");

      private:
        std::shared_ptr<std::mutex> m_mutex;
        std::shared_ptr<std::queue<variant_t>> m_queue;

      public:
        receiver(receiver &&) noexcept;
        receiver(std::shared_ptr<channel_state>, decltype(m_mutex), decltype(m_queue));

      public:
        template <typename Callback> //
        [[thread_safe]] void attach(main_loop *, Callback &&);
    };

    std::shared_ptr<channel_state> make_state();
    template <typename... Messages> std::pair<sender<Messages...>, receiver<Messages...>> channel();
} // namespace pipewire
#include "../utils/annotations.hpp"

#include "channel.inl"