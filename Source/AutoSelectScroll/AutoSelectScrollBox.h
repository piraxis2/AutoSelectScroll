// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ScrollBox.h"
#include "AutoSelectScrollBox.generated.h"

/**
 * 
 */
enum EAutoSelectScrollState
{
	stop,
	dragmove = 1 << 0,
	automove = 1 << 1,
	move = dragmove | automove,
};

DECLARE_MULTICAST_DELEGATE_OneParam(FOnAutoSelectedScrollBoxItem, UWidget*)

class SAutoSelectScrollBox : public SScrollBox
{
	friend UAutoSelectScrollBox;
public:
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	bool IsOk() const { return !bTouchPanningCapture && !bIsScrolling && bBeginTick; }
	void SetParent(UScrollBox* _Parent) { Parent = _Parent; };
	UWidget* GetTargetWidget() const { return TargetWidget; }

	void ScrollModify();
	UWidget* FindTargetWidget() const;
	float GetCenterGab(UWidget* _Widget) const;
private:
	void FillEmptyArea() const;
	bool IsCenter(UWidget* _Widget) const;
public:
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnScrollTick, SAutoSelectScrollBox*)
	FOnScrollTick OnScrollTick;
	FOnAutoSelectedScrollBoxItem OnAutoSelectedScrollBoxItem;
	FOnAutoSelectedScrollBoxItem OnAutoUnSelectedScrollBoxItem;

private:
	UScrollBox* Parent = nullptr;
	UWidget* TargetWidget = nullptr;
	UWidget* WidgetToFind = nullptr;
	bool bBeginTick = false;
};


UCLASS()
class UAutoSelectScrollBox : public UScrollBox
{
	GENERATED_BODY()

public:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	UWidget* GetTargetWidget() const ;
	void AutoSelectScrollBoxItem(UWidget* _TargetWidget);
	void AutoUnSelectScrollBoxItem(UWidget* _TargetWidget);
	void ScrollTick(SAutoSelectScrollBox* _ScrollBox);
	virtual void ClearChildren() override;
	UFUNCTION(BlueprintCallable, Category = "AutoSelectScrollBox")
	void RequestScrollInToView(UWidget* _WidgetToFind) const;
	bool IsRequested() const;

	void AddItem(UWidget* _Widget);
	

public:
	FOnAutoSelectedScrollBoxItem OnAutoSelectedScrollBoxItem;
	FOnAutoSelectedScrollBoxItem OnAutoUnSelectedScrollBoxItem;
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnScrollTick, SAutoSelectScrollBox*)
	FOnScrollTick OnScrollTick;
};
