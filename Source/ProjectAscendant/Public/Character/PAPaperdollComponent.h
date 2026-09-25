// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Character/PAPaperdollTypes.h"
#include "Inventory/PAInventoryTypes.h"
#include "PAPaperdollComponent.generated.h"

class APABaseCharacter;
class UPAEquipmentComponent;
class UPaperFlipbookComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FPAOnPaperdollVisualChanged, EPAPaperdollLayer, Layer, FName, ItemId, FName, VisualAssetId, bool, bIsVisible);

/**
 * UPAPaperdollComponent
 *
 * Quản lý trực quan hóa trang bị đa tầng (Modular Paperdoll Visualizer) cho nhân vật:
 * - Khởi đầu với trang phục vải thô tân thủ (Starter Linen Cloth).
 * - Lắng nghe sự kiện OnItemEquipped / OnItemUnequipped từ UPAEquipmentComponent.
 * - Khi người chơi click mặc đồ (Áo giáp sắt, Đồ da thợ săn, Áo choàng pháp sư, Vũ khí, Khiên),
 *   lập tức cập nhật tầng hiển thị Sprite tương ứng (EPAPaperdollLayer).
 * - Khi tháo đồ, tự động khôi phục về đồ vải ban đầu.
 * - Đồng bộ khung hình hoạt ảnh (Frame Synchronization) giữa các tầng flipbook với BaseSprite.
 * - Tương thích 100% headless testing thông qua FPAPaperdollModel.
 */
UCLASS(ClassGroup = (Character), meta = (BlueprintSpawnableComponent))
class PROJECTASCENDANT_API UPAPaperdollComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPAPaperdollComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/**
	 * Khởi tạo liên kết với UPAEquipmentComponent của chủ thể (Owner Actor).
	 */
	UFUNCTION(BlueprintCallable, Category = "Paperdoll")
	void BindToEquipmentComponent(UPAEquipmentComponent* InEquipmentComponent);

	/**
	 * Mặc một món trang bị lên Paperdoll và cập nhật trực quan.
	 */
	UFUNCTION(BlueprintCallable, Category = "Paperdoll")
	bool EquipVisual(EPAPaperdollLayer Layer, FName ItemId, FName VisualAssetId);

	/**
	 * Tháo một món trang bị khỏi Paperdoll và đưa về trang phục mặc định.
	 */
	UFUNCTION(BlueprintCallable, Category = "Paperdoll")
	bool UnequipVisual(EPAPaperdollLayer Layer);

	/**
	 * Trả nhân vật về trang phục quần áo vải thô tân thủ.
	 */
	UFUNCTION(BlueprintCallable, Category = "Paperdoll")
	void ResetToStarterCloth();

	/**
	 * Kiểm tra xem nhân vật hiện tại có đang mặc bộ đồ vải tân thủ hay không.
	 */
	UFUNCTION(BlueprintPure, Category = "Paperdoll")
	bool IsWearingStarterCloth() const { return Model.IsWearingStarterCloth(); }

	/**
	 * Trả về ngoại hình trang phục tổng hợp hiện tại.
	 */
	UFUNCTION(BlueprintPure, Category = "Paperdoll")
	const FPAPaperdollAppearance& GetAppearance() const { return Model.CurrentAppearance; }

	/**
	 * Trả về Model dữ liệu thuần (Pure Data Model) phục vụ unit test headless.
	 */
	const FPAPaperdollModel& GetModel() const { return Model; }
	FPAPaperdollModel& GetMutableModel() { return Model; }

	/**
	 * Mặc trang bị vào một trong 9 ô slot của Paperdoll (Story item-004).
	 */
	UFUNCTION(BlueprintCallable, Category = "Paperdoll")
	bool EquipSlot(EPAPaperdollSlot Slot, FName ItemId, FName VisualAssetId);

	/**
	 * Tháo trang bị khỏi một trong 9 ô slot của Paperdoll (Story item-004).
	 */
	UFUNCTION(BlueprintCallable, Category = "Paperdoll")
	bool UnequipSlot(EPAPaperdollSlot Slot);

	/**
	 * Đăng ký / Ánh xạ một Flipbook Component cho một trong 9 ô slot (item-004).
	 */
	UFUNCTION(BlueprintCallable, Category = "Paperdoll")
	void RegisterSlotFlipbookComponent(EPAPaperdollSlot Slot, UPaperFlipbookComponent* FlipbookComp);

	/**
	 * Lấy Flipbook Component được gán cho slot chỉ định.
	 */
	UFUNCTION(BlueprintPure, Category = "Paperdoll")
	UPaperFlipbookComponent* GetSlotFlipbookComponent(EPAPaperdollSlot Slot) const;

	/**
	 * Cập nhật Directional Sort Key / Translucent Sort Priority theo hướng ngắm 8 chiều:
	 * Tự động đảo priority giữa MainHand và OffHand khi mirror hướng Tây/Tây Nam/Tây Bắc.
	 */
	UFUNCTION(BlueprintCallable, Category = "Paperdoll")
	void UpdateDirectionalSortKeys(EPAAimDirection8Way Direction);

	/**
	 * Khởi tạo tự động 9 sub-components cho 9 slot (gắn HandSocket_R/L, pivot chân 64,114).
	 */
	UFUNCTION(BlueprintCallable, Category = "Paperdoll")
	void Initialize9SlotSubcomponents();

	/**
	 * Lấy hướng ngắm hiện tại của Paperdoll.
	 */
	UFUNCTION(BlueprintPure, Category = "Paperdoll")
	EPAAimDirection8Way GetCurrentOrientation() const { return CurrentOrientation; }

	/**
	 * Gán Static Material Instance theo Bậc Hiếm (Common -> Legendary) cho WeaponFlipbookComponent (Story item-005).
	 * Áp dụng cơ chế Zero-Cost Material Swap với 5 preset MI tĩnh.
	 */
	UFUNCTION(BlueprintCallable, Category = "Paperdoll|Material")
	bool SetWeaponMaterialForRarity(FGameplayTag RarityTag);

	/**
	 * Overload gán theo EPAItemRarity.
	 */
	UFUNCTION(BlueprintCallable, Category = "Paperdoll|Material")
	bool SetWeaponMaterialForRarityEnum(EPAItemRarity Rarity);

	/**
	 * Lấy thông tin preset của bậc hiếm chỉ định.
	 */
	UFUNCTION(BlueprintPure, Category = "Paperdoll|Material")
	static FPARarityMaterialPreset GetRarityMaterialPreset(EPAItemRarity Rarity);

	/**
	 * Kiểm tra xem cảnh có thỏa mãn ngưỡng Draw Call đã calibrate thực nghiệm (446-462 calls) hay không (AC-2).
	 */
	UFUNCTION(BlueprintPure, Category = "Paperdoll|Material")
	static bool IsDrawCallWithinCalibratedBudget(int32 MeasuredSceneDrawCalls);

	/**
	 * Lấy Material hiện tại gán trên vũ khí chính.
	 */
	UFUNCTION(BlueprintPure, Category = "Paperdoll|Material")
	UMaterialInterface* GetWeaponMaterial() const { return CurrentWeaponMaterial; }

	/**
	 * Gán trực tiếp MaterialInterface vào vũ khí chính.
	 */
	UFUNCTION(BlueprintCallable, Category = "Paperdoll|Material")
	void SetWeaponMaterialDirect(UMaterialInterface* InMaterial);

	/**
	 * Lấy mã tài nguyên hiển thị của tầng chỉ định.
	 */
	UFUNCTION(BlueprintPure, Category = "Paperdoll")
	FName GetActiveVisualAssetId(EPAPaperdollLayer Layer) const { return Model.GetActiveVisualAssetId(Layer); }

	// -------------------------------------------------------------------------
	// Decoupled Master Rig & Weapon Family API (Story visual-001, Sprint 7)
	// -------------------------------------------------------------------------

	/**
	 * Thiết lập Master Rig cho Lower Body locomotion (HeavyTank, Agility, Caster, Monk).
	 */
	UFUNCTION(BlueprintCallable, Category = "Paperdoll|Rig")
	void SetMasterRig(EPAMasterRig InRig);

	UFUNCTION(BlueprintPure, Category = "Paperdoll|Rig")
	EPAMasterRig GetMasterRig() const { return Model.GetMasterRig(); }

	/**
	 * Thiết lập Weapon Family cho Upper Body combat animation (Decoupled, không ảnh hưởng chân).
	 */
	UFUNCTION(BlueprintCallable, Category = "Paperdoll|Rig")
	void SetUpperBodyWeaponFamily(EPAWeaponFamily InFamily);

	UFUNCTION(BlueprintCallable, Category = "Paperdoll|Rig")
	bool SetUpperBodyWeaponFamilyByTag(FGameplayTag WeaponTag);

	UFUNCTION(BlueprintPure, Category = "Paperdoll|Rig")
	EPAWeaponFamily GetWeaponFamily() const { return Model.GetWeaponFamily(); }

	/**
	 * Khởi tạo tự động 2 sub-components cho Decoupled Master Rig (LowerBody và UpperBody).
	 */
	UFUNCTION(BlueprintCallable, Category = "Paperdoll|Rig")
	void InitializeDecoupledMasterRigComponents();

	UFUNCTION(BlueprintCallable, Category = "Paperdoll|Rig")
	void RegisterLowerBodyComponent(UPaperFlipbookComponent* InComp);

	UFUNCTION(BlueprintPure, Category = "Paperdoll|Rig")
	UPaperFlipbookComponent* GetLowerBodyComponent() const { return LowerBodyComponent; }

	UFUNCTION(BlueprintCallable, Category = "Paperdoll|Rig")
	void RegisterUpperBodyComponent(UPaperFlipbookComponent* InComp);

	UFUNCTION(BlueprintPure, Category = "Paperdoll|Rig")
	UPaperFlipbookComponent* GetUpperBodyComponent() const { return UpperBodyComponent; }

	UFUNCTION(BlueprintPure, Category = "Paperdoll|Rig")
	FName GetActiveLowerBodyVisualAssetId() const { return Model.GetLowerBodyVisualAssetId(); }

	UFUNCTION(BlueprintPure, Category = "Paperdoll|Rig")
	FName GetActiveUpperBodyVisualAssetId() const { return Model.GetUpperBodyVisualAssetId(); }

	/**
	 * Đăng ký / Ánh xạ một Flipbook Component cho một Layer đồ họa.
	 */
	UFUNCTION(BlueprintCallable, Category = "Paperdoll")
	void RegisterLayerFlipbookComponent(EPAPaperdollLayer Layer, UPaperFlipbookComponent* FlipbookComp);

	/**
	 * Lấy Flipbook Component được gán cho Layer chỉ định.
	 */
	UFUNCTION(BlueprintPure, Category = "Paperdoll")
	UPaperFlipbookComponent* GetLayerFlipbookComponent(EPAPaperdollLayer Layer) const;

	// -------------------------------------------------------------------------
	// Class Identity Sockets & Overlays API (Story visual-002, Sprint 7)
	// -------------------------------------------------------------------------

	/**
	 * Khởi tạo tự động 2 sub-components cho Class Identity (HelmCrestComponent và TabardComponent).
	 */
	UFUNCTION(BlueprintCallable, Category = "Paperdoll|Identity")
	void InitializeClassIdentityComponents();

	UFUNCTION(BlueprintCallable, Category = "Paperdoll|Identity")
	void RegisterHelmCrestComponent(UPaperFlipbookComponent* InComp);

	UFUNCTION(BlueprintPure, Category = "Paperdoll|Identity")
	UPaperFlipbookComponent* GetHelmCrestComponent() const { return HelmCrestComponent; }

	UFUNCTION(BlueprintCallable, Category = "Paperdoll|Identity")
	void RegisterTabardComponent(UPaperFlipbookComponent* InComp);

	UFUNCTION(BlueprintPure, Category = "Paperdoll|Identity")
	UPaperFlipbookComponent* GetTabardComponent() const { return TabardComponent; }

	/**
	 * Thiết lập Class Identity (tự động bind Crest và Tabard theo Class).
	 */
	UFUNCTION(BlueprintCallable, Category = "Paperdoll|Identity")
	void SetClassIdentity(FName InClassName);

	UFUNCTION(BlueprintCallable, Category = "Paperdoll|Identity")
	bool SetClassIdentityByTag(FGameplayTag InClassTag);

	UFUNCTION(BlueprintPure, Category = "Paperdoll|Identity")
	FGameplayTag GetCurrentClassTag() const { return Model.GetCurrentClassTag(); }

	UFUNCTION(BlueprintPure, Category = "Paperdoll|Identity")
	FName GetActiveHelmCrestVisualAssetId() const { return Model.GetHelmCrestVisualAssetId(); }

	UFUNCTION(BlueprintPure, Category = "Paperdoll|Identity")
	FName GetActiveTabardVisualAssetId() const { return Model.GetTabardVisualAssetId(); }

	UFUNCTION(BlueprintCallable, Category = "Paperdoll|Identity")
	void SetHelmCrestVisual(FName InAssetId);

	UFUNCTION(BlueprintCallable, Category = "Paperdoll|Identity")
	void SetTabardVisual(FName InAssetId);

	UFUNCTION(BlueprintCallable, Category = "Paperdoll|Identity")
	void SetHelmCrestVisibility(bool bVisible);

	UFUNCTION(BlueprintCallable, Category = "Paperdoll|Identity")
	void SetTabardVisibility(bool bVisible);

public:
	/** Sự kiện phát thanh khi ngoại hình một lớp trang bị thay đổi */
	UPROPERTY(BlueprintAssignable, Category = "Paperdoll|Events")
	FPAOnPaperdollVisualChanged OnPaperdollVisualChanged;

protected:
	/** Pure Data Model lưu trữ trạng thái hiển thị của các tầng Paperdoll */
	UPROPERTY(ReplicatedUsing = OnRep_Model)
	FPAPaperdollModel Model;

	UFUNCTION()
	virtual void OnRep_Model();

	/** Bắt sự kiện khi trang bị được mặc từ UPAEquipmentComponent */
	UFUNCTION()
	void HandleItemEquipped(EPAEquipmentSlot Slot, const FPAInventoryItemEntry& Item);

	/** Bắt sự kiện khi trang bị bị tháo từ UPAEquipmentComponent */
	UFUNCTION()
	void HandleItemUnequipped(EPAEquipmentSlot Slot, const FPAInventoryItemEntry& Item);

	/** Cập nhật trực quan và đồng bộ frame của các component đồ họa */
	void SynchronizeLayerSprites();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	/** Map tham chiếu đến các Flipbook Component tương ứng cho từng Layer (chỉ dùng trên Client/Standalone có render) */
	UPROPERTY(Transient)
	TMap<EPAPaperdollLayer, TObjectPtr<UPaperFlipbookComponent>> LayerComponents;

	/** Map tham chiếu đến 9 sub-component cho 9 slot (Story item-004) */
	UPROPERTY(Transient)
	TMap<EPAPaperdollSlot, TObjectPtr<UPaperFlipbookComponent>> SlotComponents;

	/** Flipbook Component hiển thị chuyển động Lower Body (Chân & Thắt lưng) */
	UPROPERTY(Transient)
	TObjectPtr<UPaperFlipbookComponent> LowerBodyComponent = nullptr;

	/** Flipbook Component hiển thị chuyển động Upper Body (Thân trên & Tay cầm vũ khí) */
	UPROPERTY(Transient)
	TObjectPtr<UPaperFlipbookComponent> UpperBodyComponent = nullptr;

	/** Flipbook Component hiển thị Mào Nón / Sừng Giáp (Story visual-002) */
	UPROPERTY(Transient)
	TObjectPtr<UPaperFlipbookComponent> HelmCrestComponent = nullptr;

	/** Flipbook Component hiển thị Cờ Ngực / Khăn Choàng (Story visual-002) */
	UPROPERTY(Transient)
	TObjectPtr<UPaperFlipbookComponent> TabardComponent = nullptr;

	/** Hướng ngắm 8 chiều hiện tại */
	UPROPERTY(Transient)
	EPAAimDirection8Way CurrentOrientation = EPAAimDirection8Way::East;

	/** Con trỏ yếu đến EquipmentComponent đã liên kết */
	TWeakObjectPtr<UPAEquipmentComponent> BoundEquipmentComponent;

	/** Material tĩnh hiện tại của vũ khí theo Rarity (Story item-005) */
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInterface> CurrentWeaponMaterial = nullptr;

	UPROPERTY(Transient)
	FGameplayTag CurrentWeaponRarityTag;
};
