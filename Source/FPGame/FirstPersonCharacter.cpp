// Fill out your copyright notice in the Description page of Project Settings.


#include "FirstPersonCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "KismetAnimationLibrary.h"
#include "KismetTraceUtils.h"
#include "TraversableObstacleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"


AFirstPersonCharacter::AFirstPersonCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create FirstPersonMesh
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("First Person Mesh"));
	FirstPersonMesh->SetupAttachment(GetMesh());
	FirstPersonMesh->SetRelativeLocationAndRotation(CDD.DefaultMeshRelativeLocation, CDD.DefaultMeshRelativeRotation);
	FirstPersonMesh->SetOnlyOwnerSee(true);
	FirstPersonMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
	FirstPersonMesh->SetCollisionProfileName(FName("NoCollision"));

	// Create FirstPersonCamera
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera"));
	FirstPersonCamera->SetupAttachment(FirstPersonMesh);
	FirstPersonCamera->SetRelativeLocationAndRotation(CDD.DefaultCameraRelativeLocation,
	                                                  CDD.DefaultCameraRelativeRotation);
	FirstPersonCamera->bUsePawnControlRotation = true;
	FirstPersonCamera->bEnableFirstPersonFieldOfView = true;
	FirstPersonCamera->bEnableFirstPersonScale = true;

	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;

	GetCapsuleComponent()->SetCapsuleRadius(42);
	GetCapsuleComponent()->SetHiddenInGame(false);
	GetCapsuleComponent()->SetCapsuleHalfHeight(CDD.DefaultCharacterHeight);
	GetCharacterMovement()->SetCrouchedHalfHeight(CDD.DefaultCharacterCrouchedHeight);

	GetCharacterMovement()->MaxWalkSpeed = CDD.DefaultWalkSpeed;
	GetCharacterMovement()->MaxWalkSpeedCrouched = CDD.DefaultCrouchedWalkSpeed;
	GetCharacterMovement()->BrakingDecelerationWalking = CDD.DefaultBrakingDecelerationWalking;
	GetCharacterMovement()->GroundFriction = CDD.DefaultGroundFriction;
	GetCharacterMovement()->bCanWalkOffLedgesWhenCrouching = true;
	GetCharacterMovement()->JumpZVelocity = CDD.DefaultJumpVelocity;
	GetCharacterMovement()->AirControl = CDD.DefaultAirControl;
	GetCharacterMovement()->FallingLateralFriction = CDD.DefaultFallingLateralFriction;
}

void AFirstPersonCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (const APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<
			UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			for (const UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}
		}
	}

	// Set Timers
	GetWorldTimerManager().SetTimer(MoveTimerHandle, this, &AFirstPersonCharacter::OnCharacterMoveTimerEvent, 0.1f,
	                                true);


	GetWorldTimerManager().SetTimer(FallTimerHandle, this, &AFirstPersonCharacter::OnCharacterFallTimerEvent, 0.1f,
	                                true);
}

void AFirstPersonCharacter::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AFirstPersonCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this,
		                                   &AFirstPersonCharacter::HandleInputMoveTriggered);

		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this,
		                                   &AFirstPersonCharacter::HandleInputMoveCompleted);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this,
		                                   &AFirstPersonCharacter::HandleInputLookTriggered);

		// Traversing
		EnhancedInputComponent->BindAction(TraverseAction, ETriggerEvent::Triggered, this,
		                                   &AFirstPersonCharacter::HandleInputTraverseTriggered);

		// Crouching
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Triggered, this,
		                                   &AFirstPersonCharacter::HandleInputCrouchTriggered);
	}
}

void AFirstPersonCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	GiveDoubleJump(false);

	// TODO I don't remember what this line doing?
	/*if (GetCharacterMovement()->IsCrouching())
	{
		return;
	}*/

	UpdateLastLandLocation();
	CRD.bHasJumped = false;
	CRD.bHasDoubleJumped = false;

	OnCharacterLanded(Hit);
}

bool AFirstPersonCharacter::CanJumpInternal_Implementation() const
{
	// CanJump, doesn't allow jumping while crouching.
	if (const bool Result = Super::CanJumpInternal_Implementation())
	{
		return Result;
	}

	return IsCrouched() && !GetCharacterMovement()->IsFalling();
}

void AFirstPersonCharacter::UnCrouch(bool bClientSimulation)
{
	if (CheckObstacleAboveHead())
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("There is an obstacle above character."));
		return;
	}

	Super::UnCrouch(bClientSimulation);
}

void AFirstPersonCharacter::HandleInputMoveTriggered(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();
	DoMove(MovementVector.X, MovementVector.Y);
}

void AFirstPersonCharacter::HandleInputMoveCompleted(const FInputActionValue& Value)
{
	DoStopMove();
}

void AFirstPersonCharacter::HandleInputLookTriggered(const FInputActionValue& Value)
{
	const FVector2D LookAxis = Value.Get<FVector2D>();
	DoLook(LookAxis.X, LookAxis.Y);
}

void AFirstPersonCharacter::HandleInputTraverseTriggered(const FInputActionValue& Value)
{
	DoTraverse();
}

void AFirstPersonCharacter::HandleInputCrouchTriggered(const FInputActionValue& Value)
{
	if ((CRD.bIsSliding || GetCharacterMovement()->IsFalling()))
	{
		return;
	}

	if (bIsCrouched)
	{
		DoUncrouch();
	}
	else
	{
		if (IsCharacterMoving(CDD.DefaultVelocityThresholdToSlide))
		{
			DoSlide();
		}

		DoCrouch();
	}
}

FVector AFirstPersonCharacter::GetFootLocation() const
{
	const FVector CurrentActorLocation = GetActorLocation();
	const float CurrentCapsuleHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	return FVector(
		CurrentActorLocation.X,
		CurrentActorLocation.Y,
		CurrentActorLocation.Z - CurrentCapsuleHalfHeight
	);
}

FVector AFirstPersonCharacter::GetHeadLocation() const
{
	const FVector CurrentActorLocation = GetActorLocation();
	const float CurrentCapsuleHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	return FVector(
		CurrentActorLocation.X,
		CurrentActorLocation.Y,
		CurrentActorLocation.Z + CurrentCapsuleHalfHeight
	);
}

float AFirstPersonCharacter::GetCharacterVelocityLength() const
{
	return GetVelocity().Length();
}

bool AFirstPersonCharacter::IsCharacterMoving(const float VelocityThreshold) const
{
	return GetCharacterVelocityLength() > VelocityThreshold;
}


bool AFirstPersonCharacter::CheckObstacleAboveHead() const
{
	const FVector TraceStartLocation = GetHeadLocation();
	const FVector TraceEndLocation = TraceStartLocation + GetActorUpVector() * CDD.DefaultCharacterHeight;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	FHitResult HitResult;
	const float CapsuleTraceRadius = GetCapsuleComponent()->GetScaledCapsuleRadius();
	const float CapsuleTraceHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const FCollisionShape CapsuleCollisionShape = FCollisionShape::MakeCapsule(CapsuleTraceRadius, CapsuleTraceHeight);

	const bool bHit = GetWorld()->SweepSingleByChannel(
		HitResult,
		TraceStartLocation,
		TraceEndLocation,
		FQuat::Identity,
		ECC_Visibility,
		CapsuleCollisionShape,
		QueryParams
	);

	return bHit;
}

bool AFirstPersonCharacter::CheckObstacleFrontFootLevel(FHitResult& HitResult) const
{
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	const int BoxSize = CDD.DefaultCharacterCrouchedHeight;
	const FVector BoxHalfExtent = FVector(BoxSize);
	const FVector TraceStartLocation = GetFootLocation() + FVector(0, 0, BoxSize);
	const FVector TraceEndLocation = TraceStartLocation + GetActorForwardVector() * 200;
	const FQuat& TraceRotation = GetActorRotation().Quaternion();

	const bool bHit = GetWorld()->SweepSingleByChannel(
		HitResult,
		TraceStartLocation,
		TraceEndLocation,
		TraceRotation,
		ECC_Visibility,
		FCollisionShape::MakeBox(BoxHalfExtent),
		QueryParams
	);

	DrawDebugBoxTraceSingle(GetWorld(), TraceStartLocation, TraceEndLocation, BoxHalfExtent, TraceRotation.Rotator(),
	                        EDrawDebugTrace::ForDuration, bHit, HitResult, FLinearColor::Red, FLinearColor::Green,
	                        1.5f);

	return bHit;
}

bool AFirstPersonCharacter::CheckObstacleFrontHeadLevel(FHitResult& HitResult) const
{
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	const int BoxSize = CDD.DefaultCharacterCrouchedHeight;
	const FVector BoxHalfExtent = FVector(BoxSize);
	const FVector TraceStartLocation = GetHeadLocation() + FVector(0, 0, BoxSize);
	const FVector TraceEndLocation = TraceStartLocation + GetActorForwardVector() * 200;
	const FQuat& TraceRotation = GetActorRotation().Quaternion();

	const bool bHit = GetWorld()->SweepSingleByChannel(
		HitResult,
		TraceStartLocation,
		TraceEndLocation,
		TraceRotation,
		ECC_Visibility,
		FCollisionShape::MakeBox(BoxHalfExtent),
		QueryParams
	);

	DrawDebugBoxTraceSingle(GetWorld(), TraceStartLocation, TraceEndLocation, BoxHalfExtent, TraceRotation.Rotator(),
	                        EDrawDebugTrace::ForDuration, bHit, HitResult, FLinearColor::Red, FLinearColor::Green,
	                        1.5f);

	return bHit;
}

void AFirstPersonCharacter::ResetCameraRelativeLocationAndRotation() const
{
	GetFirstPersonCameraComponent()->SetRelativeLocationAndRotation(CDD.DefaultCameraRelativeLocation,
	                                                                CDD.DefaultCameraRelativeRotation);
}

void AFirstPersonCharacter::ResetMeshRelativeLocationAndRotation() const
{
	GetFirstPersonMeshComponent()->SetRelativeLocationAndRotation(CDD.DefaultMeshRelativeLocation,
	                                                              CDD.DefaultMeshRelativeRotation);
}

void AFirstPersonCharacter::DoMove(const float Right, const float Forward)
{
	if (GetController())
	{
		AddMovementInput(GetActorRightVector() * Right);
		AddMovementInput(GetActorForwardVector() * Forward);
	}
}

void AFirstPersonCharacter::DoStopMove()
{
}

void AFirstPersonCharacter::DoLook(const float Yaw, const float Pitch)
{
	if (GetController())
	{
		AddControllerYawInput(Yaw * 18.0f);
		AddControllerPitchInput(Pitch * 18.0f);
	}
}

void AFirstPersonCharacter::DoTraverse()
{
	if (bIsCrouched)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow,
		                                 TEXT("Character is crouched, waiting uncrouching"));
		DoUncrouch();

		// if uncrouch succeed, bWantsToCrouch will set to false
		// otherwise, it remains true.
		if (GetCharacterMovement()->bWantsToCrouch)
		{
			GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("Character can't uncrouching"));
			return;
		}
	}

	FHitResult HitResult;
	const bool Hit = CheckObstacleFrontFootLevel(HitResult);
	if (!Hit)
	{
		DoJump();
	}
	else
	{
		DoHurdle();
	}
}

void AFirstPersonCharacter::DoJump()
{
	if (!HasJump())
	{
		return;
	}

	if (GetCharacterMovement()->IsFalling())
	{
		if (HasDoubleJump())
		{
			CRD.bHasDoubleJumped = true;
			Jump();
		}
	}
	else
	{
		if (!CRD.bHasJumped)
		{
			CRD.bHasJumped = true;
			GiveDoubleJump(true);
			Jump();
		}
	}
}

void AFirstPersonCharacter::DoStopJump()
{
	StopJumping();
}

void AFirstPersonCharacter::DoCrouch()
{
	Crouch();
}

void AFirstPersonCharacter::DoUncrouch()
{
	UnCrouch();
}

void AFirstPersonCharacter::DoSlide()
{
	if (CRD.bIsSliding)
	{
		return;
	}

	CRD.bIsSliding = true;
	GetCharacterMovement()->BrakingDecelerationWalking = CDD.DefaultBrakingDecelerationWalkingWhileSlide;
	GetCharacterMovement()->GroundFriction = CDD.DefaultGroundFrictionWhileSlide;

	FVector LaunchVelocity = GetCharacterMovement()->GetLastInputVector() * CDD.DefaultLaunchValueToSlide;
	LaunchVelocity.Z = 0.0f;
	LaunchCharacter(LaunchVelocity, false, false);
	OnCharacterSlid();

	GetWorldTimerManager().SetTimer(SlideTimerHandle, this, &AFirstPersonCharacter::DoStopSlide,
	                                CDD.DefaultSlideDuration,
	                                false);
}


void AFirstPersonCharacter::DoHurdle()
{
	if (CRD.bIsHurdling)
	{
		return;
	}

	switch (CRD.LastMovementDirection)
	{
	case EMovementDirection::Forward:
	case EMovementDirection::ForwardLeft:
	case EMovementDirection::ForwardRight:
		break;
	default:
		return;
	}

	FHitResult FootHitResult;
	if (const bool FootHit = CheckObstacleFrontFootLevel(FootHitResult))
	{
		FHitResult HeadHitResult;
		if (const bool HeadHit = CheckObstacleFrontHeadLevel(HeadHitResult))
		{
			return;
		}

		if (!FootHitResult.GetActor()->Implements<UTraversableObstacleInterface>())
		{
			return;
		}

		const FVector ImpactPoint = FootHitResult.ImpactPoint;

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);

		const FVector TraceStartLocation = GetActorLocation();
		const FVector TraceEndLocation = ImpactPoint + (GetActorForwardVector() * CDD.DefaultHurdleLength);
		const FQuat& TraceRotation = GetActorRotation().Quaternion();
		const float CapsuleRadius = GetCapsuleComponent()->GetUnscaledCapsuleRadius();
		const float CapsuleHalfHeight = GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
		const FCollisionShape TraceCollisionShape = FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHalfHeight);

		const bool bHit = GetWorld()->SweepSingleByChannel(
			FootHitResult,
			TraceStartLocation,
			TraceEndLocation,
			TraceRotation,
			ECC_Visibility,
			TraceCollisionShape,
			QueryParams
		);

		DrawDebugCapsuleTraceSingle(GetWorld(), TraceStartLocation, TraceEndLocation, CapsuleRadius, CapsuleHalfHeight,
		                            EDrawDebugTrace::ForDuration, bHit, FootHitResult, FColor::Red, FColor::Blue, 1);

		GetCharacterMovement()->DisableMovement();

		CRD.bIsHurdling = true;

		const FVector TraceEnd = FootHitResult.TraceEnd;
		CRD.LastHurdleStartPosition = FootHitResult.TraceStart;
		CRD.LastHurdleEndPosition = FVector(TraceEnd.X, TraceEnd.Y, GetActorLocation().Z + 10);
		SetActorLocation(CRD.LastHurdleEndPosition, false, nullptr, ETeleportType::TeleportPhysics);

		OnCharacterHurdle(CRD.LastHurdleStartPosition, CRD.LastHurdleEndPosition);
		GetWorldTimerManager().SetTimer(HurdleTimerHandle, this, &AFirstPersonCharacter::DoStopHurdle,
		                                CDD.DefaultHurdleDuration,
		                                false);
	}
}

void AFirstPersonCharacter::DoStopHurdle()
{
	if (CRD.bIsHurdling)
	{
		CRD.bIsHurdling = false;
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}
}

void AFirstPersonCharacter::DoStopSlide()
{
	if (!CRD.bIsSliding)
	{
		return;
	}

	CRD.bIsSliding = false;
	GetCharacterMovement()->BrakingDecelerationWalking = CDD.DefaultBrakingDecelerationWalking;
	GetCharacterMovement()->GroundFriction = CDD.DefaultGroundFriction;

	if (GetWorldTimerManager().IsTimerActive(SlideTimerHandle))
	{
		GetWorldTimerManager().ClearTimer(SlideTimerHandle);
	}

	if (IsCharacterMoving())
	{
		DoUncrouch();
	}

	OnCharacterStopSlid();
}


bool AFirstPersonCharacter::HasJump()
{
	return !CRD.bHasJumped || !CRD.bHasDoubleJumped;
}

bool AFirstPersonCharacter::HasDoubleJump()
{
	return HasJump() && (!CRD.bHasDoubleJumped && CRD.bCanDoubleJump);
}

void AFirstPersonCharacter::GiveDoubleJump(const bool CanBe)
{
	CRD.bCanDoubleJump = CanBe;
	if (CRD.bCanDoubleJump)
	{
		JumpMaxCount = CDD.DefaultJumpMaxCount;
	}
	else
	{
		JumpMaxCount = 1;
	}
}

void AFirstPersonCharacter::UpdateLastMovementDirection()
{
	if (GetVelocity().IsNearlyZero())
	{
		CRD.LastMovementDirection = EMovementDirection::None;
		return;
	}

	const float DirectionAngle = UKismetAnimationLibrary::CalculateDirection(GetVelocity(), GetActorRotation());

	const int32 DirectionIndex = FMath::FloorToInt((DirectionAngle + 180.f + 22.5f) / 45.f) % 8;

	static constexpr EMovementDirection Directions[] = {
		EMovementDirection::Backward,
		EMovementDirection::BackwardLeft,
		EMovementDirection::Left,
		EMovementDirection::ForwardLeft,
		EMovementDirection::Forward,
		EMovementDirection::ForwardRight,
		EMovementDirection::Right,
		EMovementDirection::BackwardRight
	};

	CRD.LastMovementDirection = Directions[DirectionIndex];
}

void AFirstPersonCharacter::UpdateLastJumpLocation()
{
	CRD.LastJumpLocation = GetActorLocation();
}

void AFirstPersonCharacter::UpdateLastLandLocation()
{
	CRD.LastLandLocation = GetActorLocation();
}

void AFirstPersonCharacter::UpdateLastDoubleJumpLocation()
{
	CRD.LastDoubleJumpLocation = GetActorLocation();
}

void AFirstPersonCharacter::OnCharacterMoveTimerEvent()
{
	if (IsCharacterMoving())
	{
		UpdateLastMovementDirection();
		OnCharacterMoved();
	}
	else
	{
		OnCharacterStopped();
	}
}

void AFirstPersonCharacter::OnCharacterFallTimerEvent()
{
	if (!GetCharacterMovement()->IsFalling())
	{
		UpdateLastJumpLocation();
	}
	else
	{
		OnCharacterFalling();

		if (GetCharacterMovement()->IsCrouching())
		{
			return;
		}

		if (CRD.bHasJumped)
		{
			if (HasDoubleJump())
			{
				UpdateLastDoubleJumpLocation();
			}
		}
		else
		{
			CRD.bHasJumped = true;
			GiveDoubleJump(true);
		}
	}
}
