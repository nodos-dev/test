#include <Nodos/Plugin.hpp>
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
	bool LastAssertionResult = true;
	bool AccumStatusShown = false;
	bool FinalStatusShown = false;
	uint32_t FrameCount = 0;
	bool AssertionResult;

public:

	void OnPathStart() override
	{
		LastAssertionResult = true;
		AccumStatusShown = false;
		FinalStatusShown = false;
		FrameCount = 0;
		AssertionResult = true;
		ClearNodeStatusMessages();
	}

	nosResult ExecuteNode(nosNodeExecuteParams* params) override
	{
		nos::NodeExecuteParams pins(params);
		auto& condition = *pins.GetPinData<bool>(NOS_NAME("Condition"));
		auto& behaviour = *pins.GetPinData<AssertionBehaviour>(NOS_NAME("Behaviour"));
		auto framesToWait = *pins.GetPinData<uint32_t>(NOS_NAME("NumFramesToWait"));
		auto framesToAssertOver = *pins.GetPinData<uint32_t>(NOS_NAME("NumFramesToAssertOver"));
		++FrameCount;
		nosEngine.SetPinValueByName(NodeId, NOS_NAME("FrameCount"), nos::Buffer::From(FrameCount));
		if (FrameCount <= framesToWait)
			return NOS_RESULT_SUCCESS; // Wait for the specified number of frames
		
		// Start accumulating after waiting period
		AssertionResult &= condition; // Accumulate the assertion result over multiple frames
		
		// Continue accumulating until we reach the end of the assertion period
		if (FrameCount < framesToWait + framesToAssertOver)
		{
			// Show status during accumulation
			if (behaviour == AssertionBehaviour::SHOW_ON_NODE_STATUS)
			{
				if (!AccumStatusShown || AssertionResult != LastAssertionResult)
				{
					SetNodeStatusMessage(
						AssertionResult ? "Accumulating assertions (passing)" : "Accumulating assertions (failing)",
						AssertionResult ? fb::NodeStatusMessageType::INFO : fb::NodeStatusMessageType::FAILURE);
					LastAssertionResult = AssertionResult;
					AccumStatusShown = true;
				}
			}
			return NOS_RESULT_SUCCESS; // Continue accumulating
		}
		
		// Assertion period complete, execute final behavior
		switch (behaviour)
		{
		case AssertionBehaviour::EXIT_WITH_STATUS_CODE:
#ifdef _WIN32
			TerminateProcess(GetCurrentProcess(), AssertionResult ? 0 : 1);
#else
			_exit(AssertionResult ? 0 : 1);
#endif
			break;
		case AssertionBehaviour::SHOW_ON_NODE_STATUS:
			// Show final result when it changes OR the first time we reach final state
			if (!FinalStatusShown || AssertionResult != LastAssertionResult)
			{
				SetNodeStatusMessage(
					AssertionResult ? "Assertion passed" : "Assertion failed",
					AssertionResult ? fb::NodeStatusMessageType::INFO : fb::NodeStatusMessageType::FAILURE);
				LastAssertionResult = AssertionResult;
				FinalStatusShown = true;
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
