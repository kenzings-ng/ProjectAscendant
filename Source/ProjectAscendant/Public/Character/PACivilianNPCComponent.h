// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Character/PACivilianNPCTypes.h"
#include "Character/PAPaperdollTypes.h"
#include "World/PACitadelComponent.h"
#include "PACivilianNPCComponent.generated.h"

class UPaperFlipbookComponent;
class UMaterialInstanceDynamic;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPAOnCivilianRoleChanged, EPACivilianRole, NewRole);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPAOnTownGuardStateChanged, EPAGuardAIState, NewState, AActor*, TargetOutlaw);

/**
 * UPACivilianNPCComponent
 *
 * Component quản lý NPC Dân sự trong khu vực an toàn (Story visual-003, EPIC-CHARACTER-VISUAL-001):
 * - Hỗ trợ 5 vai trò: Blacksmith, Merchant, Villager, TownGuard, QuestGiver.
 * - Tái sử dụng 100% 4 Master Rigs có sẵn (không sinh Rig thứ 5).
 * - Palette Swap qua Dynamic Material Instance M_PaperZD_Civilian_Base theo 3 Zone (Verdant, Ashen, Sanctum).
 * - Gắn 10 loại đạo cụ tĩnh 1-frame vào HandSocket_R và HandSocket_L.
 * - Town Guard AI Dual-State: Passive -> Lethal Gatekeeper Strike đẩy lùi Outlaw trong bán kính 1000cm,
 *   tái sử dụng 100% hoạt ảnh Upper Body của Weapon Family 3 (FB_Upper_2H_Polearm_Combo).
 * - Quest Giver Callout: Animation vẫy tay và bật icon '!' vàng trên đầu.
 * - 100% Headless Safe.
 */
UCLASS(ClassGroup = (Civilian), meta = (BlueprintSpawnableComponent))
class PROJECTASCENDANT_API UPACivilianNPCComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPACivilianNPCComponent();

	virtual void BeginPlay() override;

	// -------------------------------------------------------------------------
	// Role & Rig Mapping API (AC-1)
	// -------------------------------------------------------------------------

	/** Thiết lập vai trò dân sự, tự động bind Master Rig và đạo cụ mặc định */
	UFUNCTION(BlueprintCallable, Category = "Civilian|Role")
	void SetCivilianRole(EPACivilianRole NewRole);

	UFUNCTION(BlueprintPure, Category = "Civilian|Role")
	EPACivilianRole GetCivilianRole() const { return Model.Role; }

	UFUNCTION(BlueprintPure, Category = "Civilian|Role")
	EPAMasterRig GetMasterRig() const { return Model.MasterRig; }

	UFUNCTION(BlueprintPure, Category = "Civilian|Role")
	FName GetActiveLowerBodyVisualAssetId() const { return Model.LowerBodyVisualAssetId; }

	UFUNCTION(BlueprintPure, Category = "Civilian|Role")
	FName GetActiveUpperBodyVisualAssetId() const { return Model.UpperBodyVisualAssetId; }

	// -------------------------------------------------------------------------
	// Dynamic Palette Swap API (AC-2)
	// -------------------------------------------------------------------------

	/** Nạp Palette màu theo ZoneId (Verdant_Bastion, Ashen_Keep, Sanctum_Fortress) */
	UFUNCTION(BlueprintCallable, Category = "Civilian|Palette")
	bool SetZonePalette(FName ZoneId);

	/** Tự động đồng bộ Palette từ UPACitadelComponent */
	UFUNCTION(BlueprintCallable, Category = "Civilian|Palette")
	bool SetZonePaletteFromCitadel(const UPACitadelComponent* Citadel);

	UFUNCTION(BlueprintPure, Category = "Civilian|Palette")
	FName GetCurrentZoneId() const { return Model.CurrentZoneId; }

	UFUNCTION(BlueprintPure, Category = "Civilian|Palette")
	const FPACivilianPalettePreset& GetCurrentPalettePreset() const { return Model.CurrentPalette; }

	// -------------------------------------------------------------------------
	// Hand Props & Callout API (AC-3)
	// -------------------------------------------------------------------------

	/** Gắn đạo cụ tĩnh vào tay */
	UFUNCTION(BlueprintCallable, Category = "Civilian|Props")
	void AttachCivilianProp(FName PropVisualId, bool bLeftHand);

	UFUNCTION(BlueprintPure, Category = "Civilian|Props")
	FName GetAttachedProp(bool bLeftHand) const { return Model.GetAttachedProp(bLeftHand); }

	/** Kích hoạt Callout vẫy tay và icon '!' của Quest Giver */
	UFUNCTION(BlueprintCallable, Category = "Civilian|Quest")
	void PlayCalloutAnimation();

	UFUNCTION(BlueprintCallable, Category = "Civilian|Quest")
	void StopCalloutAnimation();

	UFUNCTION(BlueprintPure, Category = "Civilian|Quest")
	bool IsCalloutActive() const { return Model.bCalloutActive; }

	UFUNCTION(BlueprintPure, Category = "Civilian|Quest")
	FName GetActiveCalloutVisualId() const { return Model.ActiveCalloutVisualId; }

	// -------------------------------------------------------------------------
	// Town Guard AI Dual-State & Outlaw Deterrence API (AC-4)
	// -------------------------------------------------------------------------

	/**
	 * Đánh giá mối đe dọa từ Target Actor trong bán kính 1000cm:
	 * Nếu Target là Outlaw (Karma < 0 / bIsOutlaw == true), chuyển sang CombatActive
	 * và tung đòn đâm chí mạng tái sử dụng FB_Upper_2H_Polearm_Combo kèm Knockback.
	 */
	UFUNCTION(BlueprintCallable, Category = "Civilian|TownGuard")
	bool EvaluateTargetThreat(AActor* TargetActor, float DetectionRadius = 1000.0f);

	/** Thi hành đòn đâm trừng phạt Outlaw với lực đẩy lùi Knockback 800 cm/s */
	UFUNCTION(BlueprintCallable, Category = "Civilian|TownGuard")
	bool ExecuteLethalGatekeeperStrike(AActor* TargetActor);

	UFUNCTION(BlueprintPure, Category = "Civilian|TownGuard")
	EPAGuardAIState GetGuardAIState() const { return Model.GuardState; }

	UFUNCTION(BlueprintPure, Category = "Civilian|TownGuard")
	bool IsLethalStrikeTriggered() const { return Model.bLethalStrikeTriggered; }

	// -------------------------------------------------------------------------
	// Subcomponent Management
	// -------------------------------------------------------------------------

	UFUNCTION(BlueprintCallable, Category = "Civilian|Props")
	void InitializePropComponents();

	UFUNCTION(BlueprintCallable, Category = "Civilian|Props")
	void RegisterRightHandPropComponent(UPaperFlipbookComponent* InComp);

	UFUNCTION(BlueprintPure, Category = "Civilian|Props")
	UPaperFlipbookComponent* GetRightHandPropComponent() const { return RightHandPropComponent; }

	UFUNCTION(BlueprintCallable, Category = "Civilian|Props")
	void RegisterLeftHandPropComponent(UPaperFlipbookComponent* InComp);

	UFUNCTION(BlueprintPure, Category = "Civilian|Props")
	UPaperFlipbookComponent* GetLeftHandPropComponent() const { return LeftHandPropComponent; }

	// -------------------------------------------------------------------------
	// Model Access
	// -------------------------------------------------------------------------

	const FPACivilianNPCModel& GetModel() const { return Model; }
	FPACivilianNPCModel& GetMutableModel() { return Model; }

public:
	UPROPERTY(BlueprintAssignable, Category = "Civilian|Events")
	FPAOnCivilianRoleChanged OnCivilianRoleChanged;

	UPROPERTY(BlueprintAssignable, Category = "Civilian|Events")
	FPAOnTownGuardStateChanged OnTownGuardStateChanged;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Civilian|Model")
	FPACivilianNPCModel Model;

	void SynchronizeVisuals();

private:
	UPROPERTY(Transient)
	TObjectPtr<UPaperFlipbookComponent> RightHandPropComponent = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UPaperFlipbookComponent> LeftHandPropComponent = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> DynamicPaletteMaterial = nullptr;
};
