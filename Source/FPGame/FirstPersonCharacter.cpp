// Fill out your copyright notice in the Description page of Project Settings.


#include "FirstPersonCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "KismetAnimationLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"


AFirstPersonCharacter::AFirstPersonCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create FirstPersonMesh
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("First Person Mesh"));
	FirstPersonMesh->SetupAttachment(GetMesh());
	FirstPersonMesh->SetRelativeLocationAndRotation(FVector(0, 0, -88.0f), FRotator(0, -90.0f, 0));
	FirstPersonMesh->SetOnlyOwnerSee(true);
	FirstPersonMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
	FirstPersonMesh->SetCollisionProfileName(FName("NoCollision"));

	// Create FirstPersonCamera
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera"));
	FirstPersonCamera->SetupAttachment(FirstPersonMesh);
	FirstPersonCamera->SetRelativeLocationAndRotation(FVector(0.0f, 0.f, 130.f), FRotator(0, 90.0f, 0));
	FirstPersonCamera->bUsePawnControlRotation = true;
	FirstPersonCamera->bEnableFirstPersonFieldOfView = true;
	FirstPersonCamera->bEnableFirstPersonScale = true;

	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;

	GetCapsuleComponent()->SetCapsuleRadius(42);
	GetCapsuleComponent()->SetHiddenInGame(false);
	GetCapsuleComponent()->SetCapsuleHalfHeight(DefaultCharacterHeight);
	GetCharacterMovement()->CrouchedHalfHeight = DefaultCharacterCrouchedHeight;

	GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed;
	GetCharacterMovement()->MaxWalkSpeedCrouched = DefaultCrouchedWalkSpeed;
	GetCharacterMovement()->BrakingDecelerationWalking = DefaultBrakingDecelerationWalking;
	GetCharacterMovement()->GroundFriction = DefaultGroundFriction;
	GetCharacterMovement()->bCanWalkOffLedgesWhenCrouching = true;
	GetCharacterMovement()->JumpZVelocity = DefaultJumpVelocity;
	GetCharacterMovement()->AirControl = DefaultAirControl;
	GetCharacterMovement()->FallingLateralFriction = DefaultFallingLateralFriction;
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

		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this,
		                                   &AFirstPersonCharacter::HandleInputJumpTriggered);

		// Crouching
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Triggered, this,
		                                   &AFirstPersonCharacter::HandleInputCrouchTriggered);
	}
}

void AFirstPersonCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	GiveDoubleJump(false);

	if (bIsCrouching)
	{
		return;
	}

	UpdateLastLandLocation();
	bHasJumped = false;
	bHasDoubleJumped = false;

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

void AFirstPersonCharacter::HandleInputJumpTriggered(const FInputActionValue& Value)
{
	DoJump();
}

void AFirstPersonCharacter::HandleInputCrouchTriggered(const FInputActionValue& Value)
{
	if ((bIsCrouching && bIsUncrouching) || (bIsSliding || GetCharacterMovement()->IsFalling()))
	{
		return;
	}

	if (bIsCrouched)
	{
		DoUncrouch();
	}
	else
	{
		if (IsCharacterMoving(DefaultVelocityThresholdToSlide))
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
	const FVector TraceEndLocation = TraceStartLocation + GetActorUpVector() * DefaultCharacterHeight;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	FHitResult HitResult;
	const float CapsuleTraceRadius = GetCapsuleComponent()->GetScaledCapsuleRadius();
	const float CapsuleTraceHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const bool bHit = GetWorld()->SweepSingleByChannel(
		HitResult,
		TraceStartLocation,
		TraceEndLocation,
		FQuat::Identity,
		ECC_Visibility,
		FCollisionShape::MakeCapsule(CapsuleTraceRadius, CapsuleTraceHeight),
		QueryParams
	);

	return bHit;
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
			bHasDoubleJumped = true;
			Jump();
		}
	}
	else
	{
		DoUncrouch();

		if (!bHasJumped)
		{
			bHasJumped = true;
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
	bIsCrouching = true;
	bIsUncrouching = false;
	Crouch();
	OnCharacterCrouched();
}

void AFirstPersonCharacter::DoUncrouch()
{
	if (!IsCrouched())
	{
		return;
	}

	if (CheckObstacleAboveHead())
	{
		return;
	}

	bIsUncrouching = true;
	bIsCrouching = false;
	UnCrouch();
	OnCharacterUncrouched();
}

void AFirstPersonCharacter::DoSlide()
{
	if (bIsSliding)
	{
		return;
	}

	bIsSliding = true;
	GetCharacterMovement()->BrakingDecelerationWalking = DefaultBrakingDecelerationWalkingWhileSlide;
	GetCharacterMovement()->GroundFriction = DefaultGroundFrictionWhileSlide;

	FVector LaunchVelocity = GetCharacterMovement()->GetLastInputVector() * DefaultLaunchValueToSlide;
	LaunchVelocity.Z = 0.0f;
	LaunchCharacter(LaunchVelocity, false, false);
	OnCharacterSlid();

	GetWorldTimerManager().SetTimer(SlideTimerHandle, this, &AFirstPersonCharacter::DoStopSlide, DefaultSlideDuration,
	                                false);
}

void AFirstPersonCharacter::DoStopSlide()
{
	if (!bIsSliding)
	{
		return;
	}

	bIsSliding = false;
	GetCharacterMovement()->BrakingDecelerationWalking = DefaultBrakingDecelerationWalking;
	GetCharacterMovement()->GroundFriction = DefaultGroundFriction;

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
	return !bHasJumped || !bHasDoubleJumped;
}

bool AFirstPersonCharacter::HasDoubleJump()
{
	return HasJump() && (!bHasDoubleJumped && bCanDoubleJump);
}

void AFirstPersonCharacter::GiveDoubleJump(const bool CanBe)
{
	bCanDoubleJump = CanBe;
	if (bCanDoubleJump)
	{
		JumpMaxCount = DefaultJumpMaxCount;
	}
	else
	{
		JumpMaxCount = 1;
	}
}

void AFirstPersonCharacter::UpdateLastMovementDirection()
{
	const float DirectionAngle = UKismetAnimationLibrary::CalculateDirection(GetVelocity(), GetActorRotation());

	if (DirectionAngle > -22.5 && DirectionAngle < 22.5)
	{
		LastMovementDirection = EMovementDirection::Forward;
	}
	else if (DirectionAngle > 22.5 && DirectionAngle < 67.5)
	{
		LastMovementDirection = EMovementDirection::ForwardRight;
	}
	else if (DirectionAngle > 67.5 && DirectionAngle < 112.5)
	{
		LastMovementDirection = EMovementDirection::Right;
	}
	else if (DirectionAngle > 112.5 && DirectionAngle < 157.5)
	{
		LastMovementDirection = EMovementDirection::BackwardRight;
	}
	else if (DirectionAngle > 157.5 || DirectionAngle < -157.5)
	{
		LastMovementDirection = EMovementDirection::Backward;
	}
	else if (DirectionAngle > -157.5 && DirectionAngle < -112.5)
	{
		LastMovementDirection = EMovementDirection::BackwardLeft;
	}
	else if (DirectionAngle > -112.5 && DirectionAngle < -67.5)
	{
		LastMovementDirection = EMovementDirection::Left;
	}
	else if (DirectionAngle > -67.5 && DirectionAngle < -22.5)
	{
		LastMovementDirection = EMovementDirection::ForwardLeft;
	}
	else
	{
		LastMovementDirection = EMovementDirection::None;
	}
}

void AFirstPersonCharacter::UpdateLastJumpLocation()
{
	LastJumpLocation = GetActorLocation();
}

void AFirstPersonCharacter::UpdateLastLandLocation()
{
	LastLandLocation = GetActorLocation();
}

void AFirstPersonCharacter::UpdateLastDoubleJumpLocation()
{
	LastDoubleJumpLocation = GetActorLocation();
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

		if (bIsCrouching)
		{
			return;
		}

		if (bHasJumped)
		{
			if (HasDoubleJump())
			{
				UpdateLastDoubleJumpLocation();
			}
		}
		else
		{
			bHasJumped = true;
			GiveDoubleJump(true);
		}
	}
}
