// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PAThreatComponent.generated.h"

class APABaseCharacter;

/**
 * EPAThreatSource
 *
 * Phân loại nguồn nộ khí (Threat) theo chuẩn GDD và ADR-0001:
 * - Damage: 1 HP Sát thương = 1.0 Threat
 * - PostureDamage: 1 Phá vỡ thế đứng = 2.5 Threat
 * - Healing: 1 Máu hồi phục = 0.5 Threat
 * - Taunt: Hệ số nộ 5.0x + Snap lên Top Threat + 100
 */
UENUM(BlueprintType)
enum class EPAThreatSource : uint8
{
	Damage          UMETA(DisplayName = "Damage (1.0x)"),
	PostureDamage   UMETA(DisplayName = "Posture Damage (2.5x)"),
	Healing         UMETA(DisplayName = "Healing (0.5x)"),
	Taunt           UMETA(DisplayName = "Tank Taunt (5.0x + Instant Snap)")
};

/**
 * FPAThreatEntry
 *
 * Bản ghi nộ khí của một người chơi / thực thể tham gia giao tranh.
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPAThreatEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Threat")
	TWeakObjectPtr<AActor> CombatantActor = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Threat")
	FString CombatantId = TEXT("");

	UPROPERTY(BlueprintReadOnly, Category = "Threat")
	float ThreatScore = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Threat")
	float TimeSinceLastActivity = 0.0f;

	FPAThreatEntry()
		: CombatantActor(nullptr)
		, CombatantId(TEXT(""))
		, ThreatScore(0.0f)
		, TimeSinceLastActivity(0.0f)
	{
	}

	FPAThreatEntry(AActor* InActor, const FString& InId, float InThreat)
		: CombatantActor(InActor)
		, CombatantId(InId)
		, ThreatScore(InThreat)
		, TimeSinceLastActivity(0.0f)
	{
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPAOnAggroTargetChanged, AActor*, NewTarget, AActor*, OldTarget);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPAOnLeashResetStateChanged, bool, bIsResetting);

/**
 * UPAThreatComponent
 *
 * Component quản lý Bảng Nộ Khí (Threat Table) có thẩm quyền 100% từ Dedicated Server (Story 003 / net-003).
 *
 * Các quy tắc bắt buộc:
 * - AC-2: Quy tắc chuyển mục tiêu 130% (Threat Retargeting Rule). Quét mỗi 1.0s, chỉ đổi mục tiêu khi nộ khí vượt >= 130%.
 * - AC-3: Suy giảm nộ khí khi không hành động (Threat Decay). Đứng yên 3.0s bắt đầu suy giảm 10% mỗi giây.
 * - AC-11: Khiêu khích của Tanker (Taunt Multiplier & Snap). Nhân 5.0x nộ khí và snap ngay lập tức lên Max(Top + 100, Caster).
 * - AC-5: Giới hạn kéo quái (Leash Boundary Reset). Quá 2500 cm khỏi vị trí gốc -> Bất tử (Invulnerable), xóa nộ khí, quay về vị trí spawn.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECTASCENDANT_API UPAThreatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPAThreatComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// -------------------------------------------------------------------------
	// Threat Management API
	// -------------------------------------------------------------------------

	/**
	 * Bổ sung nộ khí cho thực thể gây sát thương / hồi máu / khống chế.
	 *
	 * @param InstigatorActor Actor gây nộ khí.
	 * @param RawAmount Lượng giá trị thô (sát thương, hồi máu).
	 * @param Source Nguồn tạo nộ khí (quyết định hệ số nhân theo ADR-0001).
	 */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Threat")
	void AddThreat(AActor* InstigatorActor, float RawAmount, EPAThreatSource Source = EPAThreatSource::Damage);

	/**
	 * Bổ sung nộ khí theo chuỗi định danh (hỗ trợ Unit Test độc lập và Server Id).
	 */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Threat")
	void AddThreatById(const FString& InstigatorId, float RawAmount, EPAThreatSource Source = EPAThreatSource::Damage);

	/**
	 * Kỹ năng Khiêu Khích (Taunt) của Tanker (AC-11).
	 * Nhân 5.0x nộ khí và snap ngay lập tức lên Max(TopThreat + 100, CasterThreat).
	 */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Threat")
	void ApplyTaunt(AActor* TaunterActor, float BaseTauntThreat = 0.0f);

	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Threat")
	void ApplyTauntById(const FString& TaunterId, float BaseTauntThreat = 0.0f);

	/**
	 * Đánh giá lại mục tiêu công kích (AC-2: 130% Threat Retargeting Rule).
	 * Boss quét mỗi 1.0s; chỉ chuyển target khi nộ khí của người khác >= CurrentTargetThreat * 1.30f.
	 */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Threat")
	void EvaluateThreatRetargeting();

	/**
	 * Xử lý suy giảm nộ khí theo thời gian không hành động (AC-3).
	 * Không có hành động trong 3.0s -> suy giảm 10% mỗi giây.
	 */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Threat")
	void TickThreatDecay(float DeltaTime);

	// -------------------------------------------------------------------------
	// Leash & Reset Mechanics (AC-5)
	// -------------------------------------------------------------------------

	/**
	 * Kiểm tra khoảng cách hiện tại so với vị trí Spawn ban đầu.
	 * Nếu cự ly > 2500 cm -> Kích hoạt LeashReset.
	 */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Threat")
	bool CheckLeashDistance(const FVector& CurrentLocation);

	/**
	 * Kích hoạt trạng thái quay về vị trí gốc: Bất tử, xóa toàn bộ nộ khí, tăng tốc chạy về.
	 */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Threat")
	void TriggerLeashReset();

	/**
	 * Kết thúc quá trình hồi vị trí, sẵn sàng chiến đấu trở lại.
	 */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Threat")
	void CompleteLeashReset();

	/**
	 * Kiểm tra xem Boss có đang trong trạng thái bất tử do Leash Reset hay không.
	 */
	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Threat")
	bool IsInvulnerable() const { return bIsLeashResetting; }

	// -------------------------------------------------------------------------
	// Queries & State
	// -------------------------------------------------------------------------

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Threat")
	float GetThreat(AActor* CombatantActor) const;

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Threat")
	float GetThreatById(const FString& CombatantId) const;

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Threat")
	AActor* GetCurrentAggroTarget() const { return CurrentAggroTarget.Get(); }

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Threat")
	FString GetCurrentAggroTargetId() const { return CurrentAggroTargetId; }

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Threat")
	int32 GetTrackedCombatantCount() const { return ThreatTable.Num(); }

	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Threat")
	void ClearThreatTable();

	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Threat")
	void SetSpawnOrigin(const FVector& InSpawnOrigin) { SpawnOrigin = InSpawnOrigin; }

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Threat")
	FVector GetSpawnOrigin() const { return SpawnOrigin; }

public:
	UPROPERTY(BlueprintAssignable, Category = "ProjectAscendant|Threat")
	FPAOnAggroTargetChanged OnAggroTargetChanged;

	UPROPERTY(BlueprintAssignable, Category = "ProjectAscendant|Threat")
	FPAOnLeashResetStateChanged OnLeashResetStateChanged;

protected:
	/** Hệ số nhân ngưỡng đổi mục tiêu (Mặc định 1.30 = 130% theo AC-2) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectAscendant|Threat", meta = (ClampMin = "1.0"))
	float ThreatRetargetRatio = 1.30f;

	/** Thời gian trễ không hành động trước khi nộ khí bắt đầu suy giảm (Mặc định 3.0s theo AC-3) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectAscendant|Threat", meta = (ClampMin = "0.0"))
	float ThreatDecayDelay = 3.0f;

	/** Tốc độ suy giảm nộ khí mỗi giây sau mốc trễ (Mặc định 0.10 = 10%/giây theo AC-3) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectAscendant|Threat", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ThreatDecayRate = 0.10f;

	/** Hệ số nhân sát thương nộ khí khi kích hoạt Taunt (Mặc định 5.0x theo AC-11) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectAscendant|Threat", meta = (ClampMin = "1.0"))
	float TauntMultiplier = 5.0f;

	/** Lượng nộ khí cộng thêm tối thiểu khi Taunt snap vượt Top Threat (Mặc định +100.0 theo AC-11) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectAscendant|Threat", meta = (ClampMin = "1.0"))
	float TauntBonusFlatThreat = 100.0f;

	/** Cự ly tối đa cho phép kéo quái/boss trước khi buộc reset (Mặc định 2500 cm theo AC-5) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectAscendant|Threat", meta = (ClampMin = "500.0"))
	float LeashMaxDistance = 2500.0f;

	/** Chu kỳ quét đánh giá lại mục tiêu nộ khí (Mặc định 1.0s theo AC-2) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectAscendant|Threat", meta = (ClampMin = "0.1"))
	float RetargetEvaluationInterval = 1.0f;

	/** Tọa độ điểm xuất hiện gốc phục vụ kiểm tra Leash Reset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectAscendant|Threat")
	FVector SpawnOrigin = FVector::ZeroVector;

	/** Cờ đánh dấu thực thể đang trong chu trình trở về và bất tử */
	UPROPERTY(ReplicatedUsing = OnRep_IsLeashResetting, BlueprintReadOnly, Category = "ProjectAscendant|Threat")
	bool bIsLeashResetting = false;

	/** Mục tiêu nộ khí cao nhất hiện tại */
	UPROPERTY(ReplicatedUsing = OnRep_AggroTarget, BlueprintReadOnly, Category = "ProjectAscendant|Threat")
	TWeakObjectPtr<AActor> CurrentAggroTarget = nullptr;

	/** Chuỗi định danh mục tiêu hiện tại */
	UPROPERTY(BlueprintReadOnly, Category = "ProjectAscendant|Threat")
	FString CurrentAggroTargetId = TEXT("");

	/** Bảng lưu trữ nộ khí của tất cả các đối thủ đang giao tranh (tối đa 32) */
	TMap<FString, FPAThreatEntry> ThreatTable;

	/** Bộ đếm thời gian chu kỳ đánh giá mục tiêu 1.0s */
	float RetargetEvaluationTimer = 0.0f;

	UFUNCTION()
	virtual void OnRep_IsLeashResetting();

	UFUNCTION()
	virtual void OnRep_AggroTarget();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	static float GetMultiplierForThreatSource(EPAThreatSource Source);
	FString GetKeyForActor(AActor* Actor) const;
	float FindCurrentTopThreat(FString& OutTopId, AActor*& OutTopActor) const;
};
