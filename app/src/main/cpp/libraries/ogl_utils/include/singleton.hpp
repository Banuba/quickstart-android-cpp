#pragma once

namespace bnb
{
    /**
     * @addtogroup utils
     * @{
     */

    /// \brief Base class for all scoped singletons.
    ///
    /// This is a Meyers Singleton implementation.
    ///
    template<typename T>
    class singleton
    {
    public:
        singleton(const singleton<T>& other) = delete;
        singleton<T>& operator=(const singleton<T>& other) = delete;

        static T& instance()
        {
            static T instance;
            return instance;
        }

    protected:
        singleton() = default;
        ~singleton() = default;
    };

    /** @} */ // endgroup utils
} // namespace bnb
