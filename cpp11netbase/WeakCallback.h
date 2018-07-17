#ifndef MUDUO_BASE_WEAKCALLBACK_H
#define MUDUO_BASE_WEAKCALLBACK_H

#include <functional>
#include <memory>

// A barely usable WeakCallback

#ifdef __GXX_EXPERIMENTAL_CXX0X__

// FIXME: support std::shared_ptr as well, maybe using template template parameters

template<typename CLASS, typename... ARGS>
class WeakCallback
{
public:

	WeakCallback(const std::weak_ptr<CLASS>& object,
	       const std::function<void (CLASS*, ARGS...)>& function)
	: object_(object), function_(function)
	{
	}

	// Default dtor, copy ctor and assignment are okay

	void operator()() const
	{
		std::shared_ptr<CLASS> ptr(object_.lock());
		if (ptr)
		{
		  function_(ptr.get());
		}
	}

private:

	std::weak_ptr<CLASS> object_;
	std::function<void (CLASS*)> function_;
};

template<typename CLASS, typename... ARGS>
WeakCallback<CLASS, ARGS...> makeWeakCallback(const std::shared_ptr<CLASS>& object,
                                              void (CLASS::*function)(ARGS...))
{
	return WeakCallback<CLASS, ARGS...>(object, function);
}

template<typename CLASS, typename... ARGS>
WeakCallback<CLASS, ARGS...> makeWeakCallback(const std::shared_ptr<CLASS>& object,
                                              void (CLASS::*function)(ARGS...) const)
{
	return WeakCallback<CLASS, ARGS...>(object, function);
}

#else  // __GXX_EXPERIMENTAL_CXX0X__

// the C++98/03 version doesn't support arguments.

template<typename CLASS>
class WeakCallback
{
public:

	WeakCallback(const std::weak_ptr<CLASS>& object,
		       const std::function<void (CLASS*)>& function)
	: object_(object), function_(function)
	{
	}

	void operator()() const
	{
		boost::shared_ptr<CLASS> ptr(object_.lock());
		if (ptr)
		{
		  function_(ptr.get());
		}
	}

private:

	boost::weak_ptr<CLASS> object_;
	boost::function<void (CLASS*)> function_;
};

template<typename CLASS>
WeakCallback<CLASS> makeWeakCallback(const std::shared_ptr<CLASS>& object,
                                     void (CLASS::*function)())
{
	return WeakCallback<CLASS>(object, function);
}

template<typename CLASS>
WeakCallback<CLASS> makeWeakCallback(const std::shared_ptr<CLASS>& object,
                                     void (CLASS::*function)() const)
{
  return WeakCallback<CLASS>(object, function);
}
#endif  // __GXX_EXPERIMENTAL_CXX0X__
#endif
