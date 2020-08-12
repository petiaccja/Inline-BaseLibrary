#pragma once

#include "../Any.hpp"
#include "../TemplateUtil.hpp"
#include "SerializableNode.hpp"

#include <any>
#include <cassert>
#include <concepts>
#include <functional>
#include <memory>
#include <optional>
#include <set>
#include <type_traits>
#include <typeindex>
#include <variant>


namespace inl {

class InputPortBase;
class OutputPortBase;
class NodeBase;

template <class T>
class OutputPort;

template <class T>
class InputPort;


template <class Func>
concept InputPortConversionQueryable = requires(Func func, std::type_index type) {
	{ func(type) }
	->std::same_as<bool>;
};

template <class Func>
concept InputPortConversionCallable = requires(Func func, std::any value) {
	{ func(value) }
	->std::same_as<std::any>;
};

template <class Func>
concept InputPortConverter = InputPortConversionQueryable<Func>&& InputPortConversionCallable<Func>;


template <class Func, class T>
concept InputPortAssigner = requires(Func func, T& lhs, const T& rhs) {
	func(lhs, rhs);
};


/// <summary>
/// <para> Output port of a node. </para>
/// <para>
/// Output ports are attached to nodes. They can be linked to
/// input ports. A node can activate them with data, and that data
/// is forwarded to connected input ports. An output port can be linked
/// to multiple input ports at the same time.
/// </para>
/// </summary>
class OutputPortBase : public ISerializableOutputPort {
public:
	using LinkIterator = std::set<InputPortBase*>::iterator;
	using ConstLinkIterator = std::set<InputPortBase*>::const_iterator;

public:
	OutputPortBase();
	~OutputPortBase();

	/// <summary> Get typeid of underlying data. </summary>
	virtual std::type_index GetType() const = 0;

	/// <summary> Link to an input port. </summary>
	/// <returns> True if succesfully linked. Make sures types are compatible. </returns>
	/// <exception cref="InvalidArgumentException"> In case the other port is already linked or types are incompatible. </exception>
	void Link(InputPortBase& destination);

	/// <summary> Remove link between this and the other end. </summary>
	/// <param param="other"> The port to unlink from this. </param>
	virtual void Unlink(InputPortBase& other);

	/// <summary> Return true if the port is linked to at least one input port. </summary>
	bool IsLinked() const;

	/// <summary> Returns the number of input ports this port is linked to. </summary>
	size_t GetNumLinks() const;

	/// <summary> Unlink all ports from this. </summary>
	virtual void UnlinkAll();

	// Iterate over links.
	LinkIterator begin();
	LinkIterator end();
	ConstLinkIterator begin() const;
	ConstLinkIterator end() const;
	ConstLinkIterator cbegin() const;
	ConstLinkIterator cend() const;

protected:
	std::set<InputPortBase*> links;
};



/// <summary>
/// <para> Input port of a Node. </para>
/// <para>
/// Input ports are attached to a node. An input port can also be linked
/// to an output port, from where it receives the data.
/// </para>
/// </summary>
class InputPortBase : public ISerializableInputPort {
	friend class OutputPortBase;
	friend class OutputPort<Any>;

public:
	InputPortBase();
	~InputPortBase();

	/// <summary> Attempts to set an arbitrary type. Does type conversion if possible. </summary>
	/// <exception cref="InvalidArgumentException"> If type could not be converted. </exception>
	virtual void Set(std::any value) = 0;

	/// <summary> Clear currently stored data. </summary>
	virtual void Clear() = 0;

	/// <summary> Get weather any valid data is set. </summary>
	virtual bool IsSet() const override = 0;

	/// <summary> Converts underlying data to string using it's &lt;&lt; operator </summary>
	/// <exception cref="InvalidCallException"> If no ostream operator available. </exception>
	virtual std::string ToString() const override = 0;

	/// <summary> Get typeid of underlying data. </summary>
	virtual std::type_index GetType() const = 0;

	/// <summary> Get if can convert from certain type. </summary>
	virtual bool IsCompatible(std::type_index type) const = 0;

	/// <summary> Link this port to an output port. </summary>
	/// <exception cref="InvalidStateException"> In case the port is already linked. </exception>
	/// <exception cref="InvalidArgumentException">  If types are incompatible. </exception>
	void Link(OutputPortBase& source);

	/// <summary> Remove link between this and the other end. </summary>
	void Unlink();

	/// <summary> Return true if the port is linked to an output port. </summary>
	bool IsLinked() const;

	/// <summary> Get which output port it is linked to. </summary>
	/// <returns> The other end. Null if not linked. </returns>
	OutputPortBase& GetLink() const override;

protected:
	OutputPortBase* link;

private:
	// should only be called by an output port when it's ready with building up the linkage
	// this function only sets internal state of the inputport to represent the link set up by outputport
	void SetLinkState(OutputPortBase* link);
};



/// <summary>
/// <para> Specialization of InputPortBase for various types of data. </para>
/// <para> Different types can be set as template parameter. Generally, it's enough to
/// just use this template, but it may be necessary to specialize this template
/// for certain data types to improve efficiency or change behaviour. </para>
/// </summary>
template <class T>
class InputPort : public InputPortBase {
public:
	InputPort() = default;

	template <InputPortConverter Func>
	explicit InputPort(Func convert);

	template <InputPortAssigner<T> Func>
	explicit InputPort(Func assign);

	template <InputPortConverter ConvertFunc, InputPortAssigner<T> AssignFunc>
	InputPort(ConvertFunc convert, AssignFunc assign);

	InputPort(const InputPort&) = default;
	InputPort(InputPort&&) = default;
	InputPort& operator=(const InputPort&) = default;
	InputPort& operator=(InputPort&&) = default;

	void Set(std::any value) override;

	/// <summary> Set an object as input to this port. </summary>
	/// <remarks> This is normally called by linked output ports, but may as well be called manually. </remarks>
	void Set(T value);

	/// <summary> Set an object as input to this port. </summary>
	void operator<<(T value) { Set(std::move(value)); }

	/// <summary> Get the data that was previously set. </summary>
	/// <exception cref="InvalidStateException"> If no data is set. </exception>
	T& Get();

	/// <summary> Get the data that was previously set. </summary>
	/// <exception cref="InvalidStateException"> If no data is set. </exception>
	const T& Get() const;

	/// <summary> Get the data that was previously set. </summary>
	/// <exception cref="InvalidStateException"> If no data is set. </exception>
	T& operator*() { return Get(); }

	/// <summary> Get the data that was previously set. </summary>
	/// <exception cref="InvalidStateException"> If no data is set. </exception>
	const T& operator*() const { return Get(); }

	/// <summary> Clear any data currently set on this port. </summary>
	void Clear() override;

	/// <summary> Get whether any data has been set. </summary>
	bool IsSet() const override;

	/// <summary> Get the underlying data type. </summary>
	std::type_index GetType() const override;

	std::string ToString() const override;

	virtual bool IsCompatible(std::type_index type) const override;

private:
	T Convert(std::any value);
	template <InputPortConverter Func>
	static auto MakeConvertWrapper(Func convert);
	template <InputPortAssigner<T> Func>
	static auto MakeAssignWrapper(Func assign);

private:
	std::optional<T> m_data;
	std::function<std::variant<std::any, bool>(std::variant<std::any, std::type_index>)> m_convert;
	std::function<void(T&, T)> m_assign;
};


template <class T>
template <InputPortConverter Func>
InputPort<T>::InputPort(Func convert) : m_convert{ MakeConvertWrapper(std::move(convert)) } {}


template <class T>
template <InputPortAssigner<T> Func>
InputPort<T>::InputPort(Func assign) : m_assign{ MakeAssignWrapper(std::move(assign)) } {}


template <class T>
template <InputPortConverter ConvertFunc, InputPortAssigner<T> AssignFunc>
InputPort<T>::InputPort(ConvertFunc convert, AssignFunc assign)
	: m_convert{ MakeConvertWrapper(std::move(convert)) },
	  m_assign{ MakeAssignWrapper(std::move(assign)) } {}


template <class T>
void InputPort<T>::Set(std::any value) {
	T raw = Convert(value);

	if (m_data && m_assign) {
		m_assign(m_data.value(), std::move(raw));
	}
	else {
		m_data = std::move(raw);
	}
}

template <class T>
void InputPort<T>::Set(T value) {
	if (m_data && m_assign) {
		m_assign(m_data.value(), std::move(value));
	}
	else {
		m_data = std::move(value);
	}
}

template <class T>
T& InputPort<T>::Get() {
	if (!m_data) {
		throw InvalidStateException("There is not value set.");
	}
	return m_data.value();
}

template <class T>
const T& InputPort<T>::Get() const {
	if (!m_data) {
		throw InvalidStateException("There is not value set.");
	}
	return m_data.value();
}

template <class T>
void InputPort<T>::Clear() {
	m_data = {};
}

template <class T>
bool InputPort<T>::IsSet() const {
	return bool(m_data);
}

template <class T>
std::type_index InputPort<T>::GetType() const {
	return typeid(T);
}

template <class T>
std::string InputPort<T>::ToString() const {
	throw NotImplementedException();
}

template <class T>
bool InputPort<T>::IsCompatible(std::type_index type) const {
	if (type == GetType() || type == typeid(std::any)) {
		return true;
	}
	else {
		if (!m_convert) {
			return false;
		}
		auto visitor = [](auto&& arg) -> bool {
			if constexpr (std::is_same_v<bool, std::decay_t<decltype(arg)>>) {
				return arg;
			}
			return false;
		};
		return std::visit(visitor, m_convert(std::variant<std::any, std::type_index>{ std::in_place_type<std::type_index>, type }));
	}
}

template <class T>
T InputPort<T>::Convert(std::any value) {
	if (std::type_index(value.type()) == GetType()) {
		return std::any_cast<T>(std::move(value));
	}

	if (!m_convert) {
		throw InvalidArgumentException("Argument cannot be converted to port type.");
	}

	auto visitor = [](auto&& arg) -> std::any {
		if constexpr (std::is_same_v<std::any, std::decay_t<decltype(arg)>>) {
			return arg;
		}
		return {};
	};
	std::any converted = std::visit(visitor, m_convert(value));
	assert(converted.type() == GetType());
	return std::any_cast<T>(std::move(converted));
}

template <class T>
template <InputPortConverter Func>
auto InputPort<T>::MakeConvertWrapper(Func convert) {
	return [convertFun = std::move(convert)](std::variant<std::any, std::type_index> arg) {
		auto visitor = [&convertFun](auto&& singleArg) {
			auto ret = convertFun(singleArg);
			return std::variant<std::any, bool>{ ret };
		};
		return std::visit(visitor, arg);
	};
}

template <class T>
template <InputPortAssigner<T> Func>
auto InputPort<T>::MakeAssignWrapper(Func assign) {
	return [assignFun = std::move(assign)](T& lhs, T rhs) {
		assignFun(lhs, std::move(rhs));
	};
}


/// <summary>
/// Specialization of OutputPortBase for various types of data.
/// Different types can be set as template parameter. Generally, it's enough to
/// just use this template, but it may be necessary to specialize this template
/// for certain data types to improve efficiency or change behaviour.
/// </summary>
template <class T>
class OutputPort : public OutputPortBase {
public:
	// ctors
	OutputPort() = default;
	OutputPort(const OutputPort&) = default;
	OutputPort(OutputPort&&) = default;
	OutputPort& operator=(const OutputPort&) = default;
	OutputPort& operator=(OutputPort&&) = default;

	/// <summary> This value is forwarded to each input port linked to this one. </summary>
	void Set(const T& value);

	/// <summary> This value is forwarded to each input port linked to this one. </summary>
	void Set(T&& value);

	/// <summary> This value is forwarded to each input port linked to this one. </summary>
	void operator<<(const T& value) {
		Set(value);
	}

	/// <summary> This value is forwarded to each input port linked to this one. </summary>
	void operator<<(T&& value) {
		Set(std::move(value));
	}

	/// <summary> Get type of underlying data. </summary>
	std::type_index GetType() const override {
		return typeid(T);
	}
};


template <class T>
void OutputPort<T>::Set(const T& value) {
	for (auto v : links) {
		if (v->GetType() == GetType()) {
			static_cast<InputPort<T>*>(v)->Set(value);
		}
		else {
			v->Set(std::any(value));
		}
	}
}

template <class T>
void OutputPort<T>::Set(T&& value) {
	auto first = links.begin();
	auto last = links.end();
	if (first != last) {
		--last;
	}

	for (; first != last; ++first) {
		auto v = *first;
		if (v->GetType() == GetType()) {
			static_cast<InputPort<T>*>(v)->Set(value);
		}
		else {
			v->Set(std::any(value));
		}
	}
	if (first != links.end()) {
		auto v = *first;
		if (v->GetType() == GetType()) {
			static_cast<InputPort<T>*>(v)->Set(std::move(value));
		}
		else {
			v->Set(std::any(std::move(value)));
		}
	}
}


//------------------------------------------------------------------------------
// Void specializations.
//------------------------------------------------------------------------------

template <>
class InputPort<void> : public InputPortBase {
public:
	InputPort() = default;
	InputPort(const InputPort&) = default;
	InputPort(InputPort&&) = default;
	InputPort& operator=(const InputPort&) = default;
	InputPort& operator=(InputPort&&) = default;

	void Set(std::any value) override { isSet = true; }

	/// <summary> Set an object as input to this port. </summary>
	/// <remarks> This is normally called by linked output ports, but may as well be called manually. </remarks>
	void Set() { isSet = true; }

	/// <summary> Clear any data currently set on this port. </summary>
	void Clear() override { isSet = false; }

	/// <summary> Get whether any data has been set. </summary>
	bool IsSet() const override { return isSet; }

	/// <summary> Get the underlying data type. </summary>
	std::type_index GetType() const override { return typeid(void); }

	std::string ToString() const override { return ""; }

	virtual bool IsCompatible(std::type_index type) const override { return true; }

private:
	bool isSet = false;
};


template <>
class OutputPort<void> : public OutputPortBase {
public:
	// ctors
	OutputPort() = default;
	OutputPort(const OutputPort&) = default;
	OutputPort(OutputPort&&) = default;
	OutputPort& operator=(const OutputPort&) = default;
	OutputPort& operator=(OutputPort&&) = default;

	void Set() {
		for (auto v : links) {
			assert(v->GetType() == GetType());
			static_cast<InputPort<void>*>(v)->Set();
		}
	}

	/// <summary> Get type of underlying data. </summary>
	std::type_index GetType() const override { return typeid(void); }
};


//------------------------------------------------------------------------------
// Any specializations.
//------------------------------------------------------------------------------


template <>
class InputPort<std::any> : public InputPortBase {
public:
	InputPort() = default;
	InputPort(const InputPort&) = default;
	InputPort(InputPort&&) = default;
	InputPort& operator=(const InputPort&) = default;
	InputPort& operator=(InputPort&&) = default;

	void Set(std::any value) override;

	/// <summary> Get the data that was previously set. </summary>
	/// <exception cref="InvalidStateException"> If no data is set. </exception>
	std::any& Get();

	/// <summary> Get the data that was previously set. </summary>
	/// <exception cref="InvalidStateException"> If no data is set. </exception>
	const std::any& Get() const;

	/// <summary> Clear any data currently set on this port. </summary>
	void Clear() override;

	/// <summary> Get whether any data has been set. </summary>
	bool IsSet() const override;

	/// <summary> Get the underlying data type. </summary>
	std::type_index GetType() const override;

	std::string ToString() const override;

	virtual bool IsCompatible(std::type_index type) const override;

private:
	std::optional<std::any> data;
};

/// <summary>
/// Specialization of OutputPortBase for various types of data.
/// Different types can be set as template parameter. Generally, it's enough to
/// just use this template, but it may be necessary to specialize this template
/// for certain data types to improve efficiency or change behaviour.
/// </summary>
template <>
class OutputPort<std::any> : public OutputPortBase {
public:
	// ctors
	OutputPort() = default;
	OutputPort(const OutputPort&) = default;
	OutputPort(OutputPort&&) = default;
	OutputPort& operator=(const OutputPort&) = default;
	OutputPort& operator=(OutputPort&&) = default;

	/// <summary> This value is forwarded to each input port linked to this one. </summary>
	void Set(const std::any& value);

	/// <summary> This value is forwarded to each input port linked to this one. </summary>
	void Set(std::any&& data);

	/// <summary> This value is forwarded to each input port linked to this one. </summary>
	void operator<<(const std::any& value) {
		Set(value);
	}

	/// <summary> This value is forwarded to each input port linked to this one. </summary>
	void operator<<(std::any&& value) {
		Set(std::move(value));
	}

	/// <summary> Get type of underlying data. </summary>
	std::type_index GetType() const override {
		return typeid(std::any);
	}
};


} // namespace inl