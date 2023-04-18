#pragma once

#include "CoreMinimal.h"

class FTextToSpeechExtension
{
public:
	static void RegisterMenus();
	static void GetExtendedActions(const struct FToolMenuContext& MenuContext);
	static bool CanExecuteTextToSpeech(const struct FToolMenuContext& MenuContext);
	static void ExecuteTextToSpeech(const struct FToolMenuContext& MenuContext);
};