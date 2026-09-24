// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Itemization/PAAffixTypes.h"
#include "Itemization/PASavedItemInstance.h"
#include "Crafting/PABlacksmithTypes.h"
#include "PABlacksmithSubsystem.generated.h"

class UAscendantAttributeSet;
class UPACurrencyComponent;
class UPAServerItemGeneratorSubsystem;

/**
 * UPABlacksmithSubsystem
 *
 * GameInstance Subsystem quản lý thẩm quyền và quy tắc rèn đúc trang bị (Story item-003, EPIC-ITEMIZATION-001, Sprint 6).
 *
 * Quy chuẩn đục lỗ khảm ngọc (Socketing):
 * 1. Outpost Forge (Tier 1): CanAddSocket luôn trả về false, mọi socket giữ nguyên bIsUnlocked = false.
 * 2. Field Forge (Tier 2): Cho phép mở khóa (bIsUnlocked = true) tối đa 2 socket Regular trên đồ Rare/Epic.
 *    Từ chối nếu vượt quá 2 hoặc item không đủ Regular socket slot.
 * 3. Forbidden Forge (Tier 3): Cho phép mở khóa socket Prismatic (độc quyền Legendary) và các socket Regular chưa mở.
 */
UCLASS()
class PROJECTASCENDANT_API UPABlacksmithSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UPABlacksmithSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**
	 * Kiểm tra xem có thể mở khóa lỗ khảm ngọc tiếp theo trên trang bị tại ForgeTier này hay không.
	 *
	 * @param Item Món trang bị cần đục lỗ
	 * @param ForgeTier Bậc lò rèn hiện tại
	 * @param OutSocketIndexToUnlock Trả về chỉ số slot tiếp theo có thể mở khóa (hoặc INDEX_NONE nếu không thể)
	 * @param OutError Trả về mã lỗi chi tiết nếu thất bại
	 * @return true nếu thỏa mãn mọi điều kiện đục lỗ
	 */
	UFUNCTION(BlueprintCallable, Category = "Itemization|Blacksmith")
	bool CanAddSocket(
		const FPASavedItemInstance& Item,
		EPAForgeTier ForgeTier,
		int32& OutSocketIndexToUnlock,
		EPACraftingError& OutError) const;

	/**
	 * Overload C++ kiểm tra nhanh không cần lấy OutSocketIndexToUnlock.
	 */
	bool CanAddSocket(
		const FPASavedItemInstance& Item,
		EPAForgeTier ForgeTier,
		EPACraftingError& OutError) const;

	/**
	 * Kiểm tra xem có thể mở khóa một ô socket cụ thể theo chỉ số SocketIndex tại ForgeTier này hay không.
	 */
	UFUNCTION(BlueprintCallable, Category = "Itemization|Blacksmith")
	bool CanAddSocketAtIndex(
		const FPASavedItemInstance& Item,
		EPAForgeTier ForgeTier,
		int32 SocketIndex,
		EPACraftingError& OutError) const;

	/**
	 * Mở khóa ô socket tiếp theo trên trang bị (chuyển bIsUnlocked = true) nếu CanAddSocket thỏa mãn.
	 */
	UFUNCTION(BlueprintCallable, Category = "Itemization|Blacksmith")
	bool UnlockNextSocket(
		UPARAM(ref) FPASavedItemInstance& Item,
		EPAForgeTier ForgeTier,
		EPACraftingError& OutError);

	/**
	 * Mở khóa ô socket cụ thể theo SocketIndex trên trang bị (chuyển bIsUnlocked = true) nếu CanAddSocketAtIndex thỏa mãn.
	 */
	UFUNCTION(BlueprintCallable, Category = "Itemization|Blacksmith")
	bool UnlockSocketAtIndex(
		UPARAM(ref) FPASavedItemInstance& Item,
		EPAForgeTier ForgeTier,
		int32 SocketIndex,
		EPACraftingError& OutError);

	/** Đếm số ô ngọc Regular đã mở khóa (bIsUnlocked == true) */
	UFUNCTION(BlueprintPure, Category = "Itemization|Blacksmith")
	static int32 GetUnlockedRegularSocketCount(const FPASavedItemInstance& Item);

	/** Đếm số ô ngọc Prismatic đã mở khóa (bIsUnlocked == true) */
	UFUNCTION(BlueprintPure, Category = "Itemization|Blacksmith")
	static int32 GetUnlockedPrismaticSocketCount(const FPASavedItemInstance& Item);

	/** Đếm tổng số ô ngọc đã mở khóa (bIsUnlocked == true) */
	UFUNCTION(BlueprintPure, Category = "Itemization|Blacksmith")
	static int32 GetTotalUnlockedSocketCount(const FPASavedItemInstance& Item);

	/**
	 * Khảm ngọc vào ô socket đã mở khóa (bIsUnlocked == true) trên trang bị (Story item-003 AC-4).
	 * Tùy chọn cộng trực tiếp chỉ số ngọc vào UAscendantAttributeSet nếu TargetAttributeSet != nullptr:
	 * - Ruby: +20.0 MaxPosture
	 * - Sapphire: +25.0 MaxMana
	 * - Topaz: +15.0 MaxStamina
	 * - Prismatic: +10.0 AttackPower
	 */
	UFUNCTION(BlueprintCallable, Category = "Itemization|Blacksmith")
	bool SocketGem(
		UPARAM(ref) FPASavedItemInstance& Item,
		int32 SocketIndex,
		FName GemId,
		UAscendantAttributeSet* TargetAttributeSet,
		EPACraftingError& OutError);

	/**
	 * Khảm ngọc bằng ItemUID tìm trong mảng vật phẩm hành trang.
	 */
	UFUNCTION(BlueprintCallable, Category = "Itemization|Blacksmith")
	bool SocketGemByUID(
		UPARAM(ref) TArray<FPASavedItemInstance>& InventoryItems,
		const FGuid& ItemUID,
		int32 SocketIndex,
		FName GemId,
		UAscendantAttributeSet* TargetAttributeSet,
		EPACraftingError& OutError);

	/**
	 * Tháo ngọc khỏi ô socket trên trang bị và thu hồi chỉ số khỏi AttributeSet.
	 */
	UFUNCTION(BlueprintCallable, Category = "Itemization|Blacksmith")
	bool UnsocketGem(
		UPARAM(ref) FPASavedItemInstance& Item,
		int32 SocketIndex,
		UAscendantAttributeSet* TargetAttributeSet,
		EPACraftingError& OutError);

	/** Lấy thông tin thuộc tính và chỉ số cộng thêm của ngọc */
	UFUNCTION(BlueprintPure, Category = "Itemization|Blacksmith")
	static bool GetGemStatBonus(FName GemId, FName& OutAttributeName, float& OutValue);

	/** Áp dụng hoặc thu hồi chỉ số ngọc lên UAscendantAttributeSet */
	UFUNCTION(BlueprintCallable, Category = "Itemization|Blacksmith")
	static bool ApplyGemBonusToAttributeSet(
		FName GemId,
		UAscendantAttributeSet* AttributeSet,
		bool bApply = true);

	// -------------------------------------------------------------------------
	// Dual-Currency Server Transactions (Story item-007, EPIC-ITEMIZATION-001)
	// -------------------------------------------------------------------------

	/**
	 * AC-1 (item-007): Sửa chữa độ bền trang bị về 100% bằng giao dịch nguyên tử (Server-Authoritative).
	 * Kiểm tra số dư Gold của Wallet. Nếu không đủ: trả về InsufficientGold, giữ nguyên số dư và độ bền.
	 */
	UFUNCTION(BlueprintCallable, Category = "Itemization|Blacksmith")
	bool ServerRepairItem(
		UPARAM(ref) FPASavedItemInstance& Item,
		UPACurrencyComponent* Wallet,
		int32 CostGold,
		EPACraftingError& OutError);

	/**
	 * AC-2 (item-007): Tẩy lại 1 dòng Affix trên trang bị tiêu hao 2,000 Gold + 5 Skill Shards (Ash Shards) (GDD §7.2).
	 * Kiểm tra số dư song tiền tệ nguyên tử. Nếu thiếu bất kỳ loại nào: trả về mã lỗi tương ứng, rollback 100%, affixes giữ nguyên.
	 */
	UFUNCTION(BlueprintCallable, Category = "Itemization|Blacksmith")
	bool ServerReforgeAffix(
		UPARAM(ref) FPASavedItemInstance& Item,
		int32 AffixIndex,
		int32 CostGold,
		int32 CostShards,
		EPAForgeTier ForgeTier,
		UPACurrencyComponent* Wallet,
		UPAServerItemGeneratorSubsystem* ItemGenerator,
		EPACraftingError& OutError);

	/**
	 * AC-3 (item-007): Đục lỗ khảm ngọc trên trang bị theo Bậc Lò Rèn tiêu hao Gold + Shards (GDD §7.2).
	 * Chi phí tự động xác định nếu CostGold/CostShards <= 0:
	 * - Socket 1: 1,000 Gold + 3 Shards
	 * - Socket 2: 3,000 Gold + 8 Shards
	 * - Socket 3 (Prismatic): 15,000 Gold + 20 Shards
	 * Kiểm tra điều kiện lò rèn và song tiền tệ. Nếu không đủ: giữ nguyên trạng thái socket (locked) và số dư.
	 */
	UFUNCTION(BlueprintCallable, Category = "Itemization|Blacksmith")
	bool ServerAddSocket(
		UPARAM(ref) FPASavedItemInstance& Item,
		int32 CostGold,
		int32 CostShards,
		EPAForgeTier ForgeTier,
		UPACurrencyComponent* Wallet,
		EPACraftingError& OutError);

	/** Overloads thao tác tìm kiếm theo ItemUID trong danh sách hành trang */
	bool ServerRepairItemByUID(
		TArray<FPASavedItemInstance>& InventoryItems,
		const FGuid& ItemUID,
		UPACurrencyComponent* Wallet,
		int32 CostGold,
		EPACraftingError& OutError);

	bool ServerReforgeAffixByUID(
		TArray<FPASavedItemInstance>& InventoryItems,
		const FGuid& ItemUID,
		int32 AffixIndex,
		int32 CostGold,
		int32 CostShards,
		EPAForgeTier ForgeTier,
		UPACurrencyComponent* Wallet,
		UPAServerItemGeneratorSubsystem* ItemGenerator,
		EPACraftingError& OutError);

	bool ServerAddSocketByUID(
		TArray<FPASavedItemInstance>& InventoryItems,
		const FGuid& ItemUID,
		int32 CostGold,
		int32 CostShards,
		EPAForgeTier ForgeTier,
		UPACurrencyComponent* Wallet,
		EPACraftingError& OutError);
};
