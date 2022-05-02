#include "core/core.hpp"
#include <pipewire/pipewire.h>

namespace pipewire
{
    struct core::impl
    {
        pw_core *core;
    };

    core::~core()
    {
        pw_core_disconnect(m_impl->core);
    }

    core::core(context &context) : m_context(context), m_impl(std::make_unique<impl>())
    {
        m_impl->core = pw_context_connect(context.get(), nullptr, 0);
    }

    int core::sync(int seq)
    {
        // NOLINTNEXTLINE
        return pw_core_sync(m_impl->core, PW_ID_CORE, seq);
    }

    void core::sync()
    {
        bool done = false;
        int pending = sync(0);
        auto listener = listen<core_listener>();

        listener.on<core_event::done>([&](std::uint32_t id, int seq) {
            if (id == PW_ID_CORE && seq == pending)
            {
                done = true;
                m_context.get_main_loop().quit();
            }
        });

        while (!done)
        {
            m_context.get_main_loop().run();
        }
    }

    template <> core_listener core::listen<core_listener>()
    {
        return {*this};
    }

    pw_core *core::get() const
    {
        return m_impl->core;
    }

    context &core::get_context()
    {
        return m_context;
    }
} // namespace pipewire