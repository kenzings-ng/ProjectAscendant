// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "PAPostureSyncComponent.generated.h"

class APABaseCharacter;
class UAbilitySystemComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPAOnPostureStaggered, const FString&, FinisherId, float, StaggerDuration);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPAOnFinisherWindowExpired, const FString&, FinisherId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPAOnStaggerExecutionAccepted, APABaseCharacter*, Executor, float, DamageDealt);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPAOnStaggerRecovered, bool, bWasExecuted);

/**
 * UPAPostureSyncComponent
 *
 * Component quản lý đồng bộ thế đứng chung (Shared Posture) và Cửa sổ ưu tiên kết liễu 1.5s
 * cho Boss và quái vật tinh anh (Story 004 / net-004, ADR-0001, GDD multiplayer-coop.md).
 *
 * Tiêu chí nghiệm thu:
 * - AC-7: Khi Posture về 0, người chơi tung đòn phá vỡ thế đứng cuối cùng nhận độc quyền 1.5s
 *         kích hoạt đòn Trừng Phạt (Stagger Execution).
 * - Sau 1.5s, nếu Finisher chưa kích hoạt, cửa sổ mở tự do cho mọi người chơi gần đó (cự ly <= 250 cm).
 * - Đòn kết liễu gây đúng 25% Max HP của mục tiêu, vinh danh đúng người thực thi (Executor),
 *   và hóa giải trạng thái Stagger.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECTASCENDANT_API UPAPostureSyncComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPAPostureSyncComponent();

	virtual void BeginPlay() override;

	// -------------------------------------------------------------------------
	// Constants
	// -------------------------------------------------------------------------
	static constexpr float kFinisherExclusiveWindow = 1.5f;               // AC-7: 1.5s độc quyền cho Finisher
	static constexpr float kStaggerDuration = 3.0f;                       // 3.0s tổng thời gian choáng vỡ thế
	static constexpr float kStaggerExecutionHPRatio = 0.25f;              // AC-7 & AC-3: 25% Max HP sát thương
	static constexpr float kMaxExecutionInteractionDistance = 250.0f;     // Cự ly tương tác kết liễu tối đa (250 cm)

	// -------------------------------------------------------------------------
	// State Queries
	// -------------------------------------------------------------------------
	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Posture")
	bool IsStaggered() const { return bIsStaggered; }

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Posture")
	FGuid GetFinisherPlayerUID() const { return FinisherPlayerUID; }

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Posture")
	FString GetFinisherPlayerId() const { return FinisherPlayerId; }

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Posture")
	float GetBreakTimestamp() const { return BreakTimestamp; }

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Posture")
	APABaseCharacter* GetFinisherCharacter() const { return FinisherCharacter.Get(); }

	/**
	 * Kiểm tra xem hiện tại có đang trong cửa sổ 1.5s ưu tiên độc quyền hay không.
	 */
	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Posture")
	bool IsInFinisherExclusiveWindow(float CurrentTime = -1.0f) const;

	/**
	 * Kiểm tra xem một người chơi cụ thể có đủ quyền kích hoạt kết liễu hay không.
	 * - Nếu chưa vỡ thế (bIsStaggered == false) -> false.
	 * - Nếu trong 1.5s đầu: Chỉ Finisher mới trả về true.
	 * - Nếu sau 1.5s: Mọi người chơi đều trả về true.
	 */
	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Posture")
	bool CanExecuteByUID(const FGuid& InPlayerUID, float CurrentTime = -1.0f) const;

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Posture")
	bool CanExecuteById(const FString& InPlayerId, float CurrentTime = -1.0f) const;

	/**
	 * Kiểm tra quyền kết liễu dựa trên đối tượng nhân vật và kiểm tra cự ly không gian (<= 250 cm).
	 */
	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Posture")
	bool CanExecute(APABaseCharacter* InCharacter, float CurrentTime = -1.0f) const;

	// -------------------------------------------------------------------------
	// Posture Break & Execution Commands (Server-Authoritative)
	// -------------------------------------------------------------------------

	/**
	 * Ghi nhận sự kiện thanh Posture bị bẻ gãy về 0.
	 */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Posture")
	void NotifyPostureBroken(APABaseCharacter* InFinisher, const FGuid& InFinisherUID, float TargetMaxHP);

	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Posture")
	void NotifyPostureBrokenWithId(const FString& InFinisherId, float TargetMaxHP, float SimulatedTime = -1.0f);

	/**
	 * Thực thi đòn kết liễu trên Dedicated Server.
	 * Vinh danh chính xác người bấm nút kết liễu (InCharacter / InPlayerId) và trừ 25% Max HP.
	 */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Posture")
	bool TryExecuteStagger(APABaseCharacter* InCharacter, float TargetMaxHP, float CurrentTime = -1.0f);

	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Posture")
	bool TryExecuteStaggerById(const FString& InPlayerId, float TargetMaxHP, float CurrentTime = -1.0f);

	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Posture")
	bool TryExecuteStaggerByUID(const FGuid& InPlayerUID, float TargetMaxHP, float CurrentTime = -1.0f);

	/**
	 * Kết thúc trạng thái Choáng vỡ thế và hồi phục lại bình thường.
	 */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Posture")
	void RecoverFromStagger();

	/** Reset toàn bộ trạng thái về ban đầu */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Posture")
	void ResetPostureSync();

public:
	// -------------------------------------------------------------------------
	// Delegates
	// -------------------------------------------------------------------------
	UPROPERTY(BlueprintAssignable, Category = "ProjectAscendant|Posture|Events")
	FPAOnPostureStaggered OnPostureStaggered;

	UPROPERTY(BlueprintAssignable, Category = "ProjectAscendant|Posture|Events")
	FPAOnFinisherWindowExpired OnFinisherWindowExpired;

	UPROPERTY(BlueprintAssignable, Category = "ProjectAscendant|Posture|Events")
	FPAOnStaggerExecutionAccepted OnStaggerExecutionAccepted;

	UPROPERTY(BlueprintAssignable, Category = "ProjectAscendant|Posture|Events")
	FPAOnStaggerRecovered OnStaggerRecovered;

protected:
	/** Cờ đánh dấu mục tiêu đang bị Choáng vỡ thế (Staggered) */
	UPROPERTY(ReplicatedUsing = OnRep_IsStaggered, BlueprintReadOnly, Category = "ProjectAscendant|Posture")
	bool bIsStaggered = false;

	/** Định danh GUID của người chơi tung đòn bẻ gãy thế đứng cuối cùng */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "ProjectAscendant|Posture")
	FGuid FinisherPlayerUID;

	/** Định danh chuỗi hiển thị của Finisher */
	UPROPERTY(ReplicatedUsing = OnRep_FinisherPlayerId, BlueprintReadOnly, Category = "ProjectAscendant|Posture")
	FString FinisherPlayerId = TEXT("");

	/** Mốc thời gian (giây) khi Posture bị bẻ gãy */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "ProjectAscendant|Posture")
	float BreakTimestamp = 0.0f;

	/** Con trỏ tới nhân vật Finisher */
	UPROPERTY()
	TWeakObjectPtr<APABaseCharacter> FinisherCharacter = nullptr;

	/** Lượng máu tối đa của mục tiêu để tính sát thương 25% */
	UPROPERTY(Replicated)
	float CachedTargetMaxHP = 1000.0f;

	/** Bộ đếm thời gian cho cửa sổ độc quyền 1.5s */
	FTimerHandle TimerHandle_FinisherExclusiveWindow;

	/** Bộ đếm thời gian cho toàn bộ chu kỳ Stagger 3.0s */
	FTimerHandle TimerHandle_StaggerDuration;

	UFUNCTION()
	virtual void OnRep_IsStaggered();

	UFUNCTION()
	virtual void OnRep_FinisherPlayerId();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	void HandleFinisherWindowExpired();
	void InternalClearStagger(bool bWasExecuted, APABaseCharacter* ActualExecutor, float ExecutionDamage);
	void ApplyGameplayTagToCharacter(APABaseCharacter* Character, const FName& TagName, bool bAdd);
	FString GetActorIdentifier(AActor* Actor) const;
	float GetEffectiveTime(float SimulatedTime) const;
};
