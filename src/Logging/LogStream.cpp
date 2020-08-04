#include <InlineLib/Logging/LogStream.hpp>

#include <InlineLib/Logging/LogNode.hpp>
#include <InlineLib/Logging/LogPipe.hpp>

#include <intrin.h>
#include <iostream>


namespace inl {


LogStream::LogStream(std::shared_ptr<LogPipe> pipe) {
	this->pipe = pipe;
}

LogStream::LogStream() {
	pipe = nullptr;
}

LogStream::LogStream(LogStream&& rhs) {
	pipe = rhs.pipe;
	rhs.pipe.reset();
}

LogStream::~LogStream() {
	if (pipe) {
		pipe->GetNode()->Flush();
	}
}

LogStream& LogStream::operator=(LogStream&& rhs) {
	if (pipe) {
		pipe->GetNode()->Flush();
	}

	pipe = rhs.pipe;
	rhs.pipe.reset();

	return *this;
}


void LogStream::Event(const inl::LogEvent& e, eEventDisplayMode displayMode) {
	//uint64_t start = __rdtsc();
	if (pipe) {
		pipe->PutEvent(e);
	}
	//uint64_t end = __rdtsc();
	//std::cout << "Event(const &): " << (end - start) << " cycles\n";
}

void LogStream::Event(inl::LogEvent&& e, eEventDisplayMode displayMode) {
	//uint64_t start = __rdtsc();
	if (pipe) {
		pipe->PutEvent(std::move(e));
	}
	//uint64_t end = __rdtsc();
	//std::cout << "Event(&&): " << (end - start) << " cycles\n";
}



} // namespace inl
