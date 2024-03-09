// Fill out your copyright notice in the Description page of Project Settings.
#include "AutoSelectScrollBoxItem.h"
#include "AutoSelectScrollBox.h"
#include "Animation/UMGSequencePlayer.h"
#include "Animation/WidgetAnimation.h"
#include "Blueprint/WidgetBlueprintGeneratedClass.h"


void UAutoSelectScrollBoxItem_Base::NativeDestruct()
{
	Super::NativeDestruct();
	if (ReleasesScrollTickDelegate)
		ReleasesScrollTickDelegate();
	ReleasesScrollTickDelegate = nullptr;
}

void UAutoSelectScrollBoxItem_Base::SetAutoSelectScrollBox(UAutoSelectScrollBox* _ScrollBox)
{
	SetAutoSelectScrollBox_BP(_ScrollBox);
	_ScrollBox->OnScrollTick.RemoveAll(this); // prevent multiple binding
	_ScrollBox->OnScrollTick.AddUObject(this, &UAutoSelectScrollBoxItem_Base::ScrollTick);
	ReleasesScrollTickDelegate = [this, _ScrollBox]() { _ScrollBox->OnScrollTick.RemoveAll(this); };
}

void UAutoSelectScrollBoxItem_Base::ScrollTick(SAutoSelectScrollBox* _AutoSelectScrollBox)
{
	ScrollTick(_AutoSelectScrollBox->GetCenterGab(this));
}

void UAutoSelectScrollBoxItem_Base::ScrollTick(float _CenterGab)
{
}

void UAutoSelectScrollBoxItem_Test::ScrollTick(SAutoSelectScrollBox* _AutoSelectScrollBox)
{
	Super::ScrollTick(_AutoSelectScrollBox);
	SetRenderScale(FVector2d::One() - _AutoSelectScrollBox->GetCenterGab(this) * 0.1f);
}

void UAutoSelectScrollBoxItem_UseAnimation::NativeConstruct()
{
	Super::NativeConstruct();
	UWidgetBlueprintGeneratedClass* WidgetBlueprintGeneratedClass = GetWidgetTreeOwningClass();
	for (auto& elem : WidgetBlueprintGeneratedClass->Animations)
	{
		Animations.Add(elem.GetFName(), elem);
	}
	if (const auto ScrollAni = Animations.FindRef(ScrollAnimationName))
		UMGSequencePlayer = PlayAnimation(ScrollAni, 0.0f, 1, EUMGSequencePlayMode::Forward, 0.001f);
}

void UAutoSelectScrollBoxItem_UseAnimation::ScrollTick(float _CenterGab)
{
	Super::ScrollTick(_CenterGab);
	if (Animations.Num() == 0)
		return;
	if (UWidgetAnimation* ScrollAni = Animations.FindRef(ScrollAnimationName))
	{
		const float fAnimEndTime = ScrollAni->GetEndTime();
		const float fAnimCurrentTime = FMath::Clamp(fAnimEndTime * (1.f - _CenterGab), 0.0f, fAnimEndTime * 0.99);
		if (UMGSequencePlayer)
			UMGSequencePlayer->SetCurrentTime(fAnimCurrentTime);

		if (0 < fAnimCurrentTime)
			UMGSequencePlayer->Play(fAnimCurrentTime, 1, EUMGSequencePlayMode::Forward, 0.001f, false);
		else
			UMGSequencePlayer->Pause();
	}
}
