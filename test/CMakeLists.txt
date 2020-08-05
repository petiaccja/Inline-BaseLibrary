# Files comprising the target
set(src_common
	"main.cpp"
)

set(src_tests
	"Test_BitOperations.cpp"
	"Test_Color.cpp"
	"Test_ContiguousVector.cpp"
	"Test_Delegate.cpp"
	"Test_DynamicTuple.cpp"
	"Test_Graph.cpp"
	"Test_EnumFlag.cpp"
	"Test_Event.cpp"
	"Test_JobSystem.cpp"
	"Test_PolymorphicVector.cpp"
	"Test_Range.cpp"
	"Test_Rect.cpp"
	"Test_StringUtil.cpp"
	"Test_TemplateUtil.cpp"
	"Test_Transform.cpp"
	"Test_UniqueIdGenerator.cpp"
)

# Create target
add_executable(InlineTest
	${src_common}
	${src_tests}
)

# Dependencies
target_link_libraries(InlineTest
	InlineLib
)