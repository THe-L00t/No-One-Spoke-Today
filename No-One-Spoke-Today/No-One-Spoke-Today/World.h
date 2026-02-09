#pragma once
#include "pch.h"
#include "City.h"
#include "Human.h"
#include "Event.h"
#include "Navigation.h"

// 게임 종료 상태
enum class GameEndState {
	None,				// 게임 진행 중
	Victory_Good,		// 안정지대 도착 - 좋은 결말
	Victory_Normal,		// 안정지대 도착 - 보통 결말
	Victory_Bad,		// 안정지대 도착 - 나쁜 결말
	GameOver_Coup,		// 쿠데타
	GameOver_Collapse,	// 도시 붕괴
	GameOver_Exodus,	// 집단 이탈
	GameOver_Starvation	// 자원 고갈
};

class World
{
public:
	World();

	void Update(float);
	void Display() const;
	void Debug();

	EventManager* GetEventManager();
	City* GetCity();
	Navigation* GetNavigation();
	Human* GetHumans(int);
	int GetHumansSize() const;
	std::vector<std::unique_ptr<Human>>& GetHumansVector();
	int GetCurrentDay() const;
	int GetMonth() const;
	int GetDay() const;
	float GetAccumulatedTime() const;

	// 플레이어 구역 관리
	Region GetPlayerRegion() const;
	void SetPlayerRegion(Region r);
	bool MovePlayerToRegion(Region target);
	std::vector<Region> GetAccessibleRegions() const;

	// 구역별 시민 관리
	std::vector<Human*> GetHumansInRegion(Region r);
	int GetHumanCountInRegion(Region r) const;

	// 구역 이벤트 알림 관리
	void AddRegionEventAlert(Region r, const std::string& eventName);
	const std::vector<std::pair<Region, std::string>>& GetRegionEventAlerts() const;
	void ClearRegionEventAlerts();
	bool HasUnseenEventInRegion(Region r) const;
	void MarkRegionEventSeen(Region r);
	const std::set<Region>& GetUnseenEventRegions() const;

	void SetCurrentDay(int d);
	void SetMonth(int m);
	void SetDay(int d);
	void SetAccumulatedTime(float t);
	void ClearHumans();
	void AddHuman(std::unique_ptr<Human> h);

private:
	int currentDay{ 1 };
	int month{ 4 };
	int day{ 12 };

	float accumulatedTime{};
	static constexpr float dayDuration = 480.0f; // 8분 = 480초

	std::unique_ptr<City> city;
	std::vector<std::unique_ptr<Human>> humans;
	std::unique_ptr<EventManager> eventManager;
	std::unique_ptr<Navigation> navigation;

	// 플레이어 위치 및 구역 이벤트 관리
	Region playerRegion{ Region::Cockpit };
	std::vector<std::pair<Region, std::string>> regionEventAlerts;
	std::set<Region> unseenEventRegions;

	// 하부구동부 지시 관리
	int angleOrderCountToday{ 0 };		// 오늘 각도 지시 횟수
	int lastAngleOrderDay{ -1 };		// 마지막 지시한 날

public:
	// 하부구동부 지시 관련
	int GetAngleOrderCountToday() const { return angleOrderCountToday; }
	void SetAngleOrderCountToday(int count) { angleOrderCountToday = count; }
	void SetLastAngleOrderDay(int day) { lastAngleOrderDay = day; }
	void IncrementAngleOrderCount();
	void ResetAngleOrderCountIfNewDay();

	// 게임 종료 상태 체크
	GameEndState CheckGameEndState();
	GameEndState GetCurrentGameEndState() const { return currentGameEndState; }
	bool IsGameEnded() const { return currentGameEndState != GameEndState::None; }
	int GetCriticalDaysCount() const { return criticalDaysCount; }
	int GetStarvationDaysCount() const { return starvationDaysCount; }

private:
	// 게임 종료 상태 추적
	GameEndState currentGameEndState{ GameEndState::None };
	int criticalDaysCount{ 0 };		// 회생불가 상태 연속 일수
	int starvationDaysCount{ 0 };	// 자원 고갈 상태 연속 일수
	int lastCriticalCheckDay{ -1 };	// 마지막 체크한 날

	// 내부 체크 함수들
	bool CheckCoupCondition() const;
	bool CheckCollapseCondition();
	bool CheckStarvationCondition();
	bool CheckExodusCondition() const;
	GameEndState CheckVictoryCondition();
};

