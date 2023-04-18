#pragma once

#include "CoreMinimal.h"

class FAudioToLipsyncExtension
{
public:
	static void RegisterMenus();
	static void GetExtendedActions(const struct FToolMenuContext& MenuContext);
	static void ExecuteCreateLipsyncAnimation(const struct FToolMenuContext& MenuContext);
};