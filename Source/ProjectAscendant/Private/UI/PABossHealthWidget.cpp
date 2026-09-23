// Copyright Project Ascendant. All Rights Reserved.

#include "UI/PABossHealthWidget.h"
#include "AbilitySystemComponent.h"
#include "Combat/AscendantAttributeSet.h"

// ===========================================================
// Lifecycle
// ===========================================================

void UPABossHealthWidget::NativeConstruct()
{
	Super::NativeConstruct();
	PreviousPhase = 0;
}

void UPABossHealthWidget::NativeDestruct()
{
	ClearBossTarget();
	Super::NativeDestruct();
}

void UPABossHealthWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Kiểm tra chuyển trạng thái stagger → recovered
	const bool bWasFlashing = Model.IsStaggerFlashing();

	Model.Update(InDeltaTime);

	// Phát sự kiện khi stagger kết thúc
	if (bWasFlashing && !Model.IsStaggerFlashing())
	{
		OnBossStaggerEnded.Broadcast();
	}

	// Cập nhật Execution Reticle position nếu Boss đang staggered
	if (Model.bExecutionReticleVisible && BossActor.IsValid())
	{
		APlayerController* PC = GetOwningPlayer();
		if (PC)
		{
			// Lấy vị trí Socket_Execution từ Boss Actor
			FVector WorldLocation = BossActor->GetActorLocation();

			// Nếu Boss có SkeletalMeshComponent với Socket_Execution, dùng socket position
			USceneComponent* RootComp = BossActor->GetRootComponent();
			if (USkeletalMeshComponent* SkelMesh = BossActor->FindComponentByClass<USkeletalMeshComponent>())
			{
				if (SkelMesh->DoesSocketExist(FName("Socket_Execution")))
				{
					WorldLocation = SkelMesh->GetSocketLocation(FName("Socket_Execution"));
				}
			}

			// World-to-Screen projection
			FVector2D ScreenPos;
			if (PC->ProjectWorldLocationToScreen(WorldLocation, ScreenPos, /*bPlayerViewportRelative=*/true))
			{
				Model.UpdateReticleScreenPosition(ScreenPos);
			}
		}
	}
}

// ===========================================================
// Boss Target Binding
// ===========================================================

void UPABossHealthWidget::SetBossTarget(AActor* InBossActor, const FString& InBossName, int32 InBossLevel, const TArray<FName>& InPartIds)
{
	// Hủy binding cũ nếu có
	ClearBossTarget();

	if (!InBossActor)
	{
		return;
	}

	BossActor = InBossActor;
	Model.SetBossIdentity(InBossName, InBossLevel);
	Model.RegisterParts(InPartIds);

	// Tìm và bind vào AbilitySystemComponent của Boss
	UAbilitySystemComponent* ASC = InBossActor->FindComponentByClass<UAbilitySystemComponent>();
	if (!ASC)
	{
		return;
	}

	BossASC = ASC;

	// Bind GAS attribute change delegates (event-driven, zero tick polling)
	ASC->GetGameplayAttributeValueChangeDelegate(
		UAscendantAttributeSet::GetHealthAttribute()
	).AddUObject(this, &UPABossHealthWidget::HandleBossHealthChanged);

	ASC->GetGameplayAttributeValueChangeDelegate(
		UAscendantAttributeSet::GetMaxHealthAttribute()
	).AddUObject(this, &UPABossHealthWidget::HandleBossMaxHealthChanged);

	ASC->GetGameplayAttributeValueChangeDelegate(
		UAscendantAttributeSet::GetPostureAttribute()
	).AddUObject(this, &UPABossHealthWidget::HandleBossPostureChanged);

	ASC->GetGameplayAttributeValueChangeDelegate(
		UAscendantAttributeSet::GetMaxPostureAttribute()
	).AddUObject(this, &UPABossHealthWidget::HandleBossMaxPostureChanged);

	// Đọc giá trị hiện tại từ AttributeSet
	const UAscendantAttributeSet* AttrSet = ASC->GetSet<UAscendantAttributeSet>();
	if (AttrSet)
	{
		Model.SetHP(AttrSet->GetHealth(), AttrSet->GetMaxHealth());
		Model.SetPosture(AttrSet->GetPosture(), AttrSet->GetMaxPosture());
	}

	PreviousPhase = Model.GetCurrentPhase();

	// Hiện widget
	SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UPABossHealthWidget::ClearBossTarget()
{
	if (BossASC.IsValid())
	{
		UAbilitySystemComponent* ASC = BossASC.Get();

		ASC->GetGameplayAttributeValueChangeDelegate(
			UAscendantAttributeSet::GetHealthAttribute()
		).RemoveAll(this);

		ASC->GetGameplayAttributeValueChangeDelegate(
			UAscendantAttributeSet::GetMaxHealthAttribute()
		).RemoveAll(this);

		ASC->GetGameplayAttributeValueChangeDelegate(
			UAscendantAttributeSet::GetPostureAttribute()
		).RemoveAll(this);

		ASC->GetGameplayAttributeValueChangeDelegate(
			UAscendantAttributeSet::GetMaxPostureAttribute()
		).RemoveAll(this);
	}

	BossASC.Reset();
	BossActor.Reset();

	// Ẩn widget
	SetVisibility(ESlateVisibility::Collapsed);
}

// ===========================================================
// GAS Attribute Change Handlers
// ===========================================================

void UPABossHealthWidget::HandleBossHealthChanged(const FOnAttributeChangeData& ChangeData)
{
	Model.SetHP(ChangeData.NewValue, Model.MaxHP);
	UpdateModel();
}

void UPABossHealthWidget::HandleBossMaxHealthChanged(const FOnAttributeChangeData& ChangeData)
{
	Model.SetHP(Model.CurrentHP, ChangeData.NewValue);
	UpdateModel();
}

void UPABossHealthWidget::HandleBossPostureChanged(const FOnAttributeChangeData& ChangeData)
{
	Model.SetPosture(ChangeData.NewValue, Model.MaxPosture);

	// Kiểm tra Posture đạt 100% → kích hoạt stagger tự động
	if (Model.GetPosturePercent() >= 1.0f && !Model.IsStaggerFlashing())
	{
		HandlePostureBroken();
	}
}

void UPABossHealthWidget::HandleBossMaxPostureChanged(const FOnAttributeChangeData& ChangeData)
{
	Model.SetPosture(Model.CurrentPosture, ChangeData.NewValue);
}

// ===========================================================
// Manual Event Triggers
// ===========================================================

void UPABossHealthWidget::HandlePostureBroken()
{
	Model.TriggerPostureBroken();
	OnBossStaggerStarted.Broadcast();
}

void UPABossHealthWidget::HandlePartBroken(FName PartId)
{
	Model.BreakPart(PartId);
	OnBossPartBroken.Broadcast(PartId);
}

// ===========================================================
// Internal
// ===========================================================

void UPABossHealthWidget::UpdateModel()
{
	const int32 CurrentPhase = Model.GetCurrentPhase();
	if (CurrentPhase != PreviousPhase && PreviousPhase != 0)
	{
		OnBossPhaseChanged.Broadcast(CurrentPhase);
	}
	PreviousPhase = CurrentPhase;
}
