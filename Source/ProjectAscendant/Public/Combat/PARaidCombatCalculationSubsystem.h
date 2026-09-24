// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PARaidCombatCalculationSubsystem.generated.h"

/**
 * FPABleedInstance
 *
 * Thực thể hiệu ứng Chảy Máu (Bleed / GE_Debuff_Bleed) trên Boss theo Mục 2.4 itemization.md:
 * - Stacks: Tối đa 3 stacks/người chơi.
 * - Duration: 3.0 giây (làm mới khi chém tiếp).
 * - GracePeriodRemaining: 2.0 giây bảo hộ miễn trừ bị thay thế khỏi Top-10.
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPABleedInstance
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RaidCombat|Bleed")
	FGuid PlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RaidCombat|Bleed")
	int32 Stacks = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RaidCombat|Bleed")
	float DamagePerSecond = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RaidCombat|Bleed")
	float RemainingDuration = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RaidCombat|Bleed")
	float GracePeriodRemaining = 2.0f;

	FPABleedInstance() = default;

	FPABleedInstance(const FGuid& InPlayerId, float InDps)
		: PlayerId(InPlayerId)
		, Stacks(1)
		, DamagePerSecond(InDps)
		, RemainingDuration(3.0f)
		, GracePeriodRemaining(2.0f)
	{
	}

	float GetCurrentDps() const
	{
		return DamagePerSecond * static_cast<float>(Stacks);
	}
};

/**
 * UPARaidCombatCalculationSubsystem
 *
 * Subsystem quản lý 4 quy tắc mạng Server-Authoritative chống lạm phát chỉ số trong Raid Boss 50 người
 * (Story item-006, EPIC-ITEMIZATION-001, Sprint 6, Section 2.4 itemization.md):
 *
 * 1. RULE 1: MAX RULE (Lấy giá trị cao nhất - không cộng dồn)
 *    - Khóa cứng trần thời gian choáng vỡ thế tối đa ở 3.5 giây (Base 3.0s + Max 0.5s).
 *
 * 2. RULE 2: INSTIGATOR ONLY (Độc quyền người kích hoạt)
 *    - Duy nhất 1 người chơi tương tác tử huyệt thực hiện kết liễu.
 *    - Sát thương kết liễu và hút máu (Leech on Stagger) chỉ tính trên Executor.
 *
 * 3. RULE 3: PERSONAL OUTGOING & BLEED MANAGER (Cá nhân hóa + Trần DoT toàn Raid)
 *    - Tối đa 3 stacks/người chơi, refresh thời lượng về 3.0s.
 *    - Trần toàn cục: Tối đa 10 nguồn Bleed độc lập từ 10 người chơi cao nhất.
 *    - Thời gian bảo hộ Grace Period = 2.0s: Miễn trừ bị thay thế.
 *    - Atomic Damage Ledger: Lưu trữ vĩnh viễn 100% công trạng sát thương xét thưởng nhặt đồ.
 *
 * 4. RULE 4: EVENT DR (Suy giảm theo sự kiện đồng thời)
 *    - Phản đòn Perfect Parry trong cửa sổ 1.0s: Phản Posture 100% -> 50% -> 25%.
 */
UCLASS()
class PROJECTASCENDANT_API UPARaidCombatCalculationSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UPARaidCombatCalculationSubsystem();

	// -------------------------------------------------------------------------
	// Rule 1: Max Rule (Stagger Duration Hard Cap)
	// -------------------------------------------------------------------------

	/**
	 * Tính toán thời gian choáng vỡ thế Posture của Boss:
	 * Lấy giá trị lớn nhất từ CandidateBonus, khóa cứng trần tối đa ở 3.5s (Base 3.0s + Max 0.5s).
	 */
	UFUNCTION(BlueprintPure, Category = "RaidCombat|Rule1_MaxRule")
	float CalculateStaggerDuration(float BaseDuration, float CandidateBonus) const;

	/**
	 * Tính toán thời gian choáng từ một danh sách nhiều người chơi cùng gây Posture Finisher.
	 */
	UFUNCTION(BlueprintPure, Category = "RaidCombat|Rule1_MaxRule")
	float CalculateMaxStaggerFromMultiple(float BaseDuration, const TArray<float>& CandidateBonuses) const;

	// -------------------------------------------------------------------------
	// Rule 2: Instigator Only (Execution & Stagger Leech)
	// -------------------------------------------------------------------------

	/**
	 * Thử đăng ký quyền kết liễu tử huyệt Boss (Socket_Execution).
	 * Chỉ duy nhất 1 người chơi đầu tiên được chấp thuận, người chơi khác bị từ chối.
	 */
	UFUNCTION(BlueprintCallable, Category = "RaidCombat|Rule2_InstigatorOnly")
	bool TryBeginExecution(const FGuid& PlayerId, float CurrentTime, float Duration = 1.2f);

	/**
	 * Kiểm tra xem Boss hiện có đang trong trạng thái bị kết liễu hay không.
	 */
	UFUNCTION(BlueprintPure, Category = "RaidCombat|Rule2_InstigatorOnly")
	bool IsExecutionActive(float CurrentTime) const;

	/**
	 * Lấy ID của người chơi hiện đang là Executor duy nhất.
	 */
	UFUNCTION(BlueprintPure, Category = "RaidCombat|Rule2_InstigatorOnly")
	FGuid GetActiveExecutorId() const { return ActiveExecutorId; }

	/**
	 * Tính toán sát thương kết liễu: Chỉ áp dụng thuộc tính của Executor hợp lệ.
	 * Nếu PlayerId không phải Executor, trả về 0.0f.
	 */
	UFUNCTION(BlueprintPure, Category = "RaidCombat|Rule2_InstigatorOnly")
	float CalculateExecutionDamage(float BossMaxHealth, float ExecutorExecutionDmgPct, const FGuid& PlayerId) const;

	/**
	 * Tính toán lượng máu phục hồi khi bẻ khớp (Leech on Stagger): Chỉ áp dụng cho Executor.
	 */
	UFUNCTION(BlueprintPure, Category = "RaidCombat|Rule2_InstigatorOnly")
	float CalculateStaggerLeech(float DamageDealt, float ExecutorLeechPct, const FGuid& PlayerId) const;

	/**
	 * Kết thúc đòn kết liễu.
	 */
	UFUNCTION(BlueprintCallable, Category = "RaidCombat|Rule2_InstigatorOnly")
	void EndExecution();

	// -------------------------------------------------------------------------
	// Rule 3: Personal Outgoing & Bleed Manager
	// -------------------------------------------------------------------------

	/**
	 * Áp dụng hiệu ứng Bleed từ một người chơi lên Boss:
	 * - Đã có nguồn: Tăng stack (max 3), refresh duration về 3.0s.
	 * - Chưa có nguồn: Thêm mới nếu < 10 nguồn (Grace Period = 2.0s).
	 * - Nếu đã đủ 10 nguồn: Chỉ thay thế nguồn có Grace Period hết hạn (<= 0s) và DPS thấp hơn.
	 */
	UFUNCTION(BlueprintCallable, Category = "RaidCombat|Rule3_BleedManager")
	bool ApplyBleed(const FGuid& PlayerId, float BleedDps, float CurrentTime);

	/**
	 * Cập nhật thời gian và tính sát thương Bleed mỗi frame:
	 * Cộng dồn vĩnh viễn vào Atomic Damage Ledger của từng người chơi.
	 */
	UFUNCTION(BlueprintCallable, Category = "RaidCombat|Rule3_BleedManager")
	void TickBleed(float DeltaTime, float& OutTotalDamageDealt);

	/**
	 * Lấy tổng sát thương Bleed đã ghi nhận vĩnh viễn trong Atomic Damage Ledger cho người chơi.
	 */
	UFUNCTION(BlueprintPure, Category = "RaidCombat|Rule3_BleedManager")
	float GetPlayerRecordedDamage(const FGuid& PlayerId) const;

	/**
	 * Lấy số lượng nguồn Bleed đang hoạt động trên Boss (tối đa 10).
	 */
	UFUNCTION(BlueprintPure, Category = "RaidCombat|Rule3_BleedManager")
	int32 GetActiveBleedSourceCount() const { return ActiveBleeds.Num(); }

	/**
	 * Lấy tổng DPS Bleed toàn Raid (khóa trần trong dải 600 - 750 DPS).
	 */
	UFUNCTION(BlueprintPure, Category = "RaidCombat|Rule3_BleedManager")
	float GetTotalBleedDPS() const;

	/**
	 * Xóa toàn bộ trạng thái Bleed (khi reset Boss).
	 */
	UFUNCTION(BlueprintCallable, Category = "RaidCombat|Rule3_BleedManager")
	void ResetBleedState();

	// -------------------------------------------------------------------------
	// Rule 4: Event DR (Diminishing Returns for Simultaneous Parries)
	// -------------------------------------------------------------------------

	/**
	 * Tính toán lực phản Posture từ cú Perfect Parry:
	 * Trong cùng cửa sổ 1.0 giây, tỷ lệ phản suy giảm: 100% -> 50% -> 25% cho các đòn tiếp theo.
	 */
	UFUNCTION(BlueprintCallable, Category = "RaidCombat|Rule4_EventDR")
	float CalculateParryPostureReflect(const FGuid& PlayerId, float BasePostureReflect, float CurrentTime);

	/**
	 * Reset cửa sổ Event DR.
	 */
	UFUNCTION(BlueprintCallable, Category = "RaidCombat|Rule4_EventDR")
	void ResetParryWindow();

public:
	// Hằng số quy chuẩn từ itemization.md
	static constexpr float HARD_CAP_STAGGER_DURATION = 3.5f;
	static constexpr float BASE_STAGGER_DURATION = 3.0f;
	static constexpr int32 MAX_ACTIVE_BLEED_SOURCES = 10;
	static constexpr int32 MAX_BLEED_STACKS_PER_PLAYER = 3;
	static constexpr float BLEED_REFRESH_DURATION = 3.0f;
	static constexpr float BLEED_GRACE_PERIOD = 2.0f;
	static constexpr float PARRY_WINDOW_DURATION = 1.0f;
	static constexpr float GLOBAL_BLEED_MAX_DPS = 750.0f;

private:
	// Trạng thái Rule 2
	FGuid ActiveExecutorId;
	float ExecutionEndTime = -1.0f;

	// Trạng thái Rule 3
	UPROPERTY(Transient)
	TArray<FPABleedInstance> ActiveBleeds;

	TMap<FGuid, float> AtomicDamageLedger;

	// Trạng thái Rule 4
	int32 ParryCountInCurrentWindow = 0;
	float CurrentParryWindowStartTime = -1.0f;
};
