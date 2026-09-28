// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PAPaper2DSocketUtility.generated.h"

class UPaperSprite;

USTRUCT(BlueprintType)
struct FPAFrameSocketData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sockets")
	int32 Frame = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sockets")
	FVector2D FootPivot = FVector2D(64.0f, 114.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sockets")
	FVector2D HelmSocket = FVector2D(64.0f, 44.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sockets")
	FVector2D HandR = FVector2D(80.0f, 76.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sockets")
	FVector2D HandL = FVector2D(48.0f, 76.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sockets")
	float WaistY = 80.0f;
};

/**
 * UPAPaper2DSocketUtility
 *
 * Tiện ích đọc metadata JSON (vanguard_metadata.json) và cấu hình Paper2D Sprite Sockets
 * (Hand_R, Hand_L, Helm, Waist) trực tiếp lên UPaperSprite.
 */
UCLASS()
class PROJECTASCENDANT_API UPAPaper2DSocketUtility : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Phân tích file metadata JSON để lấy thông tin sockets theo từng frame.
	 */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Paper2D|Sockets")
	static bool ParseSocketsMetadata(const FString& JsonFilePath, TArray<FPAFrameSocketData>& OutFrames);

	/**
	 * Gán các Socket (Hand_R, Hand_L, Helm, Waist) vào UPaperSprite từ dữ liệu frame.
	 * Tọa độ Paper2D: X sang phải, Z hướng lên, gốc tọa độ đặt tại Foot Pivot.
	 */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Paper2D|Sockets")
	static bool ApplySocketsToSprite(UPaperSprite* Sprite, const FPAFrameSocketData& SocketData, float PixelsPerUnit = 1.0f);
};
