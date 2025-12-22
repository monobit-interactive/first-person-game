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

UENUM(BlueprintType)
enum class EMovementDirection : uint8
{
	None,
	Forward,
	Backward,
	Right,
	Left,
	ForwardRight,
	ForwardLeft,
	BackwardRight,
	BackwardLeft
};

UCLASS()
class FPGAME_API AFirstPersonCharacter : public ACharacter
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* FirstPersonMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCamera;

protected:
	// Input
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	UPROPERTY(EditAnywhere, Category="Input|Input Actions")
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, Category="Input|Input Actions")
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, Category="Input|Input Actions")
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, Category="Input|Input Actions")
	UInputAction* CrouchAction;

	// Capsule Data
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character|Capsule|Data|Default")
	float DefaultCharacterHeight = 88.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character|Capsule|Data|Default")
	float DefaultCharacterCrouchedHeight = 44.0f;

	// Movement Data

	// Default Movement Data
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Character|Movement|Data|Default")
	float DefaultWalkSpeed = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Character|Movement|Data|Default")
	float DefaultCrouchedWalkSpeed = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Character|Movement|Data|Default")
	float DefaultLaunchValueToSlide = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Character|Movement|Data|Default")
	float DefaultVelocityThresholdToSlide = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Character|Movement|Data|Default")
	float DefaultGroundFriction = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Character|Movement|Data|Default")
	float DefaultBrakingDecelerationWalking = 2048.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Character|Movement|Data|Default")
	float DefaultGroundFrictionWhileSlide = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Character|Movement|Data|Default")
	float DefaultBrakingDecelerationWalkingWhileSlide = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character|Movement|Data|Default")
	int DefaultJumpMaxCount = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Character|Movement|Data|Default")
	float DefaultJumpVelocity = 475.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Character|Movement|Data|Default")
	float DefaultAirControl = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Character|Movement|Data|Default")
	float DefaultFallingLateralFriction = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Character|Movement|Data|Default")
	float DefaultSlideDuration = 1.0f;

	// Runtime Movement Data

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character|Movement|Data|Runtime")
	EMovementDirection LastMovementDirection;

	// Jump Data
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character|Movement|Data|Runtime|Jump")
	FVector LastJumpLocation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character|Movement|Data|Runtime|Jump")
	FVector LastLandLocation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character|Movement|Data|Runtime|Jump")
	FVector LastDoubleJumpLocation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character|Movement|Data|Runtime|Jump")
	bool bHasJumped;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character|Movement|Data|Runtime|Jump")
	bool bHasDoubleJumped;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character|Movement|Data|Runtime|Jump")
	bool bCanDoubleJump;

	// Slide Data
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character|Movement|Data|Runtime|Slide")
	bool bCanSlide;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character|Movement|Data|Runtime|Slide")
	bool bIsSliding;

	// Timer Handlers
	FTimerHandle MoveTimerHandle;
	FTimerHandle FallTimerHandle;
	FTimerHandle SlideTimerHandle;
	FTimerHandle CrouchTimerHandle;

public:
	AFirstPersonCharacter();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	// Overrides Members
	virtual void Landed(const FHitResult& Hit) override;
	virtual bool CanJumpInternal_Implementation() const override;
	virtual void OnJumped_Implementation() override { OnCharacterJumped(); }
	virtual void UnCrouch(bool bClientSimulation = false) override;

	// Input Handlers
	void HandleInputMoveTriggered(const FInputActionValue& Value);
	void HandleInputMoveCompleted(const FInputActionValue& Value);
	void HandleInputLookTriggered(const FInputActionValue& Value);
	void HandleInputJumpTriggered(const FInputActionValue& Value);
	void HandleInputCrouchTriggered(const FInputActionValue& Value);

	// Character Helper Functions
	UFUNCTION(BlueprintCallable)
	FVector GetHeadLocation() const;

	UFUNCTION(BlueprintCallable)
	FVector GetFootLocation() const;

	UFUNCTION(BlueprintCallable)
	float GetCharacterVelocityLength() const;

	UFUNCTION(BlueprintCallable)
	bool IsCharacterMoving(float VelocityThreshold = 0.0f) const;

	UFUNCTION(BlueprintCallable)
	bool CheckObstacleAboveHead() const;

	// Essential Movement Logic
	virtual void DoMove(float Right, float Forward);
	virtual void DoStopMove();
	virtual void DoLook(float Yaw, float Pitch);
	virtual void DoJump();
	virtual void DoStopJump();
	virtual void DoCrouch();
	virtual void DoUncrouch();
	virtual void DoSlide();

	UFUNCTION(BlueprintCallable)
	virtual void DoStopSlide();

	// Movement Helper Functions
	virtual bool HasJump();
	virtual bool HasDoubleJump();
	virtual void GiveDoubleJump(bool CanBe);

	// Movement Update Functions
	virtual void UpdateLastMovementDirection();
	virtual void UpdateLastJumpLocation();
	virtual void UpdateLastLandLocation();
	virtual void UpdateLastDoubleJumpLocation();

	// Timer Handler
	virtual void OnCharacterMoveTimerEvent();
	virtual void OnCharacterFallTimerEvent();

	// Blueprint Movement Events
	UFUNCTION(BlueprintImplementableEvent)
	void OnCharacterMoved();

	UFUNCTION(BlueprintImplementableEvent)
	void OnCharacterStopped();

	UFUNCTION(BlueprintImplementableEvent)
	void OnCharacterJumped();

	UFUNCTION(BlueprintImplementableEvent)
	void OnCharacterLanded(const FHitResult& Hit);

	UFUNCTION(BlueprintImplementableEvent)
	void OnCharacterCrouched();

	UFUNCTION(BlueprintImplementableEvent)
	void OnCharacterUncrouched();

	UFUNCTION(BlueprintImplementableEvent)
	void OnCharacterSlid();

	UFUNCTION(BlueprintImplementableEvent)
	void OnCharacterStopSlid();

	UFUNCTION(BlueprintImplementableEvent)
	void OnCharacterFalling();

	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override
	{
		Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
		OnCharacterCrouched();
	}

	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override
	{
		Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
		OnCharacterUncrouched();
	}

public:
	USkeletalMeshComponent* GetFirstPersonMeshComponent() const { return FirstPersonMesh; }
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCamera; }
};
