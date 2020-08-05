#include <InlineLib/Graph.hpp>

#include <Catch2/catch.hpp>

using namespace inl;


TEST_CASE("Link ports same type", "[Graph]") {
	InputPort<float> inputPort;
	OutputPort<float> outputPort;
	inputPort.Link(outputPort);
	REQUIRE_NOTHROW(inputPort.GetLink());
}


TEST_CASE("Link ports incompatible types", "[Graph]") {
	InputPort<float> inputPort;
	OutputPort<char*> outputPort;
	inputPort.Link(outputPort);
	REQUIRE_THROWS(inputPort.GetLink());
}


TEST_CASE("Link ports incompatible types", "[Graph]") {
	InputPort<float> inputPort;
	OutputPort<int> outputPort;
	inputPort.Link(outputPort);
	REQUIRE_NOTHROW(inputPort.GetLink());
}

