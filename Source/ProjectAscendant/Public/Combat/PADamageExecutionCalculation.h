// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "PADamageExecutionCalculation.generated.h"

/**
 * FPACombatFormulas
 *
 * Bộ công thức toán học chiến đấu thuần tuý (Pure Math Struct) cho Project Ascendant.
 * Độc lập hoàn toàn với Engine World/Actors để phục vụ cả tính toán trong GAS Execution
 * lẫn việc chạy Unit Test tự động tốc độ cao (Deterministic Tests).
 *
 * Tuân thủ tuyệt đối:
 *  - GDD attributes-system.md
 *  - ADR-0002: GAS Integration Strategy
 *  - Control Manifest: Server-Authoritative execution, không tính toán client proxy, không bỏ qua đường cong giáp.
 */
struct PROJECTASCENDANT_API FPACombatFormulas
{
	/**
	 * AC-1: Công thức tính Sát Thương Thực Tế (Effective Damage Calculation).
	 * DamageTaken = RawDamage * (100 / (100 + Armor))
	 *
	 * Quy tắc I-Frame: Nếu mục tiêu mang thẻ State.Invulnerable, DamageTaken = 0.0f.
	 *
	 * @param RawDamage Lượng sát thương thô đầu vào của đòn đánh.
	 * @param Armor Chỉ số giáp của đối tượng nhận sát thương.
	 * @param bIsInvulnerable Mục tiêu có đang trong trạng thái bất tử (I-Frame Dash) hay không.
	 * @return Sát thương thực tế sau khi giảm trừ giáp.
	 */
	static float CalculateEffectiveDamage(float RawDamage, float Armor, bool bIsInvulnerable, bool bIsInSanctuary = false);

	/**
	 * AC-2: Công thức tính Sát Thương Phá Thế Đứng (Posture Damage Calculation).
	 * PostureDamage = BaseStagger * (1 + StaggerBonus) * HitMultiplier
	 *
	 * Quy tắc đặc biệt:
	 *  - Đòn đánh thường: HitMultiplier = 1.0f
	 *  - Đánh điểm yếu (Weakspot): HitMultiplier = 1.5f
	 *  - Phản đòn hoàn hảo (Perfect Parry): Gây lập tức 35% MaxPosture của mục tiêu.
	 *
	 * @param BaseStagger Lực phá thế cơ sở của đòn đánh.
	 * @param StaggerBonus Phần trăm thưởng phá thế (ví dụ +0.3 khi tẩm dầu Dược sư).
	 * @param HitMultiplier Hệ số trúng đòn cơ sở.
	 * @param bIsWeakspot Có đánh trúng điểm yếu hay không (kích hoạt hệ số 1.5x).
	 * @param bIsPerfectParry Có phải phản đòn hoàn hảo không (kích hoạt 35% MaxPosture).
	 * @param TargetMaxPosture Ngưỡng thế đứng tối đa của mục tiêu (Boss: 1000+, Player: 100).
	 * @return Lực tích tụ Posture truyền vào thanh thế đứng của mục tiêu.
	 */
	static float CalculatePostureDamage(float BaseStagger, float StaggerBonus, float HitMultiplier, bool bIsWeakspot, bool bIsPerfectParry, float TargetMaxPosture);

	/**
	 * AC-3: Công thức Sát Thương Đòn Kết Liễu Vượt Cấp (Stagger Execution Damage).
	 * ExecuteDamage = (TargetMaxHP * 0.25) + (BaseDamage * 3.0)
	 *
	 * Bỏ qua hoàn toàn giáp phòng ngự (Armor Bypass) để đảm bảo hạ gục Boss trong đúng 4 chu kỳ kết liễu chuẩn mực.
	 *
	 * @param TargetMaxHP Lượng máu tối đa của Boss hoặc mục tiêu.
	 * @param BaseDamage Sát thương cơ bản của vũ khí/kỹ năng kết liễu.
	 * @return Sát thương kết liễu dứt điểm.
	 */
	static float CalculateStaggerExecutionDamage(float TargetMaxHP, float BaseDamage);
};

/**
 * UPADamageExecutionCalculation
 *
 * Tính toán sát thương máu thông qua UGameplayEffectExecutionCalculation (GAS).
 * Chạy trên Dedicated Server có toàn quyền xác thực (Server-Authoritative).
 * Xử lý:
 *  - Giảm trừ sát thương phi tuyến tính theo giáp (AC-1)
 *  - Triệt tiêu sát thương khi dính tag State.Invulnerable (AC-1)
 *  - Thực hiện đòn kết liễu Stagger Execution khi mục tiêu mang tag State.Staggered (AC-3)
 */
UCLASS()
class PROJECTASCENDANT_API UPADamageExecutionCalculation : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	UPADamageExecutionCalculation();

	virtual void Execute_Implementation(
		const FGameplayEffectCustomExecutionParameters& ExecutionParams,
		FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};

/**
 * UPAPostureExecutionCalculation
 *
 * Tính toán lực phá vỡ thế đứng (Posture Damage) qua GAS Execution Calculation.
 * Xử lý:
 *  - Tích tụ Posture theo lực đánh và hệ số điểm yếu Weakspot 1.5x (AC-2)
 *  - Phản đòn hoàn hảo Perfect Parry gây 35% MaxPosture tức thì (AC-2)
 *  - Miễn nhiễm Posture khi mục tiêu mang State.Invulnerable
 */
UCLASS()
class PROJECTASCENDANT_API UPAPostureExecutionCalculation : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	UPAPostureExecutionCalculation();

	virtual void Execute_Implementation(
		const FGameplayEffectCustomExecutionParameters& ExecutionParams,
		FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};

// -----------------------------------------------------------------------------
// Alias / Concrete Subclasses theo danh xưng trong GDD và AC
// -----------------------------------------------------------------------------

/**
 * UGEC_DamageCalculation: Tên gọi chuẩn trong AC-1
 */
UCLASS()
class PROJECTASCENDANT_API UGEC_DamageCalculation : public UPADamageExecutionCalculation
{
	GENERATED_BODY()
};

/**
 * UGEC_PostureCalculation: Tên gọi chuẩn trong AC-2
 */
UCLASS()
class PROJECTASCENDANT_API UGEC_PostureCalculation : public UPAPostureExecutionCalculation
{
	GENERATED_BODY()
};
