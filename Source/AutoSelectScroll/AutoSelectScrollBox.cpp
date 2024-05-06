// Fill out your copyright notice in the Description page of Project Settings.


#include "AutoSelectScrollBox.h"

#include "AutoSelectScrollBoxItem.h"
#include "Components/ScrollBoxSlot.h"

void SAutoSelectScrollBox::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SScrollBox::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
	OnScrollTick.Broadcast(this);
	const FSlateRect CanvasRect = CachedGeometry.GetRenderBoundingRect();
	if (!CanvasRect.IsEmpty() && !bBeginTick)
	{
		bBeginTick = true;
		FillEmptyArea();
	}
	ScrollModify();
}

void SAutoSelectScrollBox::ScrollModify()
{
	if (!IsOk())
		return;
		
	if (WidgetToFind)
	{
		ScrollDescendantIntoView(WidgetToFind->GetCachedWidget(), true, EDescendantScrollDestination::Center);
		WidgetToFind = nullptr;
		return;
	}


	if (!IsCenter(TargetWidget))
	{
		const auto FindWidget = FindTargetWidget();
		if (TargetWidget != FindWidget)
		{
			OnAutoUnSelectedScrollBoxItem.Broadcast(TargetWidget);
			TargetWidget = FindWidget;
			OnAutoSelectedScrollBoxItem.Broadcast(TargetWidget);
		}
		if (TargetWidget)
			ScrollDescendantIntoView(TargetWidget->GetCachedWidget(), true, EDescendantScrollDestination::Center);
	}
}

UWidget* SAutoSelectScrollBox::FindTargetWidget() const
{
	const TArray<UWidget*>& WidgetList = Parent->GetAllChildren();
	int32 idx = 0;
	TOptional<float> WidgetSize;
	TOptional<float> ScrollOffset;
	const bool bIsVertical = Orientation == Orient_Vertical;
	UWidget* Widget = nullptr;
	do
	{
		if (!WidgetList.IsValidIndex(idx))
			break;
		UWidget* elem = WidgetList[idx];
		
		TSet<TSharedRef<SWidget>> WidgetsToFind;
		{
			WidgetsToFind.Add(elem->TakeWidget());
		}
		TMap<TSharedRef<SWidget>, FArrangedWidget> Result;
		FindChildGeometries(CachedGeometry, WidgetsToFind, Result);
		if (const FArrangedWidget* WidgetGeometry = Result.Find(elem->TakeWidget()))
		{
			auto LocalSize = WidgetGeometry->Geometry.GetLocalSize();
			auto FindWidgetVector = CachedGeometry.AbsoluteToLocal(WidgetGeometry->Geometry.GetAbsolutePosition()) + (LocalSize / 2);
			auto MyVector = CachedGeometry.GetLocalSize() * FVector2D(0.5f, 0.5f);
			const float WidgetPosition = bIsVertical ? FindWidgetVector.Y : FindWidgetVector.X;
			const float MyPosition = bIsVertical ? MyVector.Y : MyVector.X;
			if (!ScrollOffset.IsSet() || FMath::Abs(ScrollOffset.GetValue()) > FMath::Abs(WidgetPosition - MyPosition))
			{
				Widget = elem;
				ScrollOffset = WidgetPosition - MyPosition;
			}
			if (!WidgetSize.IsSet())
			{
				WidgetSize = bIsVertical ? LocalSize.Y : LocalSize.X;
			}
		}
		idx++;
	}
	while (idx < WidgetList.Num() && WidgetSize.IsSet() ? FMath::Abs(ScrollOffset.IsSet() ? ScrollOffset.GetValue() : 0) >= WidgetSize.GetValue() / 2 : true);
	return Widget;
}

float SAutoSelectScrollBox::GetCenterGab(UWidget* _Widget) const
{
	if (_Widget == nullptr)
		return 0.f;
	if (_Widget->IsDesignTime())
		return 0.f;

	const bool bIsVertical = Orientation == Orient_Vertical;
	const FGeometry WidgetGeometry = FindChildGeometry(CachedGeometry, _Widget->TakeWidget());
	const auto LocalSize = WidgetGeometry.GetLocalSize();
	const auto FindWidgetVector = CachedGeometry.AbsoluteToLocal(WidgetGeometry.GetAbsolutePosition()) + (LocalSize / 2);
	const auto MyVector = CachedGeometry.GetLocalSize() * FVector2D(0.5f, 0.5f);
	const float WidgetPosition = bIsVertical ? FindWidgetVector.Y : FindWidgetVector.X;
	const float MyPosition = bIsVertical ? MyVector.Y : MyVector.X;
	return FMath::Abs<float>(WidgetPosition - MyPosition) / (bIsVertical ? LocalSize.Y / 2 : LocalSize.X / 2);
}

void SAutoSelectScrollBox::FillEmptyArea() const
{
	const FVector2D EmptySize2D = CachedGeometry.GetLocalSize() * FVector2D(0.5f, 0.5f);
	for (int32 i = 0; i < 2; i++)
	{
		auto NewSlotArguments = SScrollBox::Slot();
		SScrollBox::FSlot& NewSlot = *NewSlotArguments.GetSlot();
		const TSharedRef<SSpacer> Spacer = SNew(SSpacer).Size(EmptySize2D);
		NewSlot
		[
			Spacer
		];
		ScrollPanel->Children.InsertSlot(MoveTemp(NewSlotArguments), i == 0 ? 0 : ScrollPanel->Children.Num());
	}
}

bool SAutoSelectScrollBox::IsCenter(UWidget* _Widget) const
{
	if (_Widget == nullptr)
		return false;
	return GetCenterGab(_Widget) <= KINDA_SMALL_NUMBER;
}

TSharedRef<SWidget> UAutoSelectScrollBox::RebuildWidget()
{
	MyScrollBox = SNew(SAutoSelectScrollBox);

	for (UPanelSlot* PanelSlot : Slots)
	{
		if (UScrollBoxSlot* TypedSlot = Cast<UScrollBoxSlot>(PanelSlot))
		{
			TypedSlot->Parent = this;
			TypedSlot->BuildSlot(MyScrollBox.ToSharedRef());
		}
	}
	SAutoSelectScrollBox* ScrollBox = static_cast<SAutoSelectScrollBox*>(MyScrollBox.Get());
	ScrollBox->SetParent(this);
	ScrollBox->OnAutoSelectedScrollBoxItem.AddUObject(this, &UAutoSelectScrollBox::AutoSelectScrollBoxItem);
	ScrollBox->OnAutoUnSelectedScrollBoxItem.AddUObject(this, &UAutoSelectScrollBox::AutoUnSelectScrollBoxItem);
	ScrollBox->OnScrollTick.AddUObject(this, &UAutoSelectScrollBox::ScrollTick);
	for (auto& elem : GetAllChildren())
	{
		if (UAutoSelectScrollBoxItem_Base* Item = Cast<UAutoSelectScrollBoxItem_Base>(elem))
		{
			Item->SetAutoSelectScrollBox(this);
		}
	}
	return MyScrollBox.ToSharedRef();
}

UWidget* UAutoSelectScrollBox::GetTargetWidget() const
{
	if (const SAutoSelectScrollBox* ScrollBox = static_cast<SAutoSelectScrollBox*>(MyScrollBox.Get()))
		return ScrollBox->GetTargetWidget();
	return nullptr;
}

void UAutoSelectScrollBox::AutoSelectScrollBoxItem(UWidget* _TargetWidget)
{
	OnAutoSelectedScrollBoxItem.Broadcast(_TargetWidget);
#if WITH_EDITOR
	UE_LOG(LogTemp, Log, TEXT("%s"), *_TargetWidget->GetName());
#endif
}

void UAutoSelectScrollBox::AutoUnSelectScrollBoxItem(UWidget* _TargetWidget)
{
	OnAutoUnSelectedScrollBoxItem.Broadcast(_TargetWidget);
}

void UAutoSelectScrollBox::ScrollTick(SAutoSelectScrollBox* _ScrollBox)
{
	OnScrollTick.Broadcast(_ScrollBox);
}

void UAutoSelectScrollBox::ClearChildren()
{
	Super::ClearChildren();
	if (SAutoSelectScrollBox* ScrollBox = static_cast<SAutoSelectScrollBox*>(MyScrollBox.Get()))
		ScrollBox->bBeginTick = false;
}

void UAutoSelectScrollBox::RequestScrollInToView(UWidget* _WidgetToFind) const
{
	if (SAutoSelectScrollBox* ScrollBox = static_cast<SAutoSelectScrollBox*>(MyScrollBox.Get()))
	{
		if (ScrollBox->IsOk())
		{
			ScrollBox->ScrollDescendantIntoView(_WidgetToFind->GetCachedWidget(), true, EDescendantScrollDestination::Center);
			return;
		}
		ScrollBox->WidgetToFind = _WidgetToFind;
	}
}

bool UAutoSelectScrollBox::IsRequested() const
{
	if (SAutoSelectScrollBox* ScrollBox = static_cast<SAutoSelectScrollBox*>(MyScrollBox.Get()))
		return ScrollBox->WidgetToFind != nullptr;
	return false;
}

void UAutoSelectScrollBox::AddItem(UWidget* _Widget)
{
	if (UAutoSelectScrollBoxItem_Base* Item = Cast<UAutoSelectScrollBoxItem_Base>(_Widget))
	{
		Item->SetAutoSelectScrollBox(this);
	}
	else
		UE_LOG(LogTemp, Warning, TEXT("UAutoSelectScrollBox::AddItem : %s is not UAutoSelectScrollBoxItem_Base"), *_Widget->GetName()); 

	AddChild(_Widget);
}
