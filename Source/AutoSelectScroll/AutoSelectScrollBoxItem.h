// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AutoSelectScrollBox.h"
#include "Blueprint/UserWidget.h"
#include "AutoSelectScrollBoxItem.generated.h"

UCLASS()
class AUTOSELECTSCROLL_API UAutoSelectScrollBoxItem_Base: public UUserWidget
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void NativeDestruct() override;
	virtual void SetAutoSelectScrollBox(class UAutoSelectScrollBox* _ScrollBox);
	UFUNCTION(BlueprintImplementableEvent)
	void SetAutoSelectScrollBox_BP(class UAutoSelectScrollBox* _ScrollBox);
	virtual void ScrollTick(SAutoSelectScrollBox* _AutoSelectScrollBox);
	virtual void ScrollTick(float _CenterGab) ;

	TFunction<void()> ReleasesScrollTickDelegate;
};

UCLASS()
class AUTOSELECTSCROLL_API UAutoSelectScrollBoxItem_Test : public UAutoSelectScrollBoxItem_Base
{
	GENERATED_BODY()
public:
	virtual void ScrollTick(SAutoSelectScrollBox* _AutoSelectScrollBox) override;
};

UCLASS()
class AUTOSELECTSCROLL_API UAutoSelectScrollBoxItem_UseAnimation : public UAutoSelectScrollBoxItem_Base
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
	virtual void ScrollTick(float _CenterGab) override;

	UFUNCTION(BlueprintCallable)
	void RequestScrollIntoView();
private:
	UPROPERTY(EditAnywhere)
	FName ScrollAnimationName;
	UPROPERTY()
	TMap<FName, class UWidgetAnimation*> Animations;

	UPROPERTY()
	UUMGSequencePlayer* UMGSequencePlayer = nullptr;
};