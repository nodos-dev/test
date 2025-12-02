// Copyright MediaZ Teknoloji A.S. All Rights Reserved.

// Includes
#include <Nodos/Plugin.hpp>

NOS_INIT()

NOS_BEGIN_IMPORT_DEPS()
NOS_END_IMPORT_DEPS()

namespace nos::test
{

enum Nodes : int
{
	AssertNode = 0,
	TestScheduler,
	Count
};

nosResult RegisterAssertNode(nosNodeFunctions*);
nosResult RegisterTestScheduler(nosNodeFunctions*);

nosResult NOSAPI_CALL ExportNodeFunctions(size_t* outSize, nosNodeFunctions** outList)
{
	*outSize = Nodes::Count;
	if (!outList)
		return NOS_RESULT_SUCCESS;

#define GEN_CASE_NODE(name)					\
	case Nodes::name: {					\
		auto ret = Register##name(node);	\
		if (NOS_RESULT_SUCCESS != ret)		\
			return ret;						\
		break;								\
	}

	for (int i = 0; i < Nodes::Count; ++i)
	{
		auto node = outList[i];
		switch ((Nodes)i) {
		default:
			break;
			GEN_CASE_NODE(AssertNode)
			GEN_CASE_NODE(TestScheduler)
		}
	}
	return NOS_RESULT_SUCCESS;
}

extern "C"
{
NOSAPI_ATTR nosResult NOSAPI_CALL nosExportPlugin(nosPluginFunctions* out)
{
	out->ExportNodeFunctions = ExportNodeFunctions;
	return NOS_RESULT_SUCCESS;
}
}
}	
