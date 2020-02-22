#include "LogPipe.hpp"

#include "LogNode.hpp"

#include <iostream>
#include <thread>

namespace inl {


LogPipe::LogPipe(std::shared_ptr<LogNode> node) {
	this->node = node;
}

LogPipe::~LogPipe() {}

void LogPipe::PutEvent(const LogEvent& evt) {
	if (!node) {
		return;
	}

	// Spin until we're allowed to even try to lock.
	// This is to avoid starvation of LogNode.
	while (node->prohibitPipes) {
		std::this_thread::yield();
	}

	// Deny action while Node does its stuff.
	node->mtx.lock_shared();
	// Deny action while other thread uses this pipe.
	pipeLock.lock();

	buffer.push_back({ std::chrono::high_resolution_clock::now(), evt });

	pipeLock.unlock();
	node->mtx.unlock_shared();

	node->NotifyNewEvent();
}

void LogPipe::PutEvent(LogEvent&& evt) {
	if (!node) {
		return;
	}

	// Spin until we're allowed to even try to lock.
	// This is to avoid starvation of LogNode.
	while (node->prohibitPipes) {
		std::this_thread::yield();
	}

	// Deny action while Node does its stuff.
	node->mtx.lock_shared();
	// Deny action while other thread uses this pipe.
	pipeLock.lock();

	buffer.push_back({ std::chrono::high_resolution_clock::now(), std::move(evt) });

	pipeLock.unlock();
	node->mtx.unlock_shared();

	node->NotifyNewEvent();
}


std::shared_ptr<LogNode> LogPipe::GetNode() {
	return node;
}


} // namespace inl
