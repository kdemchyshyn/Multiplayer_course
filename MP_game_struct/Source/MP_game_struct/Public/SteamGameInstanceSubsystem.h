// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "SteamGameInstanceSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnSessionFoundDelegate, FString, SessionId, FName, SessionName, int32, SearchResultIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionSearchCompleteDelegate, bool, bWasSuccessful);

/**
 * 
 */
UCLASS()
class MP_GAME_STRUCT_API USteamGameInstanceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UPROPERTY(BlueprintAssignable, Category = "Online|Session")
	FOnSessionFoundDelegate OnSessionFound;

	UPROPERTY(BlueprintAssignable, Category = "Online|Session")
	FOnSessionSearchCompleteDelegate OnSessionSearchComplete;

private:
	IOnlineSessionPtr SessionInterface;

	FOnlineSessionSettings SessionSettings;

	void InitSettings(int32 NumberOfPlayers);

	TSharedPtr<FOnlineSessionSearch> SessionSearch;

public:
	UFUNCTION(BlueprintCallable, Category = "Online|Session")
	void CreateServerSession(FName SessionName, int32 NumberOfPlayers);

protected:
	void OnCreateSessionCompleted(FName SessionName, bool bWasSuccessful);

private:
	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	FDelegateHandle CreateSessionCompleteDelegateHandle;

public:
	UFUNCTION(BlueprintCallable, Category = "Online|Session")
	void FindServerSessions();

protected:
	void OnFindSessionsComplete(bool bWasSuccessful);

private:
	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
	FDelegateHandle FindSessionsCompleteDelegateHandle;

public:
	UFUNCTION(BlueprintCallable, Category = "Online|Session")
	void JoinServerSession(int32 SessionIndex);

	UFUNCTION(BlueprintCallable, Category = "Online|Session")
	void DestroyServerSession(FName SessionName);

protected:
	void OnJoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnDestroySessionCompleted(FName SessionName, bool bWasSuccessful);

private:
	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
	FDelegateHandle JoinSessionCompleteDelegateHandle;

	FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;
	FDelegateHandle DestroySessionCompleteDelegateHandle;
};
