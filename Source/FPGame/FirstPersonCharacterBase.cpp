// Fill out your copyright notice in the Description page of Project Settings.


#include "FirstPersonCharacterBase.h"

#include "AttributeSets/EssentialAttributeSet.h"


AFirstPersonCharacterBase::AFirstPersonCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;

	EssentialAttributeSet = CreateDefaultSubobject<UEssentialAttributeSet>(TEXT("Essential Attribute Set"));
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>("ASC");
}

void AFirstPersonCharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (GetAbilitySystemComponent())
	{
		GetAbilitySystemComponent()->InitAbilityActorInfo(this, this);
	}
}
