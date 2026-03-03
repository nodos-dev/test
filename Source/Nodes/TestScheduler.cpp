#include <Nodos/Plugin.hpp>

namespace nos::test
{

NOS_REGISTER_NAME(FreeRun)
NOS_REGISTER_NAME(DeltaTime)

struct TestScheduler : NodeContext
{
	nosResult OnCreate(nosFbNodePtr node) override
	{
		AddPinValueWatcher(NSN_DeltaTime, [this](nos::Buffer const& newVal, std::optional<nos::Buffer> oldValue) {
			nosEngine.RecompilePath(NodeId);
			});
		AddPinValueWatcher(NSN_FreeRun, [this](nos::Buffer const& newVal, std::optional<nos::Buffer> oldValue) {
			nosEngine.RecompilePath(NodeId);
			});
		return NOS_RESULT_SUCCESS;
	}

	void ScheduleNode()
	{
		nosScheduleNodeParams scheduleParams
		{
			.NodeId = NodeId,
			.AddScheduleCount = 1
		};
		nosEngine.ScheduleNode(&scheduleParams);
	}

	void OnPathStart() override
	{
		ScheduleNode();
	}

	nosResult ExecuteNode(NodeExecuteParams const& params) override
	{
		ScheduleNode();
		return NOS_RESULT_SUCCESS;
	}

	void GetScheduleInfo(nosScheduleInfo* info)
	{
		info->Type = NOS_SCHEDULE_TYPE_ON_DEMAND;
		if (**GetWatchedPinValue<bool>(NSN_FreeRun))
		{
			info->DeltaSeconds = { 0, 0 };
		}
		else
		{
			auto delta = **GetWatchedPinValue<nosVec2u>(NSN_DeltaTime);
			info->DeltaSeconds = delta;
		}
	}
};

nosResult RegisterTestScheduler(nosNodeFunctions* nodeFunctions)
{
	NOS_BIND_NODE_CLASS(NOS_NAME("TestScheduler"), TestScheduler, nodeFunctions);
	return NOS_RESULT_SUCCESS;
}
}