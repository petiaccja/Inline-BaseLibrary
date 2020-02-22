#pragma once

#include <cassert>
#include <compare>
#include <cstring>
#include <new>
#include <tuple>
#include <typeindex>
#include <utility>


namespace inl {


template <class ReturnT, class... ArgsT>
class Delegate;



namespace impl {

	template <class ReturnT, class... ArgsT>
	class Callable;

	template <class... ArgsT>
	class CallableBase {
	public:
		virtual ~CallableBase() {}
		virtual void CallVoid(ArgsT... args) const = 0;
		virtual bool IsEmpty() const = 0;
		bool operator==(const CallableBase& rhs) const;
		bool operator!=(const CallableBase& rhs) const;
		std::strong_ordering operator<=>(const CallableBase& rhs) const;

	protected:
		virtual void* GetFuncPtr() const = 0;
		virtual size_t GetFuncPtrSize() const = 0;
		virtual bool IsClass() const = 0;
		virtual std::type_index GetClassType() const = 0;
		virtual void* GetClassPtr() const = 0;
	};


	template <class ReturnT, class... ArgsT>
	class Callable : public CallableBase<ArgsT...> {
	public:
		~Callable() {}
		void CallVoid(ArgsT... args) const override final {
			Call(std::forward<ArgsT>(args)...);
		}
		virtual ReturnT Call(ArgsT... args) const = 0;
	};

	template <class ReturnT, class... ArgsT>
	class GlobalCallable : public Callable<ReturnT, ArgsT...> {
	public:
		GlobalCallable(ReturnT (*func)(ArgsT...)) : m_func(func) {}
		~GlobalCallable() {}
		bool IsEmpty() const override { return m_func == nullptr; }
		void* GetFuncPtr() const override { return &m_func; }
		size_t GetFuncPtrSize() const override { return sizeof(m_func); }
		bool IsClass() const override { return false; }
		std::type_index GetClassType() const override { return typeid(void); }
		void* GetClassPtr() const override { return nullptr; }
		ReturnT Call(ArgsT... args) const override {
			assert(m_func != nullptr);
			return m_func(std::forward<ArgsT>(args)...);
		}

	private:
		ReturnT (*m_func)(ArgsT...) = nullptr;
	};

	template <class ClassT, class ReturnT, class... ArgsT>
	class MemberCallable : public Callable<ReturnT, ArgsT...> {
	private:
		template <typename T>
		struct is_complete_helper {
			template <typename U>
			static auto test(U*) -> std::integral_constant<bool, sizeof(U) == sizeof(U)>;
			static auto test(...) -> std::false_type;
			using type = decltype(test((T*)0));
		};

		template <typename T>
		struct is_complete : is_complete_helper<T>::type {};
		template <typename T>
		static constexpr bool is_complete_v = is_complete<T>::value;
	public:
		using FuncT = std::conditional_t<std::is_const_v<ClassT>, ReturnT (ClassT::*)(ArgsT...) const, ReturnT (ClassT::*)(ArgsT...)>;
		MemberCallable(FuncT func, ClassT* cl) : m_class(cl) {
			m_func = func;
		}
		MemberCallable(const MemberCallable& rhs) : m_class(rhs.m_class) {
			m_func = rhs.m_func;
		}
		MemberCallable& operator=(const MemberCallable& rhs) {
			m_class = rhs.m_class;
			m_func = rhs.m_func;
		}
		~MemberCallable() {}
		bool IsEmpty() const override { return m_func == nullptr; }
		void* GetFuncPtr() const override { return (void*)&m_func; }
		size_t GetFuncPtrSize() const override { return sizeof(m_func); }
		bool IsClass() const override { return true; }
		std::type_index GetClassType() const override {
			if constexpr (is_complete_v<ClassT>)
				return typeid(ClassT);
			else {
				return typeid(void);
			}
		}
		void* GetClassPtr() const override { return m_class; }
		ReturnT Call(ArgsT... args) const override {
			assert(m_func != nullptr && m_class != nullptr);
			FuncT func = static_cast<FuncT>(m_func);
			return (m_class->*m_func)(std::forward<ArgsT>(args)...);
		}

	private:
		FuncT m_func = nullptr;
		ClassT* m_class = nullptr;
	};



} // namespace impl



template <class ReturnT, class... ArgsT>
class Delegate<ReturnT(ArgsT...)> {
	class Dummy;

public:
	Delegate() = default;
	Delegate(const Delegate& rhs) {
		memcpy(m_callablePlaceholder, rhs.m_callablePlaceholder, sizeof(m_callablePlaceholder));
		m_callable = reinterpret_cast<impl::Callable<void, ArgsT...>*>(m_callablePlaceholder + ((size_t)rhs.m_callable - (size_t)rhs.m_callablePlaceholder));
	}
	Delegate& operator=(const Delegate& rhs) {
		memcpy(m_callablePlaceholder, rhs.m_callablePlaceholder, sizeof(m_callablePlaceholder));
		m_callable = reinterpret_cast<impl::Callable<void, ArgsT...>*>(m_callablePlaceholder + ((size_t)rhs.m_callable - (size_t)rhs.m_callablePlaceholder));
		return *this;
	}
	Delegate(Delegate&&) = delete;
	Delegate& operator=(Delegate&&) = delete;

	Delegate(ReturnT (*func)(ArgsT...)) {
		static_assert(sizeof(impl::GlobalCallable<ReturnT, ArgsT...>) <= sizeof(m_callablePlaceholder), "Tell this error and the compiler you used to the author.");
		new (m_callablePlaceholder) impl::GlobalCallable<ReturnT, ArgsT...>(func);
		m_callable = static_cast<impl::Callable<ReturnT, ArgsT...>*>(reinterpret_cast<impl::GlobalCallable<ReturnT, ArgsT...>*>(m_callablePlaceholder));
	}

	template <class ClassT, typename std::enable_if<!std::is_const<ClassT>::value, int>::type = 0>
	Delegate(ReturnT (ClassT::*func)(ArgsT...), ClassT* owner) {
		static_assert(sizeof(impl::MemberCallable<ClassT, ReturnT, ArgsT...>) <= sizeof(m_callablePlaceholder), "Tell this error and the compiler you used to the author.");
		new (m_callablePlaceholder) impl::MemberCallable<ClassT, ReturnT, ArgsT...>(func, owner);
		m_callable = static_cast<impl::Callable<ReturnT, ArgsT...>*>(reinterpret_cast<impl::MemberCallable<ClassT, ReturnT, ArgsT...>*>(m_callablePlaceholder));
	}

	template <class ClassT>
	Delegate(ReturnT (ClassT::*func)(ArgsT...) const, const ClassT* owner) {
		static_assert(sizeof(impl::MemberCallable<ClassT, ReturnT, ArgsT...>) <= sizeof(m_callablePlaceholder), "Tell this error and the compiler you used to the author.");
		new (m_callablePlaceholder) impl::MemberCallable<ClassT, ReturnT, ArgsT...>(func, owner);
		m_callable = static_cast<impl::Callable<ReturnT, ArgsT...>*>(reinterpret_cast<impl::MemberCallable<ClassT, ReturnT, ArgsT...>*>(m_callablePlaceholder));
	}

	explicit operator bool() const {
		return m_callable && !m_callable->IsEmpty();
	}

	bool operator==(const Delegate& rhs) const {
		return (*this <=> rhs) == 0;
	}
	bool operator!=(const Delegate& rhs) const {
		return !(*this == rhs);
	}
	std::strong_ordering operator<=>(const Delegate& rhs) const {
		if (!m_callable || !rhs.m_callable) {
			return m_callable <=> rhs.m_callable;
		}
		return *m_callable <=> *rhs.m_callable;
	}

	ReturnT operator()(ArgsT... args) const {
		assert(operator bool());
		return m_callable->Call(std::forward<ArgsT>(args)...);
	}

private:
	alignas(impl::MemberCallable<Dummy, void>) char m_callablePlaceholder[sizeof(impl::MemberCallable<Dummy, void>)]; // actual stuff is placement newed here
	inl::impl::Callable<ReturnT, ArgsT...>* m_callable = nullptr;
};



template <class... ArgsT>
class Delegate<void(ArgsT...)> {
	class Dummy;

public:
	Delegate() = default;
	Delegate(const Delegate& rhs) {
		memcpy(m_callablePlaceholder, rhs.m_callablePlaceholder, sizeof(m_callablePlaceholder));
		m_callable = reinterpret_cast<impl::Callable<void, ArgsT...>*>(m_callablePlaceholder + ((size_t)rhs.m_callable - (size_t)rhs.m_callablePlaceholder));
	}
	Delegate& operator=(const Delegate& rhs) {
		memcpy(m_callablePlaceholder, rhs.m_callablePlaceholder, sizeof(m_callablePlaceholder));
		m_callable = reinterpret_cast<impl::Callable<void, ArgsT...>*>(m_callablePlaceholder + ((size_t)rhs.m_callable - (size_t)rhs.m_callablePlaceholder));
		return *this;
	}

	template <class ReturnT>
	Delegate(const Delegate<ReturnT, ArgsT...>& rhs) {
		memcpy(m_callablePlaceholder, rhs.m_callablePlaceholder, sizeof(m_callablePlaceholder));
		m_callable = reinterpret_cast<impl::Callable<ReturnT, ArgsT...>*>(m_callablePlaceholder + ((size_t)rhs.m_callable - (size_t)rhs.m_callablePlaceholder));
	}

	template <class ReturnT>
	Delegate& operator=(const Delegate<ReturnT, ArgsT...>& rhs) {
		memcpy(m_callablePlaceholder, rhs.m_callablePlaceholder, sizeof(m_callablePlaceholder));
		m_callable = reinterpret_cast<impl::Callable<ReturnT, ArgsT...>*>(m_callablePlaceholder + ((size_t)rhs.m_callable - (size_t)rhs.m_callablePlaceholder));
		return *this;
	}
	Delegate(Delegate&& rhs) : Delegate((const Delegate&)rhs) {}
	Delegate& operator=(Delegate&& rhs) { return *this = (const Delegate&)rhs; }

	template <class ReturnT>
	Delegate(ReturnT (*func)(ArgsT...)) {
		static_assert(sizeof(impl::GlobalCallable<ReturnT, ArgsT...>) <= sizeof(m_callablePlaceholder), "Tell this error and the compiler you used to the author.");
		new (m_callablePlaceholder) impl::GlobalCallable<ReturnT, ArgsT...>(func);
		m_callable = static_cast<impl::Callable<ReturnT, ArgsT...>*>(reinterpret_cast<impl::GlobalCallable<ReturnT, ArgsT...>*>(m_callablePlaceholder));
	}

	template <class ClassT, class ReturnT, typename std::enable_if<!std::is_const<ClassT>::value, int>::type = 0>
	Delegate(ReturnT (ClassT::*func)(ArgsT...), ClassT* owner) {
		static_assert(sizeof(impl::MemberCallable<ClassT, ReturnT, ArgsT...>) <= sizeof(m_callablePlaceholder), "Tell this error and the compiler you used to the author.");
		new (m_callablePlaceholder) impl::MemberCallable<ClassT, ReturnT, ArgsT...>(func, owner);
		m_callable = static_cast<impl::Callable<ReturnT, ArgsT...>*>(reinterpret_cast<impl::MemberCallable<ClassT, ReturnT, ArgsT...>*>(m_callablePlaceholder));
	}

	template <class ClassT, class ReturnT>
	Delegate(ReturnT (ClassT::*func)(ArgsT...) const, const ClassT* owner) {
		static_assert(sizeof(impl::MemberCallable<ClassT, ReturnT, ArgsT...>) <= sizeof(m_callablePlaceholder), "Tell this error and the compiler you used to the author.");
		new (m_callablePlaceholder) impl::MemberCallable<ClassT, ReturnT, ArgsT...>(func, owner);
		m_callable = static_cast<impl::Callable<ReturnT, ArgsT...>*>(reinterpret_cast<impl::MemberCallable<ClassT, ReturnT, ArgsT...>*>(m_callablePlaceholder));
	}

	explicit operator bool() const {
		return m_callable && !m_callable->IsEmpty();
	}

	bool operator==(const Delegate& rhs) const {
		return (*this <=> rhs) == 0;
	}
	bool operator!=(const Delegate& rhs) const {
		return !(*this == rhs);
	}
	std::strong_ordering operator<=>(const Delegate& rhs) const {
		if (!m_callable || !rhs.m_callable) {
			return m_callable <=> rhs.m_callable;
		}
		return *m_callable <=> *rhs.m_callable;
	}

	void operator()(ArgsT... args) const {
		assert(operator bool());
		m_callable->CallVoid(std::forward<ArgsT>(args)...);
	}

private:
	alignas(impl::MemberCallable<Dummy, void>) char m_callablePlaceholder[sizeof(impl::MemberCallable<Dummy, void>)]; // actual stuff is placement newed here
	impl::CallableBase<ArgsT...>* m_callable = nullptr;
};



template <class... ArgsT>
bool impl::CallableBase<ArgsT...>::operator==(const CallableBase& rhs) const {
	return (*this <=> rhs) == 0;
}

template <class... ArgsT>
bool impl::CallableBase<ArgsT...>::operator!=(const CallableBase& rhs) const {
	return !(*this == rhs);
}


template <class... ArgsT>
std::strong_ordering impl::CallableBase<ArgsT...>::operator<=>(const CallableBase& rhs) const {
	auto lhsTuple = std::make_tuple(IsClass(), GetClassPtr(), GetClassType(), GetFuncPtrSize());
	auto rhsTuple = std::make_tuple(rhs.IsClass(), rhs.GetClassPtr(), rhs.GetClassType(), rhs.GetFuncPtrSize());
	auto cmp = lhsTuple < rhsTuple ? std::strong_ordering::less : (lhsTuple == rhsTuple ? std::strong_ordering::equal : std::strong_ordering::greater);
	if (cmp == 0) {
#ifdef _MSC_VER
		// MSVC stores the member function's address in the first PTR of the MFP structure.
		// It's enough to compare that, the rest is just undecipherable garbage because of its shitty implementation.
		return 0 <=> memcmp(GetFuncPtr(), rhs.GetFuncPtr(), sizeof(void*));
#else
		return 0 <=> memcmp(GetFuncPtr(), rhs.GetFuncPtr(), GetFuncPtrSize());
#endif
	}
	return cmp;
}


} // namespace inl
