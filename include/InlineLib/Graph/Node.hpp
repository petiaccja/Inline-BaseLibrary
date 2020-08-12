#pragma once

// inheritence by dominance
// derived classes of *PortConfig inherit methods from multiple sources or smth like that
#ifdef _MSC_VER
#pragma warning(disable : 4250)
#endif

#include "Port.hpp"

#include <cstddef>
#include <initializer_list>
#include <regex>
#include <string>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <vector>


#undef GetClassName // retarded Windows


namespace inl {


class InputPortBase;


/// <summary>
/// <para> Base class for nodes. </para>
/// <para>
/// Nodes are basic unit of data manipulation. They are connected into
/// a network, which has some sources and sinks. Such networks help visually
/// implement data processing and logic, where each node transform the data
/// in some way.
/// </para>
/// <para>
///	A single node can have multiple input port and multiple output ports.
/// Each port is typed, that is, they may be float, int, or something else.
/// An output port of a node can be connected to the input port of another,
/// but only if their types are compatible. This way, the network is created.
/// </para>
/// <para>
/// To create your own nodes, override methods of this interface. Nodes may
/// be registered to the node factory if they have default constructor and
/// implement the required static methods. To quickly add ports, inherit from
/// InputPortConfig and/or OutputPortConfig.
/// </para>
/// </summary>
class NodeBase : public ISerializableNode {
public:
	virtual ~NodeBase() = default;

	/// <summary> Returns the number of input ports. </summary>
	virtual size_t GetNumInputs() const override = 0;
	/// <summary> Returns the number of output ports. </summary>
	virtual size_t GetNumOutputs() const override = 0;

	/// <summary> Get pointer to the indexth input port. </summary>
	virtual InputPortBase& GetInput(size_t index) override = 0;
	/// <summary> Get pointer to the indexth output port. </summary>
	virtual OutputPortBase& GetOutput(size_t index) override = 0;

	/// <summary> Get pointer to the indexth input port. </summary>
	virtual const InputPortBase& GetInput(size_t index) const override = 0;
	/// <summary> Get pointer to the indexth output port. </summary>
	virtual const OutputPortBase& GetOutput(size_t index) const override = 0;

	/// <summary> Read and process input ports and activate output. </summary>
	virtual void Update() = 0;

	/// <summary> Returns the name of the input port. This is optionally specified for the node class. </summary>
	virtual const std::string& GetInputName(size_t index) const override {
		static const std::string n = "";
		return n;
	}
	/// <summary> Returns the name of the output port. This is optionally specified for the node class. </summary>
	virtual const std::string& GetOutputName(size_t index) const override {
		static const std::string n = "";
		return n;
	}

	/// <summary> Sets a name for the node so that a graph drawing is more readable. </summary>
	void SetDisplayName(std::string name) { m_displayName = name; }
	/// <summary> Gets the drawing name of the node. </summary>
	const std::string& GetDisplayName() const override { return m_displayName; }

	/// <summary> Gets the class name of the node. </summary>
	/// <remarks> By default, this function returns the C++ class name of the node.
	///		Override this function if you don't like the C++ class name. </remarks>
	virtual std::string GetClassName(bool simplify, const std::vector<std::regex>& additional = {}) const;

	virtual std::string GetClassName() const override { return GetClassName(false); }

protected:
	std::string m_displayName;
};



/// <summary>
/// A list for the types of input ports used in conjunction with <see cref="Node"/>.
/// </summary>
/// <typeparam name="...InputTypes"> Types of the input ports of the node. </typeparam>
template <class... InputTypes>
class InputPorts {};

/// <summary>
/// A list for the types of output ports used in conjunction with <see cref="Node"/>.
/// </summary>
/// <typeparam name="...OutputTypes"> Types of the output ports of the node. </typeparam>
template <class... OutputTypes>
class OutputPorts {};

template <class InputPorts, class OutputPorts>
class Node;

/// <summary> A node with a fixed set of input and output ports. </summary>
/// <typeparam name="...InputTypes"> Types of the input ports of the node. </typeparam>
/// <typeparam name="...OutputTypes"> Types of the output ports of the node. </typeparam>
template <class... InputTypes, class... OutputTypes>
class Node<InputPorts<InputTypes...>, OutputPorts<OutputTypes...>> : public NodeBase {
public:
	Node() = default;
	Node(InputPort<InputTypes>&&... inputs, OutputPort<OutputTypes>&&... outputs);

	template <InputPortConverter Func>
	Node(Func commonConverter);

	size_t GetNumInputs() const override;
	size_t GetNumOutputs() const override;

	const InputPortBase& GetInput(size_t index) const override;
	InputPortBase& GetInput(size_t index) override;

	const OutputPortBase& GetOutput(size_t index) const override;
	OutputPortBase& GetOutput(size_t index) override;

	template <size_t Index, class... InputTypes_, class... OutputTypes_>
	friend const auto& GetInput(const Node<InputPorts<InputTypes_...>, OutputPorts<OutputTypes_...>>& node);
	template <size_t Index, class... InputTypes_, class... OutputTypes_>
	friend auto& GetInput(Node<InputPorts<InputTypes_...>, OutputPorts<OutputTypes_...>>& node);
	template <size_t Index, class... InputTypes_, class... OutputTypes_>
	friend const auto& GetOutput(const Node<InputPorts<InputTypes_...>, OutputPorts<OutputTypes_...>>& node);
	template <size_t Index, class... InputTypes_, class... OutputTypes_>
	friend auto& GetOutput(Node<InputPorts<InputTypes_...>, OutputPorts<OutputTypes_...>>& node);

private:
	template <class Return, size_t TrialIndex, class Tuple>
	static Return DynamicGet(size_t index, Tuple&& tuple) {
		if constexpr (TrialIndex < std::tuple_size_v<std::decay_t<Tuple>>) {
			return TrialIndex == index ? std::get<TrialIndex>(std::forward<Tuple>(tuple)) : DynamicGet<Return, TrialIndex + 1>(index, std::forward<Tuple>(tuple));
		}
		else {
			throw OutOfRangeException("Index larger than tuple size.");
		}
	}

private:
	std::tuple<InputPort<InputTypes>...> m_inputs;
	std::tuple<OutputPort<OutputTypes>...> m_outputs;
};


template <class... InputTypes, class... OutputTypes>
Node<InputPorts<InputTypes...>, OutputPorts<OutputTypes...>>::Node(InputPort<InputTypes>&&... inputs, OutputPort<OutputTypes>&&... outputs)
	: m_inputs{ std::forward<InputTypes>(inputs)... },
	  m_outputs{ std::forward<OutputTypes>(outputs)... } {}


template <class... InputTypes, class... OutputTypes>
template <InputPortConverter Func>
Node<InputPorts<InputTypes...>, OutputPorts<OutputTypes...>>::Node(Func commonConverter)
	: m_inputs{ InputPort<InputTypes>{ commonConverter }... },
	  m_outputs{} {}


template <class... InputTypes, class... OutputTypes>
size_t Node<InputPorts<InputTypes...>, OutputPorts<OutputTypes...>>::GetNumInputs() const {
	return std::tuple_size_v<std::decay_t<decltype(m_inputs)>>;
}

template <class... InputTypes, class... OutputTypes>
size_t Node<InputPorts<InputTypes...>, OutputPorts<OutputTypes...>>::GetNumOutputs() const {
	return std::tuple_size_v<std::decay_t<decltype(m_outputs)>>;
}

template <class... InputTypes, class... OutputTypes>
const InputPortBase& Node<InputPorts<InputTypes...>, OutputPorts<OutputTypes...>>::GetInput(size_t index) const {
	return DynamicGet<const InputPortBase&, 0>(index, m_inputs);
}

template <class... InputTypes, class... OutputTypes>
InputPortBase& Node<InputPorts<InputTypes...>, OutputPorts<OutputTypes...>>::GetInput(size_t index) {
	return DynamicGet<InputPortBase&, 0>(index, m_inputs);
}

template <class... InputTypes, class... OutputTypes>
const OutputPortBase& Node<InputPorts<InputTypes...>, OutputPorts<OutputTypes...>>::GetOutput(size_t index) const {
	return DynamicGet<const OutputPortBase&, 0>(index, m_outputs);
}

template <class... InputTypes, class... OutputTypes>
OutputPortBase& Node<InputPorts<InputTypes...>, OutputPorts<OutputTypes...>>::GetOutput(size_t index) {
	return DynamicGet<OutputPortBase&, 0>(index, m_outputs);
}


template <size_t Index, class... InputTypes_, class... OutputTypes_>
const auto& GetInput(const Node<InputPorts<InputTypes_...>, OutputPorts<OutputTypes_...>>& node) {
	return std::get<Index>(node.m_inputs);
}
template <size_t Index, class... InputTypes_, class... OutputTypes_>
auto& GetInput(Node<InputPorts<InputTypes_...>, OutputPorts<OutputTypes_...>>& node) {
	return std::get<Index>(node.m_inputs);
}
template <size_t Index, class... InputTypes_, class... OutputTypes_>
const auto& GetOutput(const Node<InputPorts<InputTypes_...>, OutputPorts<OutputTypes_...>>& node) {
	return std::get<Index>(node.m_outputs);
}
template <size_t Index, class... InputTypes_, class... OutputTypes_>
auto& GetOutput(Node<InputPorts<InputTypes_...>, OutputPorts<OutputTypes_...>>& node) {
	return std::get<Index>(node.m_outputs);
}



} // namespace inl
