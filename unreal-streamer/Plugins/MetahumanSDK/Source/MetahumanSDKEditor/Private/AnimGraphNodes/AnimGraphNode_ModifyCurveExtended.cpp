#include "AnimGraphNode_ModifyCurveExtended.h"
#include "Textures/SlateIcon.h"
#include "GraphEditorActions.h"
#include "ScopedTransaction.h"
#include "Kismet2/CompilerResultsLog.h"
#include "AnimationGraphSchema.h"
#include "BlueprintActionDatabaseRegistrar.h"
#include "Framework/Commands/UIAction.h"
#include "ToolMenus.h"
#include "Kismet2/BlueprintEditorUtils.h"

#define LOCTEXT_NAMESPACE "ModifyCurveExtended"


UAnimGraphNode_ModifyCurveExtended::UAnimGraphNode_ModifyCurveExtended() 
{
}

FString UAnimGraphNode_ModifyCurveExtended::GetNodeCategory() const
{
	return TEXT("Skeletal Control Nodes");
}

FText UAnimGraphNode_ModifyCurveExtended::GetTooltipText() const
{
	return GetNodeTitle(ENodeTitleType::ListView);
}

FText UAnimGraphNode_ModifyCurveExtended::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("AnimGraphNode_ModifyCurveExtended_Title", "Modify Curve (Extended)");
}

#undef LOCTEXT_NAMESPACE
