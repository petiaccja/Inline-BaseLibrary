#include <InlineLib/Graph.hpp>

#include <Catch2/catch.hpp>

using namespace inl;


template <class Target>
struct ArithmeticConverter {
	std::any operator()(std::any value) const {
		if (value.type() == typeid(float)) {
			return Target(std::any_cast<const float&>(value));
		}
		if (value.type() == typeid(double)) {
			return Target(std::any_cast<const double&>(value));
		}
		if (value.type() == typeid(int)) {
			return Target(std::any_cast<const int&>(value));
		}
		if (value.type() == typeid(unsigned)) {
			return Target(std::any_cast<const unsigned&>(value));
		}
		throw InvalidArgumentException("Cannot convert argument to given type.");
	}
	bool operator()(std::type_index type) const {
		if (type == typeid(float)) {
			return true;
		}
		if (type == typeid(double)) {
			return true;
		}
		if (type == typeid(int)) {
			return true;
		}
		if (type == typeid(unsigned)) {
			return true;
		}
		return false;
	}
};


//--------------------------------------
// Link normal ports
//--------------------------------------
TEST_CASE("Link ports same type", "[Graph]") {
	InputPort<float> inputPort;
	OutputPort<float> outputPort;
	inputPort.Link(outputPort);
	REQUIRE_NOTHROW(inputPort.GetLink());
	REQUIRE(inputPort.IsLinked());
	REQUIRE(outputPort.IsLinked());
	REQUIRE(outputPort.GetNumLinks() == 1);
}

TEST_CASE("Link ports same type reverse", "[Graph]") {
	InputPort<float> inputPort;
	OutputPort<float> outputPort;
	outputPort.Link(inputPort);
	REQUIRE_NOTHROW(inputPort.GetLink());
	REQUIRE(inputPort.IsLinked());
	REQUIRE(outputPort.IsLinked());
}

TEST_CASE("Link ports incompatible types", "[Graph]") {
	InputPort<float> inputPort;
	OutputPort<char*> outputPort;
	REQUIRE_THROWS(inputPort.Link(outputPort));
	REQUIRE_THROWS(inputPort.GetLink());
	REQUIRE(!inputPort.IsLinked());
	REQUIRE(!outputPort.IsLinked());
	REQUIRE(outputPort.GetNumLinks() == 0);
}

TEST_CASE("Link ports convertible types", "[Graph]") {
	InputPort<float> inputPort{ ArithmeticConverter<float>{} };
	OutputPort<int> outputPort;
	inputPort.Link(outputPort);
	REQUIRE_NOTHROW(inputPort.GetLink());
	REQUIRE(inputPort.IsLinked());
	REQUIRE(outputPort.IsLinked());
}

//--------------------------------------
// Link void ports
//--------------------------------------
TEST_CASE("Create and link void ports", "[Graph]") {
	InputPort<void> inputPort;
	OutputPort<void> outputPort;
	outputPort.Link(inputPort);
	REQUIRE_NOTHROW(inputPort.GetLink());
	REQUIRE(inputPort.IsLinked());
	REQUIRE(outputPort.IsLinked());
}

TEST_CASE("Link void input ports to non-void outputs", "[Graph]") {
	InputPort<void> inputPort;
	OutputPort<int> outputPort;
	outputPort.Link(inputPort);
	REQUIRE_NOTHROW(inputPort.GetLink());
	REQUIRE(inputPort.IsLinked());
	REQUIRE(outputPort.IsLinked());
}

TEST_CASE("Link void output ports to non-void inputs", "[Graph]") {
	InputPort<int> inputPort;
	OutputPort<void> outputPort;
	REQUIRE_THROWS(outputPort.Link(inputPort));
	REQUIRE_THROWS(inputPort.GetLink());
	REQUIRE(!inputPort.IsLinked());
	REQUIRE(!outputPort.IsLinked());
}

//--------------------------------------
// Link any ports.
//--------------------------------------

TEST_CASE("Create and link anytype ports", "[Graph]") {
	InputPort<std::any> inputPort;
	OutputPort<std::any> outputPort;
	outputPort.Link(inputPort);
	REQUIRE_NOTHROW(inputPort.GetLink());
	REQUIRE(inputPort.IsLinked());
	REQUIRE(outputPort.IsLinked());
}

TEST_CASE("Link anytype input ports to typed outputs", "[Graph]") {
	InputPort<std::any> inputPort;
	OutputPort<int> outputPort;
	outputPort.Link(inputPort);
	REQUIRE_NOTHROW(inputPort.GetLink());
	REQUIRE(inputPort.IsLinked());
	REQUIRE(outputPort.IsLinked());
}

TEST_CASE("Link typed output ports to anytype inputs", "[Graph]") {
	InputPort<int> inputPort;
	OutputPort<std::any> outputPort;
	REQUIRE_NOTHROW(outputPort.Link(inputPort));
	REQUIRE(inputPort.IsLinked());
	REQUIRE(outputPort.IsLinked());
}


//--------------------------------------
// Multiple ports and unlinking
//--------------------------------------

TEST_CASE("Link multiple ports", "[Graph]") {
	InputPort<int> inputPort1;
	InputPort<int> inputPort2;
	InputPort<int> inputPort3;
	OutputPort<int> outputPort;
	outputPort.Link(inputPort1);
	outputPort.Link(inputPort2);
	outputPort.Link(inputPort3);

	REQUIRE(outputPort.GetNumLinks() == 3);
	REQUIRE(inputPort1.IsLinked());
	REQUIRE(inputPort2.IsLinked());
	REQUIRE(inputPort3.IsLinked());
}

TEST_CASE("Unlink one port", "[Graph]") {
	InputPort<int> inputPort1;
	InputPort<int> inputPort2;
	InputPort<int> inputPort3;
	OutputPort<int> outputPort;
	outputPort.Link(inputPort1);
	outputPort.Link(inputPort2);
	outputPort.Link(inputPort3);

	inputPort2.Unlink();

	REQUIRE(outputPort.GetNumLinks() == 2);
	REQUIRE(inputPort1.IsLinked());
	REQUIRE(!inputPort2.IsLinked());
	REQUIRE(inputPort3.IsLinked());
}

TEST_CASE("Unlink all ports", "[Graph]") {
	InputPort<int> inputPort1;
	InputPort<int> inputPort2;
	InputPort<int> inputPort3;
	OutputPort<int> outputPort;
	outputPort.Link(inputPort1);
	outputPort.Link(inputPort2);
	outputPort.Link(inputPort3);

	outputPort.UnlinkAll();

	REQUIRE(outputPort.GetNumLinks() == 0);
	REQUIRE(!inputPort1.IsLinked());
	REQUIRE(!inputPort2.IsLinked());
	REQUIRE(!inputPort3.IsLinked());
}


//--------------------------------------
// Setting data
//--------------------------------------
TEST_CASE("Uninitialized input port", "[Graph]") {
	InputPort<int> inputPort;
	REQUIRE(!inputPort.IsSet());
	REQUIRE_THROWS(inputPort.Get());
}

TEST_CASE("Set data", "[Graph]") {
	InputPort<int> inputPort;
	inputPort.Set(10);

	REQUIRE(inputPort.IsSet());
	REQUIRE(inputPort.Get() == 10);
}

TEST_CASE("Clear data", "[Graph]") {
	InputPort<int> inputPort;
	inputPort.Set(10);
	inputPort.Clear();

	REQUIRE(!inputPort.IsSet());
	REQUIRE_THROWS(inputPort.Get());
}

TEST_CASE("Set data via link", "[Graph]") {
	InputPort<int> inputPort;
	OutputPort<int> outputPort;
	outputPort.Link(inputPort);

	outputPort.Set(10);
	REQUIRE(inputPort.Get() == 10);
}

TEST_CASE("Set converted data via link", "[Graph]") {
	InputPort<float> inputPort{ ArithmeticConverter<float>{} };
	OutputPort<int> outputPort;
	outputPort.Link(inputPort);

	outputPort.Set(10);
	REQUIRE(inputPort.Get() == 10.f);
}

TEST_CASE("Set data via multiple links move", "[Graph]") {
	InputPort<int> inputPort1;
	InputPort<int> inputPort2;
	InputPort<int> inputPort3;
	OutputPort<int> outputPort;
	outputPort.Link(inputPort1);
	outputPort.Link(inputPort2);
	outputPort.Link(inputPort3);

	outputPort.Set(10);
	REQUIRE(inputPort1.IsSet());
	REQUIRE(inputPort2.IsSet());
	REQUIRE(inputPort3.IsSet());
	REQUIRE(inputPort1.Get() == 10);
	REQUIRE(inputPort2.Get() == 10);
	REQUIRE(inputPort3.Get() == 10);
}


TEST_CASE("Set data via multiple links copy", "[Graph]") {
	InputPort<int> inputPort1;
	InputPort<int> inputPort2;
	InputPort<int> inputPort3;
	OutputPort<int> outputPort;
	outputPort.Link(inputPort1);
	outputPort.Link(inputPort2);
	outputPort.Link(inputPort3);

	int value = 11;
	outputPort.Set(value);
	REQUIRE(inputPort1.IsSet());
	REQUIRE(inputPort2.IsSet());
	REQUIRE(inputPort3.IsSet());
	REQUIRE(inputPort1.Get() == 11);
	REQUIRE(inputPort2.Get() == 11);
	REQUIRE(inputPort3.Get() == 11);
}


//--------------------------------------
// Setting data through any ports
//--------------------------------------

TEST_CASE("Set anytype data via link, input", "[Graph]") {
	InputPort<std::any> inputPort;
	OutputPort<int> outputPort;
	outputPort.Link(inputPort);

	outputPort.Set(10);
	REQUIRE(std::any_cast<int>(inputPort.Get()) == 10);
}

TEST_CASE("Set anytype data via link, output", "[Graph]") {
	InputPort<int> inputPort;
	OutputPort<std::any> outputPort;
	outputPort.Link(inputPort);

	outputPort.Set(10);
	REQUIRE(inputPort.Get() == 10);
}

TEST_CASE("Set anytype data via link w/ conversion", "[Graph]") {
	InputPort<int> inputPort{ ArithmeticConverter<int>{} };
	OutputPort<std::any> outputPort;
	outputPort.Link(inputPort);

	outputPort.Set(10.0f);
	REQUIRE(inputPort.Get() == 10);
}


//--------------------------------------
// Special assigners
//--------------------------------------

TEST_CASE("Special assigner", "[Graph]") {
	OutputPort<int> outputPort;
	InputPort<int> inputPort{ [](int& lhs, int rhs) { lhs += rhs; } };
	inputPort.Link(outputPort);

	outputPort.Set(1);
	outputPort.Set(1);
	outputPort.Set(3);

	REQUIRE(inputPort.Get() == 5);
}

TEST_CASE("Special assigner with conversion", "[Graph]") {
	OutputPort<int> outputPort;
	InputPort<float> inputPort{ ArithmeticConverter<float>{}, [](float& lhs, float rhs) { lhs += rhs; } };
	inputPort.Link(outputPort);

	outputPort.Set(1);
	outputPort.Set(1);
	outputPort.Set(3);

	REQUIRE(inputPort.Get() == 5.0f);
}


//--------------------------------------
// Create nodes
//--------------------------------------


class TestAddNode : public Node<InputPorts<float, float>,
								OutputPorts<float>> {
	using NodeT = Node<InputPorts<float, float>,
						 OutputPorts<float>>;
public:
	TestAddNode() = default;
	template <class Func>
	TestAddNode(Func portConverter) : NodeT{ portConverter } {}

	void Update() override {
		out << *a + *b;
	}

private:
	InputPort<float>& a = inl::GetInput<0>(*this);
	InputPort<float>& b = inl::GetInput<1>(*this);
	OutputPort<float>& out = inl::GetOutput<0>(*this);
};


class TestNode2 : public Node<InputPorts<float>, OutputPorts<float>> {
	using NodeT =Node<InputPorts<float>, OutputPorts<float>>;
public:
	TestNode2() : NodeT{ InputPort<float>{}, OutputPort<float>{} } {}	
};




TEST_CASE("Get node ports", "[Graph]") {
	TestAddNode node;
	REQUIRE(node.GetInput(0).GetType() == typeid(float));
	REQUIRE(node.GetInput(1).GetType() == typeid(float));
	REQUIRE_THROWS(node.GetInput(2));
	REQUIRE(node.GetOutput(0).GetType() == typeid(float));
	REQUIRE_THROWS(node.GetOutput(1));
}

TEST_CASE("Addition node produces correct output", "[Graph]") {
	TestAddNode node;
	InputPort<float> readout;
	node.GetOutput(0).Link(readout);

	GetInput<0>(node) << 1;
	GetInput<1>(node) << 2;
	node.Update();
	REQUIRE(*readout == 3);
}

TEST_CASE("Addition node uses port converter", "[Graph]") {
	TestAddNode node{ ArithmeticConverter<float>{} };
	InputPort<float> readout;
	node.GetOutput(0).Link(readout);

	GetInput<0>(node).Set(std::any(1));
	GetInput<1>(node).Set(std::any(2));
}
