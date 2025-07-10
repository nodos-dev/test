#include <Nodos/PluginHelpers.hpp>
#include "Generated/Test_generated.h"
#ifdef _WIN32
#include <windows.h>
#include <process.h>
#endif

namespace nos::test
{

struct AssertNode : NodeContext
{
	using NodeContext::NodeContext;

private:
	bool LastCondition = true; // Track previous condition state
	bool StatusInitialized = false; // Track if status was ever set
	uint32_t FrameCount = 0;

public:

	void OnPathStart() override
	{
		LastCondition = true;
		StatusInitialized = false;
		FrameCount = 0;
		ClearNodeStatusMessages();
	}

	nosResult ExecuteNode(nosNodeExecuteParams* params) override
	{
		nos::NodeExecuteParams pins(params);
		auto& condition = *pins.GetPinData<bool>(NOS_NAME("Condition"));
		auto& behaviour = *pins.GetPinData<AssertionBehaviour>(NOS_NAME("Behaviour"));
		auto& framesToWait = *pins.GetPinData<uint32_t>(NOS_NAME("NumFramesToWait"));
		++FrameCount;
		nosEngine.SetPinValueByName(NodeId, NOS_NAME("FrameCount"), nos::Buffer::From(FrameCount));
		if (FrameCount < framesToWait)
			return NOS_RESULT_SUCCESS; // Wait for the specified number of frames
		switch (behaviour)
		{
		case AssertionBehaviour::EXIT_WITH_STATUS_CODE:
#ifdef _WIN32
			TerminateProcess(GetCurrentProcess(), condition ? 0 : 1);
#else
			_exit(condition ? 0 : 1);
#endif
			break;
		case AssertionBehaviour::SHOW_ON_NODE_STATUS:
			if (!StatusInitialized || condition != LastCondition)
			{
				SetNodeStatusMessage(
					condition ? "Assertion passed" : "Assertion failed",
					condition ? fb::NodeStatusMessageType::INFO : fb::NodeStatusMessageType::FAILURE);
				LastCondition = condition;
				StatusInitialized = true;
			}
			break;
		}
		return NOS_RESULT_SUCCESS;
	}
};

nosResult RegisterAssertNode(nosNodeFunctions* funcs)
{
	NOS_BIND_NODE_CLASS(NOS_NAME_STATIC("Assert"), AssertNode, funcs);
	return NOS_RESULT_SUCCESS;
}
}
