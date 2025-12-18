// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "FirstPersonCharacterBase.generated.h"

class UEssentialAttributeSet;

UCLASS(Abstract)
class FPGAME_API AFirstPersonCharacterBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability System")
	UAbilitySystemComponent* AbilitySystemComponent;

public:
	AFirstPersonCharacterBase();

	virtual void PossessedBy(AController* NewController) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability System", meta = (AllowPrivateAccess = "true"))
	UEssentialAttributeSet* EssentialAttributeSet;

public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override
	{
		return AbilitySystemComponent;
	}
};
