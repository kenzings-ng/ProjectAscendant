// Copyright Project Ascendant. All Rights Reserved.

#include "Animation/PAPaper2DSocketUtility.h"
#include "PaperSprite.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"

bool UPAPaper2DSocketUtility::ParseSocketsMetadata(const FString& JsonFilePath, TArray<FPAFrameSocketData>& OutFrames)
{
	OutFrames.Empty();

	FString ResolvedPath = JsonFilePath;
	if (FPaths::IsRelative(ResolvedPath))
	{
		ResolvedPath = FPaths::ProjectDir() / JsonFilePath;
	}

	FString JsonContent;
	if (!FFileHelper::LoadFileToString(JsonContent, *ResolvedPath))
	{
		UE_LOG(LogTemp, Warning, TEXT("UPAPaper2DSocketUtility: Failed to load file: %s"), *ResolvedPath);
		return false;
	}

	TSharedPtr<FJsonObject> RootObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonContent);
	if (!FJsonSerializer::Deserialize(Reader, RootObject) || !RootObject.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("UPAPaper2DSocketUtility: Failed to parse JSON from %s"), *ResolvedPath);
		return false;
	}

	const TArray<TSharedPtr<FJsonValue>>* SocketsArray = nullptr;
	if (!RootObject->TryGetArrayField(TEXT("per_frame_sockets"), SocketsArray) || !SocketsArray)
	{
		return false;
	}

	for (const TSharedPtr<FJsonValue>& Val : *SocketsArray)
	{
		TSharedPtr<FJsonObject> FrameObj = Val->AsObject();
		if (!FrameObj.IsValid())
		{
			continue;
		}

		FPAFrameSocketData Data;
		Data.Frame = FrameObj->GetIntegerField(TEXT("frame"));

		// Pivot
		const TArray<TSharedPtr<FJsonValue>>* PivotArr = nullptr;
		if (FrameObj->TryGetArrayField(TEXT("foot_pivot"), PivotArr) && PivotArr && PivotArr->Num() >= 2)
		{
			Data.FootPivot = FVector2D((*PivotArr)[0]->AsNumber(), (*PivotArr)[1]->AsNumber());
		}

		// Helm
		const TArray<TSharedPtr<FJsonValue>>* HelmArr = nullptr;
		if (FrameObj->TryGetArrayField(TEXT("helm_socket"), HelmArr) && HelmArr && HelmArr->Num() >= 2)
		{
			Data.HelmSocket = FVector2D((*HelmArr)[0]->AsNumber(), (*HelmArr)[1]->AsNumber());
		}

		// Hand R
		const TArray<TSharedPtr<FJsonValue>>* HandRArr = nullptr;
		if (FrameObj->TryGetArrayField(TEXT("hand_r"), HandRArr) && HandRArr && HandRArr->Num() >= 2)
		{
			Data.HandR = FVector2D((*HandRArr)[0]->AsNumber(), (*HandRArr)[1]->AsNumber());
		}

		// Hand L
		const TArray<TSharedPtr<FJsonValue>>* HandLArr = nullptr;
		if (FrameObj->TryGetArrayField(TEXT("hand_l"), HandLArr) && HandLArr && HandLArr->Num() >= 2)
		{
			Data.HandL = FVector2D((*HandLArr)[0]->AsNumber(), (*HandLArr)[1]->AsNumber());
		}

		// Waist Y
		Data.WaistY = FrameObj->GetNumberField(TEXT("waist_y"));

		OutFrames.Add(Data);
	}

	return OutFrames.Num() > 0;
}

bool UPAPaper2DSocketUtility::ApplySocketsToSprite(UPaperSprite* Sprite, const FPAFrameSocketData& SocketData, float PixelsPerUnit)
{
	if (!Sprite)
	{
		return false;
	}

	const float PPU = (PixelsPerUnit > 0.0f) ? PixelsPerUnit : 1.0f;
	const FVector2D Pivot = SocketData.FootPivot;

	auto MakeTransform = [Pivot, PPU](const FVector2D& PixelCoord) -> FTransform
	{
		// X: Right positive; Z: Up positive (114 - Y)
		const float OffsetX = (PixelCoord.X - Pivot.X) / PPU;
		const float OffsetZ = (Pivot.Y - PixelCoord.Y) / PPU;
		return FTransform(FVector(OffsetX, 0.0f, OffsetZ));
	};

	class UPAPaperSpriteSocketAccessor : public UPaperSprite
	{
	public:
		static TArray<FPaperSpriteSocket>& GetSockets(UPaperSprite* InSprite)
		{
			return static_cast<UPAPaperSpriteSocketAccessor*>(InSprite)->Sockets;
		}
	};

	auto SetOrAddSocket = [Sprite](FName SocketName, const FTransform& Transform)
	{
		if (FPaperSpriteSocket* Existing = Sprite->FindSocket(SocketName))
		{
			Existing->LocalTransform = Transform;
			return;
		}
		FPaperSpriteSocket NewSocket;
		NewSocket.SocketName = SocketName;
		NewSocket.LocalTransform = Transform;
		UPAPaperSpriteSocketAccessor::GetSockets(Sprite).Add(NewSocket);
	};

	SetOrAddSocket(FName(TEXT("Hand_R")), MakeTransform(SocketData.HandR));
	SetOrAddSocket(FName(TEXT("Hand_L")), MakeTransform(SocketData.HandL));
	SetOrAddSocket(FName(TEXT("Helm")), MakeTransform(SocketData.HelmSocket));
	SetOrAddSocket(FName(TEXT("Waist")), MakeTransform(FVector2D(Pivot.X, SocketData.WaistY)));

	return true;
}
