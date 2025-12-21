// Fill out your copyright notice in the Description page of Project Settings.


#include "FirstPersonCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"


AFirstPersonCharacter::AFirstPersonCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create FirstPersonMesh
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("First Person Mesh"));
	FirstPersonMesh->SetupAttachment(GetMesh());
	FirstPersonMesh->SetRelativeLocationAndRotation(FVector(0, 0, -88.0f), FRotator(0, 0, -90.0f));
	FirstPersonMesh->SetOnlyOwnerSee(true);
	FirstPersonMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
	FirstPersonMesh->SetCollisionProfileName(FName("NoCollision"));

	// Create FirstPersonCamera
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera"));
	FirstPersonCamera->SetupAttachment(FirstPersonMesh);
	FirstPersonCamera->SetRelativeLocationAndRotation(FVector(0.0f, 0.f, 130.f), FRotator(0, 0, 90.0f));
	FirstPersonCamera->bUsePawnControlRotation = true;
	FirstPersonCamera->bEnableFirstPersonFieldOfView = true;
	FirstPersonCamera->bEnableFirstPersonScale = true;

	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;

	GetCapsuleComponent()->SetCapsuleRadius(42);
	GetCapsuleComponent()->SetHiddenInGame(false);
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
}

void AFirstPersonCharacter::Tick(float DeltaTime)
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
	}
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
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}
