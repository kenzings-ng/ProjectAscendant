// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Crafting/PABlacksmithTypes.h"
#include "Itemization/PAAffixTypes.h"
#include "Itemization/PASavedItemInstance.h"
#include "PABlacksmithComponent.generated.h"

class UPAInventoryComponent;
class UPACurrencyComponent;
class APlayerController;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FPAOnItemRepaired, int32, SlotIndex, int32, GoldCost, float, NewDurability);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FPAOnItemSalvaged, int32, SlotIndex, FName, ItemId, int32, ShardsGained);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FPAOnItemEnhanced, int32, SlotIndex, int32, NewLevel, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FPAOnSocketUnlocked, int32, SlotIndex, int32, NewSocketCount, int32, MaxSockets);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FPAOnGemSocketed, int32, EquipmentSlotIndex, int32, SocketIndex, FName, GemItemId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FPAOnGemUnsocketed, int32, EquipmentSlotIndex, int32, SocketIndex, FName, GemItemId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPAOnCraftingFailed, EPACraftingError, ErrorCode);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPAOnBossSoulForged, FName, ItemId, EPAItemRarity, RarityTier);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FPAOnBackpackExpanded, int32, OldCapacity, int32, NewCapacity, int32, GoldCost);

/**
 * UPABlacksmithComponent
 *
 * Component quản lý nghiệp vụ Thợ Rèn Phân Vùng (Zone-tiered Blacksmithing System).
 * Story crft-001 (Tier 1 Outpost Forge):
 * - AC-1: Sửa chữa độ bền trang bị về 100%, trừ Vàng theo công thức chuẩn.
 * - AC-2: Phân rã trang bị và Sách Kỹ Năng ra Tàn Trang (Skill Shards); bảo vệ đồ khóa (bIsLocked).
 * - AC-3: Cường hóa an toàn 100% (+0 -> +3) tiêu hao Vàng và Quặng Sắt; chặn vượt mốc Tier 1.
 * - AC-4: Kiểm tra khoảng cách tương tác (<= 300cm), cấm rèn khi đang giao chiến (In-Combat), đảm bảo Server Authority.
 *
 * Story crft-002 (Enhancement Pipeline +4 to +10 & Socketing):
 * - AC-1: Cường hóa +4 đến +6 (Tier 2 Forge, 70%-50% success, failure preserves level).
 * - AC-2: Cường hóa +7 đến +10 (Tier 3 Forge, 40%-15% success, failure drops 1 level unless protected by ward).
 * - AC-3: Đục tối đa 2 Lỗ Khảm Ngọc (Tier 2 Forge, độ hiếm Rare trở lên), khảm ngọc và tháo ngọc hoàn trả vào kho đồ (phí 100 Vàng).
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECTASCENDANT_API UPABlacksmithComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPABlacksmithComponent();

	virtual void BeginPlay() override;

	// -------------------------------------------------------------------------
	// Forge Tier Configuration
	// -------------------------------------------------------------------------

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Crafting")
	EPABlacksmithTier GetForgeTier() const { return ForgeTier; }

	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Crafting")
	void SetForgeTier(EPABlacksmithTier InTier) { ForgeTier = InTier; }

	/** Ghi đè xúc xắc RNG cho kiểm thử đơn vị tự động (-1.0f = dùng ngẫu nhiên FMath::FRand()) */
	void SetTestRollOverride(float InRoll) { TestRollOverride = InRoll; }

	// -------------------------------------------------------------------------
	// Core Blacksmithing Services (Server-Authoritative)
	// -------------------------------------------------------------------------

	/**
	 * AC-1: Sửa chữa độ bền trang bị về 100.0%.
	 */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Crafting")
	bool RepairItem(UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 SlotIndex, EPACraftingError& OutError);

	/**
	 * AC-2: Phân rã trang bị hoặc Sách Kỹ Năng thừa thành Tàn Trang (Skill Shards).
	 */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Crafting")
	bool SalvageItem(UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 SlotIndex, int32& OutShardsGained, EPACraftingError& OutError);

	/**
	 * AC-3 (crft-001) / AC-1 & AC-2 (crft-002): Cường hóa trang bị (+1 đến +10).
	 * Hỗ trợ bUseWard bảo vệ trang bị mốc cao (+7 đến +10) chống tụt cấp.
	 */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Crafting")
	bool EnhanceItem(UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 SlotIndex, bool bUseWard, EPACraftingError& OutError);

	/** Phiên bản quá tải tương thích ngược không dùng đá bảo hộ */
	bool EnhanceItem(UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 SlotIndex, EPACraftingError& OutError)
	{
		return EnhanceItem(Inventory, Wallet, SlotIndex, false, OutError);
	}

	/**
	 * AC-3 (crft-002): Đục thêm 1 lỗ khảm ngọc trên trang bị (tối đa 2 lỗ tại Tier 2 Forge).
	 * Chỉ áp dụng cho trang bị độ hiếm Rare trở lên.
	 */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Crafting")
	bool UnlockSocket(UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 SlotIndex, EPACraftingError& OutError);

	/**
	 * AC-3 (crft-002): Khảm ngọc vào ô socket trống trên trang bị.
	 * Tiêu hao viên ngọc từ kho đồ.
	 */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Crafting")
	bool SocketGem(UPAInventoryComponent* Inventory, int32 EquipmentSlotIndex, int32 SocketIndex, FName GemItemId, EPACraftingError& OutError);

	/**
	 * AC-3 (crft-002): Tháo ngọc đã khảm ra khỏi trang bị.
	 * Chi phí 100 Vàng, hoàn trả viên ngọc vào kho đồ, ô socket trở về rỗng (NAME_None).
	 */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Crafting")
	bool UnsocketGem(UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 EquipmentSlotIndex, int32 SocketIndex, EPACraftingError& OutError);

	/**
	 * AC-1 (crft-003): Rèn đúc trang bị độ hiếm Legendary từ Linh Hồn Boss.
	 * Độc quyền tại Tier 3 Ancient Sanctuary Forge.
	 * Yêu cầu: 1x Linh Hồn Boss, 4x Mảnh vỡ Boss, 5x Quặng Hư Không, 5,000 Vàng.
	 * X11a: trang bị đầu ra do server quyết định qua BossSoulRecipes (theo BossSoulItemId); client không chọn được đầu ra.
	 */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Crafting")
	bool ForgeBossSoulEquipment(
		UPAInventoryComponent* Inventory,
		UPACurrencyComponent* Wallet,
		FName BossSoulItemId,
		FName BossPartItemId,
		FName VoidOreItemId,
		EPACraftingError& OutError);

	/**
	 * AC-3 (crft-003): Mở rộng sức chứa kho đồ theo cấp tuần tự (30 -> 40 -> 50 -> 60).
	 * Kiểm tra điều kiện nguyên liệu, vàng và Bậc Lò Rèn tương ứng.
	 */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Crafting")
	bool ExpandBackpackCapacity(UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, EPACraftingError& OutError);

	/**
	 * AC-4: Kiểm tra điều kiện tương tác (khoảng cách <= 300cm, không ở trạng thái giao chiến).
	 */
	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Crafting")
	bool ValidateInteraction(const AActor* InteractingActor, bool bInCombat, EPACraftingError& OutError) const;

	// -------------------------------------------------------------------------
	// X11a: Server-side request validation (ownership + distance + combat)
	// -------------------------------------------------------------------------

	/**
	 * Player that may have sent a Server RPC on this component: the APlayerController on the owner chain of
	 * this component's actor. nullptr for NPC-owned forges (such RPCs cannot arrive from a client; see X11b).
	 */
	APlayerController* GetRequestingPlayerController() const;

	/**
	 * Server precondition run by every Server RPC before any state mutation:
	 * - authority on this forge, requesting player present and possessing a pawn;
	 * - client-supplied Inventory / Wallet (when non-null) must be owned by the requesting player;
	 * - ValidateInteraction(pawn, State.InCombat on pawn ASC) with kMaxInteractionDistance.
	 */
	bool ValidateServerRequest(const APlayerController* Requester, const UActorComponent* Inventory, const UActorComponent* Wallet, EPACraftingError& OutError) const;

	/** Forge tier of this (server-side) forge converted to the itemization EPAForgeTier scale. */
	EPAForgeTier GetItemizationForgeTier() const;

	// -------------------------------------------------------------------------
	// X11a: Boss Soul recipes (server configuration)
	// -------------------------------------------------------------------------

	void AddBossSoulRecipe(const FPABossSoulRecipe& Recipe) { BossSoulRecipes.Add(Recipe); }

	/** Returns the server recipe for BossSoulItemId, or nullptr. */
	const FPABossSoulRecipe* FindBossSoulRecipe(FName BossSoulItemId) const;

	// -------------------------------------------------------------------------
	// Server RPCs (Client Request -> Server Execution)
	// -------------------------------------------------------------------------

	UFUNCTION(Server, Reliable, WithValidation, Category = "ProjectAscendant|Crafting")
	void Server_RequestRepair(UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 SlotIndex);

	UFUNCTION(Server, Reliable, WithValidation, Category = "ProjectAscendant|Crafting")
	void Server_RequestSalvage(UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 SlotIndex);

	UFUNCTION(Server, Reliable, WithValidation, Category = "ProjectAscendant|Crafting")
	void Server_RequestEnhance(UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 SlotIndex);

	UFUNCTION(Server, Reliable, WithValidation, Category = "ProjectAscendant|Crafting")
	void Server_RequestEnhanceWithWard(UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 SlotIndex, bool bUseWard);

	UFUNCTION(Server, Reliable, WithValidation, Category = "ProjectAscendant|Crafting")
	void Server_RequestUnlockSocket(UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 SlotIndex);

	UFUNCTION(Server, Reliable, WithValidation, Category = "ProjectAscendant|Crafting")
	void Server_RequestSocketGem(UPAInventoryComponent* Inventory, int32 EquipmentSlotIndex, int32 SocketIndex, FName GemItemId);

	UFUNCTION(Server, Reliable, WithValidation, Category = "ProjectAscendant|Crafting")
	void Server_RequestUnsocketGem(UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 EquipmentSlotIndex, int32 SocketIndex);

	UFUNCTION(Server, Reliable, WithValidation, Category = "ProjectAscendant|Crafting")
	void Server_RequestForgeBossSoul(
		UPAInventoryComponent* Inventory,
		UPACurrencyComponent* Wallet,
		FName BossSoulItemId,
		FName BossPartItemId,
		FName VoidOreItemId);

	UFUNCTION(Server, Reliable, WithValidation, Category = "ProjectAscendant|Crafting")
	void Server_RequestExpandBackpack(UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet);

	// -------------------------------------------------------------------------
	// Dual-Currency Server RPCs (Story item-007, EPIC-ITEMIZATION-001)
	// -------------------------------------------------------------------------

	void BindSavedItemInventory(TArray<FPASavedItemInstance>* InItems, UPACurrencyComponent* InWallet);

	/** AC-1: Server RPC sửa chữa vật phẩm bằng ItemUID (X11a: chi phí do server tính) */
	UFUNCTION(Server, Reliable, WithValidation, Category = "ProjectAscendant|Itemization")
	void Server_RepairItem(const FGuid& ItemInstanceUID);

	/** AC-2: Server RPC tẩy dòng Affix bằng ItemUID (X11a: chi phí và bậc lò do server xác định) */
	UFUNCTION(Server, Reliable, WithValidation, Category = "ProjectAscendant|Itemization")
	void Server_ReforgeAffix(const FGuid& ItemInstanceUID, int32 AffixIndex);

	/** AC-3: Server RPC đục lỗ khảm ngọc bằng ItemUID (X11a: chi phí theo ô và bậc lò lấy từ ForgeTier của lò này) */
	UFUNCTION(Server, Reliable, WithValidation, Category = "ProjectAscendant|Itemization")
	void Server_AddSocket(const FGuid& ItemInstanceUID);

public:
	UPROPERTY(BlueprintAssignable, Category = "ProjectAscendant|Crafting")
	FPAOnItemRepaired OnItemRepaired;

	UPROPERTY(BlueprintAssignable, Category = "ProjectAscendant|Crafting")
	FPAOnItemSalvaged OnItemSalvaged;

	UPROPERTY(BlueprintAssignable, Category = "ProjectAscendant|Crafting")
	FPAOnItemEnhanced OnItemEnhanced;

	UPROPERTY(BlueprintAssignable, Category = "ProjectAscendant|Crafting")
	FPAOnSocketUnlocked OnSocketUnlocked;

	UPROPERTY(BlueprintAssignable, Category = "ProjectAscendant|Crafting")
	FPAOnGemSocketed OnGemSocketed;

	UPROPERTY(BlueprintAssignable, Category = "ProjectAscendant|Crafting")
	FPAOnGemUnsocketed OnGemUnsocketed;

	UPROPERTY(BlueprintAssignable, Category = "ProjectAscendant|Crafting")
	FPAOnBossSoulForged OnBossSoulForged;

	UPROPERTY(BlueprintAssignable, Category = "ProjectAscendant|Crafting")
	FPAOnBackpackExpanded OnBackpackExpanded;

	UPROPERTY(BlueprintAssignable, Category = "ProjectAscendant|Crafting")
	FPAOnCraftingFailed OnCraftingFailed;

	static constexpr float kMaxInteractionDistance = 300.0f; // 300cm theo AC-4

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectAscendant|Crafting")
	EPABlacksmithTier ForgeTier = EPABlacksmithTier::Tier1_Outpost;

	/** X11a: Server-side Boss Soul recipes (BossSoulItemId -> Legendary output). Never sent by clients. */
	UPROPERTY(EditAnywhere, Category = "ProjectAscendant|Crafting")
	TArray<FPABossSoulRecipe> BossSoulRecipes;

	/** Biến đè tỷ lệ tung xúc xắc ngẫu nhiên cho TDD Automation Tests */
	float TestRollOverride = -1.0f;

	TArray<FPASavedItemInstance>* BoundSavedItems = nullptr;
	TWeakObjectPtr<UPACurrencyComponent> BoundWallet = nullptr;
};
