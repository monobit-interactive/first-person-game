// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "FirstPersonCharacter.generated.h"

class UInputAction;
class UInputMappingContext;
class USkeletalMeshComponent;
class UCameraComponent;
struct FInputActionValue;

UCLASS()
class FPGAME_API AFirstPersonCharacter : public ACharacter
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* FirstPersonMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCamera;

protected:
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	UPROPERTY(EditAnywhere, Category="Input|Input Actions")
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, Category="Input|Input Actions")
	UInputAction* LookAction;

public:
	AFirstPersonCharacter();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	// Input Handlers
	void HandleInputMoveTriggered(const FInputActionValue& Value);

	void HandleInputMoveCompleted(const FInputActionValue& Value);

	void HandleInputLookTriggered(const FInputActionValue& Value);

	// Essential Movement Logic
	virtual void DoMove(float Right, float Forward);

	virtual void DoStopMove();

	virtual void DoLook(float Yaw, float Pitch);

public:
	USkeletalMeshComponent* GetFirstPersonMeshComponent() const { return FirstPersonMesh; }

	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCamera; }
};
