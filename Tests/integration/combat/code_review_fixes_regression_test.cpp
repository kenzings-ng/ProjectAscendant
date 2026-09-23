// Copyright Project Ascendant. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Character/PABaseCharacter.h"
#include "Combat/PAGameplayAbility_Finisher.h"
#include "Combat/PAGameplayAbility_MeleeAttack.h"
#include "Combat/AscendantAttributeSet.h"
#include "Inventory/PAInventoryComponent.h"
#include "Inventory/PAEquipmentComponent.h"
#include "Inventory/PAInventoryTypes.h"
#include "Inventory/PAItemStaticDataAsset.h"
#include "Network/PANetLagCompensation.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagsManager.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * FPACodeReviewFixesRegressionTest
 *
 * Kiểm thử hồi quy tự động cho 5 lỗi Code Review:
 *  - Fix 1: Death & Stagger Handling (HandleOutOfHealth adds State.Dead tag, disables movement/collision; HandlePostureBroken adds State.Broken/Stunned tags).
 *  - Fix 2: Finisher Target Validation & Server-Authoritative Execution (ValidateExecutionTarget distance <= 250cm, Broken/Staggered tag, target != attacker).
 *  - Fix 3: Item UID Retention (AddItemToSlot preserves existing UID when provided from unequip/stash).
 *  - Fix 4: Melee Lag Compensation Position History Buffer & Rewind (PositionHistoryBuffer records snapshots, GetRewoundTransform linearly interpolates past transforms).
 *  - Fix 5: AttributeSet Broadcast Deduplication (State flags prevent duplicate broadcasts on successive hits/DoTs, reset upon recovery).
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPACodeReviewFixesRegressionTest,
	"ProjectAscendant.Core.Combat.ReviewFixesRegression",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPACodeReviewFixesRegressionTest::RunTest(const FString& Parameters)
{
	// Đăng ký Native GameplayTags phòng trường hợp chạy headless test
	UGameplayTagsManager& TagMgr = UGameplayTagsManager::Get();
	TagMgr.AddNativeGameplayTag(FName(TEXT("State.Dead")));
	TagMgr.AddNativeGameplayTag(FName(TEXT("State.Stunned")));
	TagMgr.AddNativeGameplayTag(FName(TEXT("State.Broken")));
	TagMgr.AddNativeGameplayTag(FName(TEXT("State.Staggered")));
	TagMgr.AddNativeGameplayTag(FName(TEXT("State.Invulnerable")));
	TagMgr.AddNativeGameplayTag(FName(TEXT("Ability.Skill.Finisher")));
	TagMgr.AddNativeGameplayTag(FName(TEXT("Ability.Skill.Attack")));

	const FGameplayTag TagDead = FGameplayTag::RequestGameplayTag(FName("State.Dead"));
	const FGameplayTag TagStunned = FGameplayTag::RequestGameplayTag(FName("State.Stunned"));
	const FGameplayTag TagBroken = FGameplayTag::RequestGameplayTag(FName("State.Broken"));
	const FGameplayTag TagStaggered = FGameplayTag::RequestGameplayTag(FName("State.Staggered"));

	// =========================================================================
	// Fix 1: Death & Stagger Handling (PABaseCharacter)
	// =========================================================================
	{
		APABaseCharacter* TestChar = NewObject<APABaseCharacter>();
		TestNotNull(TEXT("Fix 1: Khởi tạo APABaseCharacter thành công"), TestChar);

		if (TestChar)
		{
			UAbilitySystemComponent* ASC = TestChar->GetAbilitySystemComponent();
			TestNotNull(TEXT("Fix 1: Character ASC hợp lệ"), ASC);

			// 1.1 Kiểm tra HandlePostureBroken
			TestChar->HandlePostureBroken(nullptr);
			if (ASC)
			{
				TestTrue(TEXT("Fix 1: Khi vỡ posture, ASC phải có thẻ State.Broken"), ASC->HasMatchingGameplayTag(TagBroken));
				TestTrue(TEXT("Fix 1: Khi vỡ posture, ASC phải có thẻ State.Stunned"), ASC->HasMatchingGameplayTag(TagStunned));
			}

			// 1.2 Kiểm tra HandleOutOfHealth
			TestChar->HandleOutOfHealth(nullptr);
			if (ASC)
			{
				TestTrue(TEXT("Fix 1: Khi hết máu, ASC phải có thẻ State.Dead"), ASC->HasMatchingGameplayTag(TagDead));
			}

			if (UCharacterMovementComponent* MoveComp = TestChar->GetCharacterMovement())
			{
				TestEqual(TEXT("Fix 1: Khi chết, MovementMode phải là MOVE_None"),
					MoveComp->MovementMode.GetValue(), (uint8)MOVE_None);
			}

			if (UCapsuleComponent* Capsule = TestChar->GetCapsuleComponent())
			{
				TestEqual(TEXT("Fix 1: Khi chết, Pawn collision response phải là ECR_Ignore"),
					Capsule->GetCollisionResponseToChannel(ECC_Pawn), ECR_Ignore);
			}
		}
	}

	// =========================================================================
	// Fix 2: Finisher Target Validation & Server-Authoritative Execution
	// =========================================================================
	{
		UPAGameplayAbility_Finisher* Finisher = NewObject<UPAGameplayAbility_Finisher>();
		TestNotNull(TEXT("Fix 2: Khởi tạo UPAGameplayAbility_Finisher thành công"), Finisher);

		APABaseCharacter* Attacker = NewObject<APABaseCharacter>();
		APABaseCharacter* Target = NewObject<APABaseCharacter>();

		if (Finisher && Attacker && Target)
		{
			Attacker->SetActorLocation(FVector(0.0f, 0.0f, 0.0f));
			Target->SetActorLocation(FVector(200.0f, 0.0f, 0.0f)); // Cự ly 200cm (<= 250cm)

			// 2.1 Không có tag State.Broken -> Target không hợp lệ
			TestFalse(TEXT("Fix 2: Target trong tầm nhưng không mang thẻ Broken -> ValidateExecutionTarget = false"),
				Finisher->ValidateExecutionTarget(Attacker, Target));

			// 2.2 Mang tag State.Broken -> Target hợp lệ
			if (UAbilitySystemComponent* TargetASC = Target->GetAbilitySystemComponent())
			{
				TargetASC->AddLooseGameplayTag(TagBroken);
				TestTrue(TEXT("Fix 2: Target trong cự ly 200cm có thẻ State.Broken -> ValidateExecutionTarget = true"),
					Finisher->ValidateExecutionTarget(Attacker, Target));

				// 2.3 Quá cự ly (> 250cm) -> Không hợp lệ
				Target->SetActorLocation(FVector(300.0f, 0.0f, 0.0f));
				TestFalse(TEXT("Fix 2: Target có thẻ Broken nhưng ngoài cự ly 300cm (> 250cm) -> ValidateExecutionTarget = false"),
					Finisher->ValidateExecutionTarget(Attacker, Target));

				// 2.4 Cự ly hợp lệ với thẻ State.Staggered
				Target->SetActorLocation(FVector(150.0f, 0.0f, 0.0f));
				TargetASC->RemoveLooseGameplayTag(TagBroken);
				TargetASC->AddLooseGameplayTag(TagStaggered);
				TestTrue(TEXT("Fix 2: Target trong cự ly 150cm có thẻ State.Staggered -> ValidateExecutionTarget = true"),
					Finisher->ValidateExecutionTarget(Attacker, Target));

				// 2.5 Attacker tự target chính mình -> Không hợp lệ
				TestFalse(TEXT("Fix 2: Attacker không thể tự kết liễu bản thân"),
					Finisher->ValidateExecutionTarget(Attacker, Attacker));
			}
		}
	}

	// =========================================================================
	// Fix 3: Item UID Retention on Unequip & Stash
	// =========================================================================
	{
		UPAInventoryComponent* Inv = NewObject<UPAInventoryComponent>();
		TestNotNull(TEXT("Fix 3: Khởi tạo UPAInventoryComponent thành công"), Inv);

		if (Inv)
		{
			UItemStaticDataAsset* SwordData = NewObject<UItemStaticDataAsset>();
			SwordData->ItemId = FName(TEXT("Item_IronSword"));

			const FGuid OriginalUID = FGuid::NewGuid();
			// Thêm item vào slot 0 với UID xác định (mô phỏng unequip/stash withdrawal)
			const bool bAdded = Inv->AddItemToSlot(0, SwordData, 1, FPAItemInstanceData(), OriginalUID);
			TestTrue(TEXT("Fix 3: Thêm item vào slot 0 thành công"), bAdded);

			const FPAInventoryItemEntry* StoredItem = Inv->GetItemAtSlot(0);
			TestNotNull(TEXT("Fix 3: Lấy item tại slot 0 thành công"), StoredItem);
			if (StoredItem)
			{
				TestEqual(TEXT("Fix 3: UID item phải được bảo toàn chính xác, không sinh UID ngẫu nhiên mới"),
					StoredItem->ItemInstanceUID, OriginalUID);
			}

			// Thêm item không truyền UID (mô phỏng loot đồ mới) -> Phải tự sinh UID hợp lệ
			UItemStaticDataAsset* PotionData = NewObject<UItemStaticDataAsset>();
			PotionData->ItemId = FName(TEXT("Item_HealthPotion"));
			const bool bLootAdded = Inv->AddItemToSlot(1, PotionData, 5);
			TestTrue(TEXT("Fix 3: Thêm item loot mới vào slot 1 thành công"), bLootAdded);
			const FPAInventoryItemEntry* LootItem = Inv->GetItemAtSlot(1);
			TestNotNull(TEXT("Fix 3: Lấy item tại slot 1 thành công"), LootItem);
			if (LootItem)
			{
				TestTrue(TEXT("Fix 3: Item loot mới phải có UID hợp lệ"), LootItem->ItemInstanceUID.IsValid());
				TestNotEqual(TEXT("Fix 3: Item loot mới không được trùng UID với item trước"),
					LootItem->ItemInstanceUID, OriginalUID);
			}
		}
	}

	// =========================================================================
	// Fix 4: Melee Lag Compensation Position History Buffer & Rewind
	// =========================================================================
	{
		APABaseCharacter* Char = NewObject<APABaseCharacter>();
		TestNotNull(TEXT("Fix 4: Khởi tạo APABaseCharacter thành công"), Char);

		if (Char)
		{
			FPAPositionHistoryBuffer& Buffer = Char->GetMutablePositionHistoryBuffer();

			// Thêm 2 điểm vị trí: T=10.0s tại (0,0,0) và T=10.1s tại (100,0,0)
			Buffer.RecordSnapshot(10.0f, FVector(0.0f, 0.0f, 0.0f), FRotator::ZeroRotator);
			Buffer.RecordSnapshot(10.1f, FVector(100.0f, 0.0f, 0.0f), FRotator::ZeroRotator);

			// Tua lại tại thời điểm T=10.05s (giữa 2 snapshot)
			FPAPastTransform Rewound;
			const bool bRewound = Char->GetRewoundTransform(10.05f, Rewound);
			TestTrue(TEXT("Fix 4: GetRewoundTransform tại 10.05s phải thành công"), bRewound);
			if (bRewound)
			{
				TestNearlyEqual(TEXT("Fix 4: Vị trí nội suy X phải xấp xỉ 50.0cm"),
					(float)Rewound.Location.X, 50.0f, 1.0f);
				TestNearlyEqual(TEXT("Fix 4: Vị trí nội suy Y phải xấp xỉ 0.0cm"),
					(float)Rewound.Location.Y, 0.0f, 0.01f);
			}

			// Thẩm định client timestamp với window 200ms
			float ClampedTime = 0.0f;
			const bool bValidTs = UPANetcodeMath::ValidateClientTimestamp(10.05f, 10.15f, 0.2f, ClampedTime);
			TestTrue(TEXT("Fix 4: Timestamp trong vòng 100ms (<= 200ms) phải hợp lệ"), bValidTs);
			TestNearlyEqual(TEXT("Fix 4: ClampedTime phải bằng ClientTime khi hợp lệ"), (float)ClampedTime, 10.05f, 0.001f);
		}
	}

	// =========================================================================
	// Fix 5: AttributeSet Broadcast Deduplication
	// =========================================================================
	{
		UAscendantAttributeSet* AttrSet = NewObject<UAscendantAttributeSet>();
		TestNotNull(TEXT("Fix 5: Khởi tạo UAscendantAttributeSet thành công"), AttrSet);

		if (AttrSet)
		{
			int32 OutOfHealthBroadcasts = 0;
			int32 PostureBrokenBroadcasts = 0;

			AttrSet->OnOutOfHealth.AddLambda([&OutOfHealthBroadcasts](AActor*) {
				OutOfHealthBroadcasts++;
			});

			AttrSet->OnPostureBroken.AddLambda([&PostureBrokenBroadcasts](AActor*) {
				PostureBrokenBroadcasts++;
			});

			AttrSet->InitMaxHealth(100.0f);
			AttrSet->InitHealth(100.0f);
			AttrSet->InitMaxPosture(100.0f);
			AttrSet->InitPosture(0.0f);

			// 5.1 Giảm máu về 0 -> Bắn delegate lần đầu
			AttrSet->InitHealth(0.0f);
			AttrSet->EvaluateOutOfHealthBroadcast();
			TestEqual(TEXT("Fix 5: Giảm Health về 0 -> OnOutOfHealth bắn 1 lần"), OutOfHealthBroadcasts, 1);

			// Tiếp tục nhận sát thương thừa (overkill / DoT) khi đang ở 0 -> KHÔNG được bắn lặp
			AttrSet->InitHealth(0.0f);
			AttrSet->EvaluateOutOfHealthBroadcast();
			TestEqual(TEXT("Fix 5: Tiếp tục nhận sát thương khi đang 0 HP -> OnOutOfHealth KHÔNG được bắn lặp"), OutOfHealthBroadcasts, 1);

			// Hồi máu > 0 rồi lại chết -> Reset cờ và cho phép bắn lần 2
			AttrSet->InitHealth(50.0f);
			AttrSet->EvaluateOutOfHealthBroadcast();
			AttrSet->InitHealth(0.0f);
			AttrSet->EvaluateOutOfHealthBroadcast();
			TestEqual(TEXT("Fix 5: Hồi sinh rồi chết lại -> OnOutOfHealth bắn lần 2"), OutOfHealthBroadcasts, 2);

			// 5.2 Tăng Posture chạm ngưỡng MaxPosture -> Bắn delegate lần đầu
			AttrSet->InitPosture(100.0f);
			AttrSet->EvaluatePostureBrokenBroadcast();
			TestEqual(TEXT("Fix 5: Posture đạt MaxPosture -> OnPostureBroken bắn 1 lần"), PostureBrokenBroadcasts, 1);

			// Tiếp tục tăng Posture khi đã vỡ thế -> KHÔNG được bắn lặp
			AttrSet->InitPosture(100.0f);
			AttrSet->EvaluatePostureBrokenBroadcast();
			TestEqual(TEXT("Fix 5: Posture duy trì MaxPosture -> OnPostureBroken KHÔNG được bắn lặp"), PostureBrokenBroadcasts, 1);

			// Reset posture về 0 rồi vỡ thế lại -> Cho phép bắn lần 2
			AttrSet->InitPosture(0.0f);
			AttrSet->EvaluatePostureBrokenBroadcast();
			AttrSet->InitPosture(100.0f);
			AttrSet->EvaluatePostureBrokenBroadcast();
			TestEqual(TEXT("Fix 5: Reset Posture rồi vỡ thế lại -> OnPostureBroken bắn lần 2"), PostureBrokenBroadcasts, 2);
		}
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
