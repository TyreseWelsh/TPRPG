// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SpellPoint.generated.h"

// This class does not need to be modified.
UINTERFACE(BlueprintType)
class USpellPoint : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class TPRPG_API ISpellPoint
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	// BlueprintNativeEvent means the class using this interface must implement this function also see https://docs.unrealengine.com/4.27/en-US/ProgrammingAndScripting/GameplayArchitecture/Interfaces/
	// BlueprintImplementableEvent means the class using this interface has the option to implement this function and cannot be overriden in c++, only in blueprints
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = InterfaceFunction)
	FString EnablePoint();
};
