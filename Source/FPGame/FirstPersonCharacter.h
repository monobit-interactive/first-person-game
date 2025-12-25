// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Engine/DataAsset.h"
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

ENUM_CLASS_FLAGS(EMovementDirection);

USTRUCT(BlueprintType)
struct FCharacterInputData
{
	GENERATED_BODY()
};

USTRUCT(BlueprintType)
struct FCharacterDefaultData
{
	GENERATED_BODY()

	// Mesh Data
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector DefaultMeshRelativeLocation = FVector(0, 0, -88.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRotator DefaultMeshRelativeRotation = FRotator(0, -90.0f, 0);

	// Capsule Data
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float DefaultCharacterHeight = 88.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float DefaultCharacterCrouchedHeight = 44.0f;

	// Camera Data
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector DefaultCameraRelativeLocation = FVector(0.0f, 0.f, 130.f);

	// Movement Data
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRotator DefaultCameraRelativeRotation = FRotator(0, 90.0f, 0);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DefaultGroundFriction = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DefaultBrakingDecelerationWalking = 2048.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DefaultAirControl = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DefaultFallingLateralFriction = 0.05f;

	// Walk Data
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DefaultWalkSpeed = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DefaultCrouchedWalkSpeed = 300.0f;

	// Slide Data
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DefaultLaunchValueToSlide = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DefaultVelocityThresholdToSlide = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DefaultGroundFrictionWhileSlide = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DefaultBrakingDecelerationWalkingWhileSlide = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DefaultSlideDuration = 1.0f;

	// Jump Data
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int DefaultJumpMaxCount = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DefaultJumpVelocity = 475.0f;

	// Hurdle Data
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DefaultHurdleDuration = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DefaultHurdleLength = 100.0f;
};

USTRUCT(BlueprintType)
struct FCharacterRuntimeData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EMovementDirection LastMovementDirection;

	// Jump Data
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector LastJumpLocation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector LastLandLocation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector LastDoubleJumpLocation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bHasJumped;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bHasDoubleJumped;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bCanDoubleJump;

	// Slide Data
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bCanSlide;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIsSliding;

	// Hurdle Data
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bCanHurdle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIsHurdling;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector LastHurdleStartPosition;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector LastHurdleEndPosition;
};

UCLASS()
class FPGAME_API AFirstPersonCharacter : public ACharacter
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* FirstPersonMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCamera;

	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	UPROPERTY(EditAnywhere, Category="Input|Input Actions")
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, Category="Input|Input Actions")
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, Category="Input|Input Actions")
	UInputAction* TraverseAction;

	UPROPERTY(EditAnywhere, Category="Input|Input Actions")
	UInputAction* CrouchAction;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character|Data|Default")
	FCharacterDefaultData CharacterDefaultData;
	FCharacterDefaultData& CDD = CharacterDefaultData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character|Data|Runtime")
	FCharacterRuntimeData CharacterRuntimeData;
	FCharacterRuntimeData& CRD = CharacterRuntimeData;

	FTimerHandle MoveTimerHandle;
	FTimerHandle FallTimerHandle;
	FTimerHandle SlideTimerHandle;
	FTimerHandle CrouchTimerHandle;
	FTimerHandle HurdleTimerHandle;

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
	void HandleInputTraverseTriggered(const FInputActionValue& Value);
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

	UFUNCTION(BlueprintCallable)
	bool CheckObstacleFrontFootLevel(FHitResult& HitResult) const;

	UFUNCTION(BlueprintCallable)
	bool CheckObstacleFrontHeadLevel(FHitResult& HitResult) const;

	UFUNCTION(BlueprintCallable)
	void ResetCameraRelativeLocationAndRotation() const;

	UFUNCTION(BlueprintCallable)
	void ResetMeshRelativeLocationAndRotation() const;

	// Essential Movement Logic
	virtual void DoMove(float Right, float Forward);
	virtual void DoStopMove();
	virtual void DoLook(float Yaw, float Pitch);
	virtual void DoTraverse();
	virtual void DoJump();
	virtual void DoStopJump();
	virtual void DoCrouch();
	virtual void DoUncrouch();
	virtual void DoSlide();
	virtual void DoStopSlide();
	virtual void DoHurdle();
	virtual void DoStopHurdle();

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
	void OnCharacterHurdle(FVector FromLocation, FVector ToLocation);

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
