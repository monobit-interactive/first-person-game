// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TraversableObstacleComponent.generated.h"

class USplineComponent;

UENUM(BlueprintType)
enum class ETraversalActionType : uint8
{
	None = 0,
	// Traverse over a thin object and end on the ground at a similar level (Low fence)
	Hurdle = 1,
	// Traverse over a thin object and end in a falling state (Tall fence, or elevated obstacle with no floor on the other side)
	Vault = 2,
	// Traverse up and onto an object without passing over it
	Mantle = 3
};

UINTERFACE(Blueprintable)
class UTraversableObstacleInterface : public UInterface
{
	GENERATED_BODY()
};

class FPGAME_API ITraversableObstacleInterface
{
	GENERATED_BODY()
};


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FPGAME_API UTraversableObstacleComponent : public UActorComponent, public ITraversableObstacleInterface
{
	GENERATED_BODY()

public:
	UTraversableObstacleComponent();
};
