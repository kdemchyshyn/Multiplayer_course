// Fill out your copyright notice in the Description page of Project Settings.


#include "SteamGameInstanceSubsystem.h"
#include "OnlineSubsystem.h"
#include "Online/OnlineSessionNames.h"

void USteamGameInstanceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	IOnlineSubsystem* SubsystemAPI = IOnlineSubsystem::Get();
	if (SubsystemAPI)
	{
		SessionInterface = SubsystemAPI->GetSessionInterface();
	}

	CreateSessionCompleteDelegate = FOnCreateSessionCompleteDelegate::CreateUObject(this, &USteamGameInstanceSubsystem::OnCreateSessionCompleted);
	FindSessionsCompleteDelegate = FOnFindSessionsCompleteDelegate::CreateUObject(this, &USteamGameInstanceSubsystem::OnFindSessionsComplete);
	JoinSessionCompleteDelegate = FOnJoinSessionCompleteDelegate::CreateUObject(this, &USteamGameInstanceSubsystem::OnJoinSessionCompleted);
	DestroySessionCompleteDelegate = FOnDestroySessionCompleteDelegate::CreateUObject(this, &USteamGameInstanceSubsystem::OnDestroySessionCompleted);
}

void USteamGameInstanceSubsystem::Deinitialize()
{
	// Destroy any lingering session so Steam doesn't keep it alive after exit
	if (SessionInterface.IsValid())
	{
		const IOnlineSessionPtr& SI = SessionInterface;
		TArray<FNamedOnlineSession*> Sessions;
		// Destroy the default named session if it exists
		if (SI->GetNamedSession(NAME_GameSession))
		{
			SI->DestroySession(NAME_GameSession);
		}
	}

	Super::Deinitialize();
}

void USteamGameInstanceSubsystem::InitSettings(int32 NumberOfPlayers)
{
	SessionSettings.bIsLANMatch = false;
	SessionSettings.NumPublicConnections = NumberOfPlayers;
	SessionSettings.bAllowJoinInProgress = true;
	SessionSettings.bUsesPresence = true;
	SessionSettings.bUseLobbiesIfAvailable = true;
	SessionSettings.bShouldAdvertise = true;
}

void USteamGameInstanceSubsystem::CreateServerSession(FName SessionName, int32 NumberOfPlayers)
{
	if (!SessionInterface.IsValid()) return;

	if (SessionInterface->GetNamedSession(NAME_GameSession))
	{
		UE_LOG(LogTemp, Error, TEXT("Session with such name already exists!"));
		return;
	}

	InitSettings(NumberOfPlayers);

	SessionSettings.Set(FName(TEXT("SESSION_NAME")), SessionName.ToString(), EOnlineDataAdvertisementType::ViaOnlineService);

	SessionSettings.Set(FName(TEXT("CUSTOM_MATCH_TAG")), FString("P8TestBuild"), EOnlineDataAdvertisementType::ViaOnlineService);

	CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

	SessionInterface->CreateSession(0, NAME_GameSession, SessionSettings);
}

void USteamGameInstanceSubsystem::OnCreateSessionCompleted(FName SessionName, bool bWasSuccessful)
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
	}

	if (bWasSuccessful)
	{
		GetWorld()->ServerTravel("/Game/ThirdPerson/Lvl_ThirdPerson?listen?port=7777");
	}
}

void USteamGameInstanceSubsystem::DestroyServerSession(FName SessionName)
{
	if (!SessionInterface.IsValid()) return;

	DestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);
	SessionInterface->DestroySession(SessionName);
}

void USteamGameInstanceSubsystem::OnDestroySessionCompleted(FName SessionName, bool bWasSuccessful)
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
	}
}

void USteamGameInstanceSubsystem::FindServerSessions()
{
	if (!SessionInterface.IsValid()) return;

	SessionSearch = MakeShareable(new FOnlineSessionSearch());

	SessionSearch->MaxSearchResults = 10000;
	SessionSearch->bIsLanQuery = false;
	SessionSearch->QuerySettings.Set(FName(TEXT("CUSTOM_MATCH_TAG")), FString("P8TestBuild"), EOnlineComparisonOp::Equals);
	SessionSearch->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);

	FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);
	SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
}

void USteamGameInstanceSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
	}

	if (bWasSuccessful && SessionSearch->SearchResults.Num() > 0)
	{
		for (int32 Index = 0; Index < SessionSearch->SearchResults.Num(); ++Index)
		{
			const FOnlineSessionSearchResult& SearchResult = SessionSearch->SearchResults[Index];

			FString SessionId = SearchResult.GetSessionIdStr();

			FString CustomName;
			if (!SearchResult.Session.SessionSettings.Get(FName(TEXT("SESSION_NAME")), CustomName))
			{
				CustomName = SessionId; 
			}

			OnSessionFound.Broadcast(SessionId, FName(*CustomName), Index);
		}
	}

	OnSessionSearchComplete.Broadcast(bWasSuccessful);
}

void USteamGameInstanceSubsystem::JoinServerSession(int32 SessionIndex)
{
	if (!SessionInterface.IsValid() || !SessionSearch.IsValid()) return;

	if (SessionSearch->SearchResults.IsValidIndex(SessionIndex))
	{
		FOnlineSessionSearchResult SearchResult = SessionSearch->SearchResults[SessionIndex];

		SearchResult.Session.SessionSettings.bUsesPresence = true;
		SearchResult.Session.SessionSettings.bUseLobbiesIfAvailable = true;

		JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);

		// Use NAME_GameSession as the local session name (FOnlineSessionSearchResult has no SessionName in UE5)
		SessionInterface->JoinSession(0, NAME_GameSession, SearchResult);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Attempted to join an invalid session index."));
	}
}

void USteamGameInstanceSubsystem::OnJoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
	}

	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		FString ConnectString;

		if (SessionInterface->GetResolvedConnectString(SessionName, ConnectString))
		{
			APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
			if (PlayerController)
			{
				PlayerController->ClientTravel(ConnectString, TRAVEL_Absolute);
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to join session. Result code: %d"), static_cast<int32>(Result));
	}
}
