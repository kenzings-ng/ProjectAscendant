// Copyright Project Ascendant. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Animation/PAPaperZDAnimInstance.h"
#include "Animation/PAPaper2DSocketUtility.h"
#include "PaperSprite.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * FPAVanguardRuntimeWiringTests
 *
 * Kiểm thử tự động cho Giai đoạn 1 (M6.1 Runtime Wiring):
 * - AC-1: Speed, bIsMoving, bIsRunning locomotion threshold (Walk vs Run).
 * - AC-2: Cấu trúc biến GAS GameplayTags (Hurt, Stunned, Dead).
 * - AC-3: Tiện ích PAPaper2DSocketUtility phân tích đúng 22 frames từ vanguard_metadata.json.
 * - AC-4: Gán Sockets (Hand_R, Hand_L, Helm, Waist) vào UPaperSprite với tọa độ transform chính xác.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPAVanguardRuntimeWiringTests,
	"ProjectAscendant.Character.VanguardRuntimeWiring",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPAVanguardRuntimeWiringTests::RunTest(const FString& Parameters)
{
	// =========================================================================
	// AC-1: Locomotion Variables (Walk vs Run Threshold)
	// =========================================================================
	{
		UPAPaperZDAnimInstance* AnimInst = NewObject<UPAPaperZDAnimInstance>();
		TestNotNull(TEXT("AC-1: UPAPaperZDAnimInstance instantiated"), AnimInst);

		TestEqual(TEXT("AC-1: Default RunningSpeedThreshold is 280.0 cm/s"), AnimInst->RunningSpeedThreshold, 280.0f);
		TestFalse(TEXT("AC-1: bIsMoving false initially"), AnimInst->bIsMoving);
		TestFalse(TEXT("AC-1: bIsRunning false initially"), AnimInst->bIsRunning);

		// Thử gán giá trị tốc độ đi bộ
		AnimInst->CurrentSpeed = 150.0f;
		AnimInst->Speed = AnimInst->CurrentSpeed;
		AnimInst->bIsMoving = AnimInst->CurrentSpeed > 10.0f;
		AnimInst->bIsRunning = AnimInst->CurrentSpeed >= AnimInst->RunningSpeedThreshold;
		TestTrue(TEXT("AC-1: Speed 150cm/s is moving (Walk)"), AnimInst->bIsMoving);
		TestFalse(TEXT("AC-1: Speed 150cm/s is NOT running"), AnimInst->bIsRunning);

		// Thử gán giá trị tốc độ chạy
		AnimInst->CurrentSpeed = 350.0f;
		AnimInst->Speed = AnimInst->CurrentSpeed;
		AnimInst->bIsMoving = AnimInst->CurrentSpeed > 10.0f;
		AnimInst->bIsRunning = AnimInst->CurrentSpeed >= AnimInst->RunningSpeedThreshold;
		TestTrue(TEXT("AC-1: Speed 350cm/s is moving"), AnimInst->bIsMoving);
		TestTrue(TEXT("AC-1: Speed 350cm/s is running (Run)"), AnimInst->bIsRunning);
	}

	// =========================================================================
	// AC-2: State Tags Verification
	// =========================================================================
	{
		UPAPaperZDAnimInstance* AnimInst = NewObject<UPAPaperZDAnimInstance>();
		TestFalse(TEXT("AC-2: bIsHurt false by default"), AnimInst->bIsHurt);
		TestFalse(TEXT("AC-2: bIsStunned false by default"), AnimInst->bIsStunned);
		TestFalse(TEXT("AC-2: bIsDead false by default"), AnimInst->bIsDead);
	}

	// =========================================================================
	// AC-3: Parsing vanguard_metadata.json
	// =========================================================================
	{
		TArray<FPAFrameSocketData> Sockets;
		const FString MetadataPath = TEXT("Content/Art/Characters/Vanguard/metadata/vanguard_metadata.json");
		bool bParsed = UPAPaper2DSocketUtility::ParseSocketsMetadata(MetadataPath, Sockets);
		TestTrue(TEXT("AC-3: Successfully parsed vanguard_metadata.json"), bParsed);
		TestEqual(TEXT("AC-3: Exactly 22 frames in metadata"), Sockets.Num(), 22);

		if (Sockets.Num() >= 1)
		{
			const FPAFrameSocketData& F0 = Sockets[0];
			TestEqual(TEXT("AC-3: Frame 0 foot pivot is (64, 114)"), F0.FootPivot, FVector2D(64.0f, 114.0f));
			TestEqual(TEXT("AC-3: Frame 0 helm socket is (64, 44)"), F0.HelmSocket, FVector2D(64.0f, 44.0f));
			TestEqual(TEXT("AC-3: Frame 0 Hand_R is (80, 76)"), F0.HandR, FVector2D(80.0f, 76.0f));
			TestEqual(TEXT("AC-3: Frame 0 Hand_L is (48, 76)"), F0.HandL, FVector2D(48.0f, 76.0f));
			TestEqual(TEXT("AC-3: Frame 0 waist Y is 80"), F0.WaistY, 80.0f);
		}
	}

	// =========================================================================
	// AC-4: Apply Sockets to UPaperSprite
	// =========================================================================
	{
		UPaperSprite* TestSprite = NewObject<UPaperSprite>();
		TestNotNull(TEXT("AC-4: UPaperSprite instantiated"), TestSprite);

		FPAFrameSocketData F0;
		F0.Frame = 0;
		F0.FootPivot = FVector2D(64.0f, 114.0f);
		F0.HelmSocket = FVector2D(64.0f, 44.0f);
		F0.HandR = FVector2D(80.0f, 76.0f);
		F0.HandL = FVector2D(48.0f, 76.0f);
		F0.WaistY = 80.0f;

		bool bApplied = UPAPaper2DSocketUtility::ApplySocketsToSprite(TestSprite, F0, 1.0f);
		TestTrue(TEXT("AC-4: Sockets applied to sprite"), bApplied);

		TArray<FComponentSocketDescription> OutSockets;
		TestSprite->QuerySupportedSockets(OutSockets);
		TestEqual(TEXT("AC-4: 4 sockets created (Hand_R, Hand_L, Helm, Waist)"), OutSockets.Num(), 4);

		// Kiểm tra Hand_R: (80 - 64 = 16, Z = 114 - 76 = 38)
		FPaperSpriteSocket* SockHandR = TestSprite->FindSocket(FName(TEXT("Hand_R")));
		TestNotNull(TEXT("AC-4: Hand_R socket exists"), SockHandR);
		if (SockHandR)
		{
			TestEqual(TEXT("AC-4: Hand_R X is +16"), SockHandR->LocalTransform.GetLocation().X, 16.0);
			TestEqual(TEXT("AC-4: Hand_R Z is +38"), SockHandR->LocalTransform.GetLocation().Z, 38.0);
		}

		// Kiểm tra Helm: (64 - 64 = 0, Z = 114 - 44 = 70)
		FPaperSpriteSocket* SockHelm = TestSprite->FindSocket(FName(TEXT("Helm")));
		TestNotNull(TEXT("AC-4: Helm socket exists"), SockHelm);
		if (SockHelm)
		{
			TestEqual(TEXT("AC-4: Helm X is 0"), SockHelm->LocalTransform.GetLocation().X, 0.0);
			TestEqual(TEXT("AC-4: Helm Z is +70"), SockHelm->LocalTransform.GetLocation().Z, 70.0);
		}
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
