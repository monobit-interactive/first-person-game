// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "EssentialAttributeSet.generated.h"

UCLASS()
class FPGAME_API UEssentialAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UEssentialAttributeSet();

	// Health
	UPROPERTY(BlueprintReadOnly, Category="Attributes")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS_BASIC(UEssentialAttributeSet, Health);

	UPROPERTY(BlueprintReadOnly, Category="Attributes")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS_BASIC(UEssentialAttributeSet, MaxHealth);


	// Stamina
	UPROPERTY(BlueprintReadOnly, Category="Attributes")
	FGameplayAttributeData Stamina;
	ATTRIBUTE_ACCESSORS_BASIC(UEssentialAttributeSet, Stamina);

	UPROPERTY(BlueprintReadOnly, Category="Attributes")
	FGameplayAttributeData MaxStamina;
	ATTRIBUTE_ACCESSORS_BASIC(UEssentialAttributeSet, MaxStamina);
};
