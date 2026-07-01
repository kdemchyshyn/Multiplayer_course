# Death match architecture
## Diagram
```mermaid
classDiagram
     class TDMGameMode {
        -FTimerHandle MatchTimerHandle
        +ATDMGameMode()
        +BeginPlay() void
        +PostLogin(APlayerController* NewPlayer) void
        +CheckWinCondition() void
        +ScoreKill(APlayerController* victimController, APlayerController* killerController) void
        -UpdateMatchTimer() void
    }

    class TDMGameState {
        -TArray~int32~ TeamScores
        -float RemainingMatchTime
        +FOnPropertiesChanged OnTeamScoreChanged
        +FOnPropertyFloatChanged OnMatchTimeChanged
        +ATDMGameState()
        +BeginPlay() void
        +GetLifetimeReplicatedProps(TArray~FLifetimeProperty~ OutLifetimeProps) void
        -OnRep_TeamScores() void
        -OnRep_RemainingMatchTime() void
        +AddTeamScore(int32 TeamId) void
        +ResetTeamScores() void
        +GetTeamScore(int32 TeamId) int32
        +SetRemainingMatchTime(float Amount) void
        +GetRemainingMatchTime() float
    }

    class TDMPlayerState {
        -int32 TeamId
        -int32 Kills
        -int32 Deaths
        +FOnPropertyIntChanged OnTeamIdChanged
        +FOnPropertyIntChanged OnKillsChanged
        +FOnPropertyIntChanged OnDeathsChanged
        +ATDMPlayerState()
        +GetLifetimeReplicatedProps(TArray~FLifetimeProperty~ OutLifetimeProps) void
        -OnRep_TeamId() void
        -OnRep_Kills() void
        -OnRep_Deaths() void
        +SetTeam(int32 Id) void
        +AddKill() void
        +AddDeath() void
        +GetTeamId() int32
        +GetKills() int32
        +GetDeaths() int32
    }

    GameMode <|-- TDMGameMode
    GameState <|-- TDMGameState
    PlayerState <|-- TDMPlayerState

    %% Interaction relationships (Dependencies)
    TDMGameMode ..> TDMGameState : Updates (RemainingMatchTime)
    TDMGameMode ..> TDMGameState : Updates (TeamScores)
    TDMGameMode ..> TDMPlayerState : Updates (Kills/Deaths)
    TDMGameMode ..> TDMPlayerState : Assigns (Team)

    TDMGameState o-- TDMPlayerState : Aggregates (Inherited 'PlayerArray')
```

## Responsibilities
### GameMode
This is server only class that controls match flow and players connections. The only source of thruth that orchestrates everything for the match:
- Assigns team to the logined player
- Scores team points and personal player statistic(kills/deaths)
- Updates remaining match time using timer
- Controls match state -> ends it and decides winner

### GameState
Shared current state of the game, accessible from every point. Contains general information and is replicated for everyone:
- Contains replicated TeamScores (Updated by GameMode)
- Contains replicated RemainingMatchTime (Updated and controled by GameMode)
- Contains list of all PlayerStates (PlayerArray)

### PlayerState
Individual player information, shared and replicated on server and local player. Contains personal player statistic:
- Contains replicated Kills (Updated by GameMode)
- Contains replicated Deaths (Updated by GameMode)
- Contains replicated TeamId (Assigned by GameMode)
