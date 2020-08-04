# Files comprising the target
set(src_common
	"SpinMutex.cpp"
	"Timer.cpp"
)

set(src_exception
	"Exception/Exception.cpp"
)

set(src_graph
	"Graph/Node.cpp"
	"Graph/NodeFactory.cpp"
	"Graph/NodeLibrary.cpp"
	"Graph/Port.cpp"
)

set(src_grapheditor
	"GraphEditor/GraphParser.cpp"
)
set(src_jobsystem
	"JobSystem/ConditionVariable.cpp"
	"JobSystem/Fence.cpp"
	"JobSystem/Mutex.cpp"
	"JobSystem/ThreadpoolScheduler.cpp"
)

set(src_logging
	"Logging/Event.cpp"
	"Logging/Logger.cpp"
	"Logging/LogNode.cpp"
	"Logging/LogPipe.cpp"
	"Logging/LogStream.cpp"
)

set(src_memory
	"Memory/RingAllocationEngine.cpp"
	"Memory/SlabAllocatorEngine.cpp"
)

set(src_platform
	"Platform/Win32/Input.cpp"
	"Platform/Win32/System.cpp"
	"Platform/Win32/Window.cpp"
)

# Create target
add_library(InlineLib STATIC
	${src_common}
	${src_exception}
	${src_graph}
	${src_grapheditor}
	${src_jobsystem}
	${src_logging}
	${src_memory}
	${src_platform}
)

# Dependencies
if (TARGET_PLATFORM_WINDOWS) 
	set(LINK_DBGHELP 1)
else()
	set(LINK_DBGHELP 0)
endif()

target_link_libraries(InlineLib
	$<IF:${LINK_DBGHELP},dbghelp,>
)