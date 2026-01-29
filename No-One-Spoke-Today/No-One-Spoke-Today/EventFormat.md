# Event Definition Format

이벤트 정의 파일(.txt) 작성 규칙

---

## 파일 구조

```
################################################################################
# 이벤트 정의 파일
# 버전: 1.0
################################################################################

[EVENT_START]
...이벤트 내용...
[EVENT_END]

[EVENT_START]
...다음 이벤트...
[EVENT_END]
```

- 모든 이벤트는 `[EVENT_START]`로 시작, `[EVENT_END]`로 종료
- `#`으로 시작하는 줄은 주석
- 빈 줄은 무시됨

---

## 필드 정의

### 기본 정보 (필수)

| 필드 | 타입 | 설명 | 예시 |
|------|------|------|------|
| `id` | string | 고유 식별자 | `dust_storm_001` |
| `name` | string | 이벤트 이름 | `먼지 폭풍 접근` |
| `category` | enum | 카테고리 | `Environment` |
| `description` | string | 설명 텍스트 | `거대한 먼지 폭풍이...` |

### Category 값

| 값 | 설명 |
|----|------|
| `Environment` | 환경/자연재해/외부 요인 |
| `Personal` | 개인 심리/정신 상태 |
| `Social` | 사회적/집단 역학 |
| `Interpersonal` | 대인 관계 |
| `CityWide` | 도시 전체 영향 |

---

### 트리거 조건

| 필드 | 타입 | 설명 | 기본값 |
|------|------|------|--------|
| `trigger_type` | enum | `random` 또는 `condition` | `condition` |
| `city_mask` | uint16 (hex) | 검사할 City 비트 마스크 | `0x0000` |
| `city_min` | uint16 (hex) | City 코드 최소값 | `0x0000` |
| `city_max` | uint16 (hex) | City 코드 최대값 | `0xFFFF` |
| `human_mask` | uint64 (hex) | 검사할 Human 비트 마스크 | `0x0` |
| `human_min` | uint64 (hex) | Human 코드 최소값 | `0x0` |
| `human_max` | uint64 (hex) | Human 코드 최대값 | `0xFFFFFFFFFFFFFFFF` |
| `min_human_count` | int | 최소 충족 인원 | `0` |
| `region_id` | int | 특정 지역 ID (-1 = 무관) | `-1` |

#### 트리거 없는 이벤트 (랜덤 발생)

```
trigger_type      = random
city_mask         = 0x0000
human_mask        = 0x0
min_human_count   = 0
```

---

### 쿨타임

| 필드 | 타입 | 설명 | 기본값 |
|------|------|------|--------|
| `cooldown_min` | int | 최소 쿨타임 (일) | `1` |
| `cooldown_max` | int | 최대 쿨타임 (일) | `7` |

발동 시 `cooldown_min` ~ `cooldown_max` 사이에서 랜덤 결정

---

### 효과 범위 (Effect Scope)

| 필드 | 타입 | 설명 |
|------|------|------|
| `effect_scope` | enum | 기본 효과 적용 대상 |

#### Effect Scope 값

| 값 | 설명 |
|----|------|
| `Triggered` | 트리거 조건을 충족한 Human들만 |
| `AllHumans` | 도시 전체 Human |
| `City` | 도시 지표만 (Human 영향 없음) |
| `Region` | 특정 지역의 Human들 (region_id 사용) |
| `Custom` | 커스텀 로직 (custom_effect_id 필요) |

---

### 효과 (Effect)

#### 즉시 효과 (선택지 없을 때)

```
immediate_effect  = [EffectType], [Field], [Delta]
immediate_effect  = [EffectType], [Field], [Delta]
```

#### 선택지 효과

```
choice_N_effect   = [EffectType], [Field], [Delta], [Scope]
```

#### EffectType 값

| 값 | 설명 | Field 타입 |
|----|------|-----------|
| `ModifyDrive` | Human 누적값 변경 | DriveField |
| `ModifyCityMetric` | 도시 지표 변경 | MetricField |
| `ModifyMentalState` | 정신상태 직접 변경 | MentalField |
| `Kill` | 사망 처리 | - (delta = 인원수) |
| `AddImmigrant` | 이주민 추가 | - (delta = 인원수) |
| `Custom` | 커스텀 효과 | custom_id |

#### DriveField 값

| 값 | 설명 | 범위 |
|----|------|------|
| `StressLoad` | 스트레스 누적 | 0~10000 |
| `EmotionalArousal` | 감정 각성 | 0~10000 |
| `Fatigue` | 피로 | 0~10000 |
| `CognitiveCapacity` | 인지 능력 | 0~10000 |
| `InterpersonalTrust` | 대인 신뢰 | 0~10000 |
| `SocialSafety` | 사회적 안전감 | 0~10000 |
| `SenseOfControl` | 통제감 | 0~10000 |
| `Motivation` | 동기 | 0~10000 |

#### MetricField 값

| 값 | 설명 | 범위 |
|----|------|------|
| `Mood` | 도시 분위기 | 0~10000 |
| `Activity` | 활동성 | 0~10000 |
| `Scarcity` | 자원 부족 | 0~10000 |

#### MentalField 값

| 값 | 설명 |
|----|------|
| `Arousal` | 감정 각성 상태 (Calm/Tense/Irritable/Hostile) |
| `Social` | 사회적 태도 (Neutral/Cooperative/Withdrawn) |
| `Energy` | 에너지 상태 (Normal/Fatigued/Exhausted) |
| `Control` | 통제 상태 (Autonomous/Dependent/Stubborn) |

---

### 선택지 (Choices)

| 필드 | 타입 | 설명 |
|------|------|------|
| `requires_player` | bool | 플레이어 선택 필요 여부 |
| `choice_N_text` | string | N번 선택지 텍스트 |
| `choice_N_effect` | effect | N번 선택지 효과 (복수 가능) |

- **최소 선택지**: 2개
- **최대 선택지**: 5개
- N은 1부터 시작

---

## 비트 코드 참조

### CityCode (16비트)

```
[15:12] 예약
[11:9]  Scarcity (3비트, 0~4 레벨)
[8:6]   Activity (3비트, 0~4 레벨)
[5:3]   Mood (3비트, 0~4 레벨)
[2:0]   예약
```

| 필드 | Shift | Mask |
|------|-------|------|
| Mood | 3 | `0x0038` |
| Activity | 6 | `0x01C0` |
| Scarcity | 9 | `0x0E00` |

### HumanCode (64비트)

```
[63:53] 예약
[52:50] Region (3비트, 소속 구역)
[49:32] Trait (18비트: 6개 x 3비트)
[31:8]  Drives (24비트: 8개 x 3비트)
[7:0]   MentalState (8비트: 4개 x 2비트)
```

#### MentalState (비트 0~7)

| 필드 | Shift | Mask |
|------|-------|------|
| Arousal | 0 | `0x03` |
| Social | 2 | `0x0C` |
| Energy | 4 | `0x30` |
| Control | 6 | `0xC0` |

#### Drives (비트 8~31)

| 필드 | Shift | Mask |
|------|-------|------|
| StressLoad | 8 | `0x0700` |
| EmotionalArousal | 11 | `0x3800` |
| Fatigue | 14 | `0x1C000` |
| CognitiveCapacity | 17 | `0xE0000` |
| InterpersonalTrust | 20 | `0x700000` |
| SocialSafety | 23 | `0x3800000` |
| SenseOfControl | 26 | `0x1C000000` |
| Motivation | 29 | `0xE0000000` |

#### Traits (비트 32~49)

| 필드 | Shift | Mask |
|------|-------|------|
| Rationality | 32 | `0x700000000` |
| Aggression | 35 | `0x3800000000` |
| Planning | 38 | `0x1C000000000` |
| Dependency | 41 | `0xE0000000000` |
| Rigidity | 44 | `0x700000000000` |
| EmotionalSensitivity | 47 | `0x3800000000000` |

#### Region (비트 50~52)

| 필드 | Shift | Mask |
|------|-------|------|
| Region | 50 | `0x1C000000000000` |

---

## 예시: 트리거 조건 계산

### 예시 1: 스트레스 레벨 3 이상인 Human

```
StressLoad는 비트 8~10 (Shift=8, Mask=0x0700)
레벨 3 = 0b011 << 8 = 0x0300

human_mask = 0x0700
human_min  = 0x0300
human_max  = 0xFFFFFFFFFFFFFFFF
```

### 예시 2: 적대적(Hostile) 상태인 Human

```
Arousal은 비트 0~1 (Shift=0, Mask=0x03)
Hostile = 3 = 0b11

human_mask = 0x03
human_min  = 0x03
human_max  = 0x03
```

### 예시 3: 도시 Mood가 낮음 (레벨 1 이하)

```
Mood는 비트 3~5 (Shift=3, Mask=0x0038)
레벨 1 = 0b001 << 3 = 0x0008

city_mask = 0x0038
city_min  = 0x0000
city_max  = 0x0008
```

---

## 전체 예시

```
################################################################################
# 환경 이벤트: 먼지 폭풍
################################################################################

[EVENT_START]
id                = dust_storm_001
name              = 먼지 폭풍 접근
category          = Environment
description       = 거대한 먼지 폭풍이 도시를 향해 다가오고 있습니다. 전방 시야가 급격히 나빠지고 있습니다.

trigger_type      = random
city_mask         = 0x0000
human_mask        = 0x0
min_human_count   = 0

cooldown_min      = 5
cooldown_max      = 12

effect_scope      = AllHumans
requires_player   = true

choice_1_text     = 엔진 출력을 높여 폭풍을 돌파한다
choice_1_effect   = ModifyDrive, Fatigue, 1000, AllHumans
choice_1_effect   = ModifyDrive, StressLoad, 500, AllHumans
choice_1_effect   = ModifyCityMetric, Scarcity, 300, City

choice_2_text     = 속도를 줄이고 폭풍이 지나가길 기다린다
choice_2_effect   = ModifyDrive, StressLoad, 800, AllHumans
choice_2_effect   = ModifyDrive, SocialSafety, -300, AllHumans
choice_2_effect   = ModifyCityMetric, Activity, -500, City

choice_3_text     = 우회 경로를 탐색한다
choice_3_effect   = ModifyDrive, CognitiveCapacity, -500, Triggered
choice_3_effect   = ModifyDrive, Fatigue, 500, AllHumans
choice_3_effect   = ModifyCityMetric, Scarcity, 500, City

choice_4_text     = 시민들에게 대피 준비를 지시한다
choice_4_effect   = ModifyDrive, SenseOfControl, 300, AllHumans
choice_4_effect   = ModifyDrive, StressLoad, 300, AllHumans
choice_4_effect   = ModifyCityMetric, Activity, -200, City
[EVENT_END]

################################################################################
# 개인 심리 이벤트: 번아웃
################################################################################

[EVENT_START]
id                = burnout_001
name              = 번아웃 증후군
category          = Personal
description       = 극심한 스트레스와 피로에 시달리던 시민이 완전히 무너져버렸습니다. 더 이상 아무것도 할 수 없다고 합니다.

trigger_type      = condition
city_mask         = 0x0000
human_mask        = 0x0000000000006700
human_min         = 0x0000000000004300
human_max         = 0xFFFFFFFFFFFFFFFF
min_human_count   = 1

cooldown_min      = 2
cooldown_max      = 5

effect_scope      = Triggered
requires_player   = true

choice_1_text     = 강제로 휴식을 취하게 한다
choice_1_effect   = ModifyDrive, Fatigue, -2000, Triggered
choice_1_effect   = ModifyDrive, Motivation, -800, Triggered
choice_1_effect   = ModifyCityMetric, Activity, -100, City

choice_2_text     = 다른 시민들에게 업무를 분담시킨다
choice_2_effect   = ModifyDrive, StressLoad, -1500, Triggered
choice_2_effect   = ModifyDrive, Fatigue, 400, AllHumans
choice_2_effect   = ModifyDrive, InterpersonalTrust, -200, Triggered

choice_3_text     = 개인 상담을 통해 마음을 달랜다
choice_3_effect   = ModifyDrive, StressLoad, -1000, Triggered
choice_3_effect   = ModifyDrive, SenseOfControl, 500, Triggered
choice_3_effect   = ModifyDrive, InterpersonalTrust, 300, Triggered

choice_4_text     = 무시하고 계속 일하게 한다
choice_4_effect   = ModifyDrive, StressLoad, 500, Triggered
choice_4_effect   = ModifyDrive, Motivation, -1500, Triggered
choice_4_effect   = ModifyDrive, SocialSafety, -400, AllHumans

choice_5_text     = 중요한 직책에서 제외시킨다
choice_5_effect   = ModifyDrive, SenseOfControl, -1000, Triggered
choice_5_effect   = ModifyDrive, StressLoad, -800, Triggered
choice_5_effect   = ModifyCityMetric, Activity, -200, City
[EVENT_END]

################################################################################
# 사회 이벤트: 집단 공황
################################################################################

[EVENT_START]
id                = collective_panic_001
name              = 집단 공황
category          = Social
description       = 불안이 전염되어 여러 시민들이 동시에 공황 상태에 빠졌습니다. 비명과 울음소리가 도시 곳곳에서 들려옵니다.

trigger_type      = condition
city_mask         = 0x0038
city_min          = 0x0000
city_max          = 0x0010
human_mask        = 0x0000000000000003
human_min         = 0x0000000000000002
human_max         = 0xFFFFFFFFFFFFFFFF
min_human_count   = 5

cooldown_min      = 7
cooldown_max      = 14

effect_scope      = AllHumans
requires_player   = true

choice_1_text     = 차분하게 상황을 설명하고 안심시킨다
choice_1_effect   = ModifyDrive, StressLoad, -600, AllHumans
choice_1_effect   = ModifyDrive, InterpersonalTrust, 400, Triggered
choice_1_effect   = ModifyCityMetric, Mood, 200, City

choice_2_text     = 강경하게 질서를 잡는다
choice_2_effect   = ModifyDrive, StressLoad, -300, AllHumans
choice_2_effect   = ModifyDrive, SocialSafety, -500, AllHumans
choice_2_effect   = ModifyDrive, SenseOfControl, -400, Triggered

choice_3_text     = 신뢰받는 시민들에게 진정을 맡긴다
choice_3_effect   = ModifyDrive, StressLoad, -400, AllHumans
choice_3_effect   = ModifyDrive, InterpersonalTrust, 200, AllHumans
choice_3_effect   = ModifyDrive, Motivation, 300, Triggered
[EVENT_END]
```

---

## 파일 저장 위치

```
data/events.txt
```

---

## 버전 기록

| 버전 | 날짜 | 변경 내용 |
|------|------|----------|
| 1.0 | 2025-01 | 초기 버전 |
