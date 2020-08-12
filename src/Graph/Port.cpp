#include <InlineLib/Graph/Port.hpp>
#include <InlineLib/Graph/Node.hpp>

#include <cassert>
#include <sstream>


namespace inl {


//------------------------------------------------------------------------------
// InputPortBase
//------------------------------------------------------------------------------


InputPortBase::InputPortBase() {
	link = nullptr;
}


InputPortBase::~InputPortBase() {
	Unlink();
}


void InputPortBase::Link(OutputPortBase& source) {
	if (IsLinked()) {
		throw InvalidStateException("Input port is already linked.");
	}

	// just let the source OutputPortBase do the nasty stuff
	// note that OutputPortBase is a friend, and sets this' members correctly
	return source.Link(*this);
}


void InputPortBase::Unlink() {
	if (IsLinked()) {
		// note that OutputPortBase is a friend, and sets this' members correctly
		link->Unlink(*this);
	}
}

bool InputPortBase::IsLinked() const {
	return link != nullptr;
}


OutputPortBase& InputPortBase::GetLink() const {
	if (!link) {
		throw InvalidStateException("Port is not linked.");
	}
	return *link;
}

void InputPortBase::SetLinkState(OutputPortBase* link) {
	this->link = link;
}



//------------------------------------------------------------------------------
// OutputPortBase
//
//------------------------------------------------------------------------------

OutputPortBase::OutputPortBase() {
	// = default
}


OutputPortBase::~OutputPortBase() {
	UnlinkAll();
}


void OutputPortBase::Link(InputPortBase& destination) {
	if (destination.IsLinked()) {
		throw InvalidArgumentException("Input port is already linked.");
	}

	if (destination.IsCompatible(GetType()) || GetType() == typeid(Any)) {
		links.insert(&destination);
		destination.link = this;
	}
	else {
		std::stringstream ss;
		ss << GetType().name() << " -> " << destination.GetType().name();
		throw InvalidArgumentException("Port types are not compatible.", ss.str());
	}
}


void OutputPortBase::Unlink(InputPortBase& other) {
	std::set<InputPortBase*>::iterator it;

	it = links.find(&other);
	if (it != links.end()) {
		links.erase(it);
		other.link = nullptr;
		return;
	}
}

bool OutputPortBase::IsLinked() const {
	return !links.empty();
}

size_t OutputPortBase::GetNumLinks() const {
	return links.size();
}


void OutputPortBase::UnlinkAll() {
	for (auto v : links) {
		v->SetLinkState(nullptr);
	}
	links.clear();
}

OutputPortBase::LinkIterator OutputPortBase::begin() {
	return links.begin();
}
OutputPortBase::LinkIterator OutputPortBase::end() {
	return links.end();
}
OutputPortBase::ConstLinkIterator OutputPortBase::begin() const {
	return links.begin();
}
OutputPortBase::ConstLinkIterator OutputPortBase::end() const {
	return links.end();
}
OutputPortBase::ConstLinkIterator OutputPortBase::cbegin() const {
	return links.cbegin();
}
OutputPortBase::ConstLinkIterator OutputPortBase::cend() const {
	return links.cend();
}





//------------------------------------------------------------------------------
// Any specializations.
//------------------------------------------------------------------------------

void InputPort<std::any>::Set(std::any value) {
	data = std::move(value);
}

std::any& InputPort<std::any>::Get() {
	if (!data) {
		throw InvalidStateException("There is no value set.");
	}
	return data.value();
}

const std::any& InputPort<std::any>::Get() const {
	if (!data) {
		throw InvalidStateException("There is no value set.");
	}
	return data.value();
}

void InputPort<std::any>::Clear() {
	data = {};
}

bool InputPort<std::any>::IsSet() const {
	return bool(data);
}

std::type_index InputPort<std::any>::GetType() const {
	return typeid(std::any);
}

std::string InputPort<std::any>::ToString() const {
	throw NotImplementedException();
}

bool InputPort<std::any>::IsCompatible(std::type_index type) const {
	return type != typeid(void);
}



void OutputPort<std::any>::Set(const std::any& data) {
	for (auto v : links) {
		v->Set(data);
	}
}

void OutputPort<std::any>::Set(std::any&& data) {
	auto first = links.begin();
	auto last = links.end();
	if (first != last) {
		--last;
	}

	for (; first != last; ++first) {
		auto v = *first;
		v->Set(data);
	}
	if (first != links.end()) {
		auto v = *first;
		v->Set(std::move(data));
	}
}




} // namespace inl
