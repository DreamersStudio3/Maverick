# 2026 소울라이크 게임 시장 조사

> 조사 기준일: 2026-07-26
>
> 조사 범위: 글로벌 PC·콘솔 프리미엄 게임 시장
>
> 목적: [Maverick MDA 전투 구조 v0.3](./MDAFrameworkAnalysis.md)의 시장 배경과 제품 포지셔닝 참고
>
> 세부 시장 참고: [10시간 내외 3인칭 보스러시 액션 RPG 시장 분석](./boss_rush_market_analysis_ko.md)
>
> 주의: 소울라이크만을 독립 집계한 공인 시장 통계는 없다. 이 문서는 **관측값, 제3자 추정값, 기획용 시나리오**를 구분한다.

## 0. 결론

2026년의 소울라이크는 더 이상 틈새 장르는 아니지만, 전체 게임 시장을 좌우하는 독립 대형 장르도 아니다.

- 엄격한 의미의 소울라이크는 공인 매출 통계가 없다. 기획에서 사용할 **잠정 중심 시나리오**는 연간 소비자 매출 약 6억~10억 달러다.
- 넓은 민감도 범위는 **약 3.5억~14억 달러**다. 2025년 글로벌 프리미엄 본편 추정 시장의 1~4%를 적용한 가정이며 관측된 장르 매출이 아니다.
- 중심 범위는 전체 PC·콘솔 시장의 약 **0.6~1.1%**, 프리미엄 본편 시장의 약 **1.7~2.8%**에 해당한다.
- Steam에서 `Souls-like` 태그를 가진 출시작은 2025년 381개였지만 Steam 전체 출시작의 1.78%에 그쳤다. 공급은 많고 희소성은 낮다.
- 공식 누적 판매·출하 공개치만 모은 핵심 표본에서는 FromSoftware 개발작이 약 **80.7%**를 차지한다. 공급은 분산됐지만 수요는 상위 IP에 강하게 집중됐다.
- 2026년 경쟁의 중심은 난도 자체가 아니라 **소울식 학습·실패 루프를 협동, 로그라이트, PvPvE, 슈터, 덱빌더, 지역 문화와 어떻게 결합하는가**로 이동했다.

Maverick에 가장 중요한 결론은 “소울라이크를 얼마나 많이 구현했는가”보다 **현재 설계 중인 공격·스킬 순환이 다른 작품과 무엇이 다른지를 첫 30분 안에 증명해야 한다**는 점이다.

---

## 1. 조사 정의와 방법

### 1.1 두 개의 시장을 분리한다

#### 핵심 소울라이크

다음 문법을 중심으로 제품 전체가 설계된 게임을 가리킨다.

- 체크포인트와 적 재배치
- 사망 리스크와 자원 회수
- 행동 Commit과 정밀한 회피·방어
- 반복 학습을 요구하는 보스
- 제한된 회복 또는 스태미나·행동 자원
- 위험을 감수하는 탐색과 성장

`Dark Souls`, `Bloodborne`, `Sekiro`, `Elden Ring`, `Nioh`, `Lies of P`, `Lords of the Fallen`, `Wo Long` 등이 이 분석의 핵심 표본이다.

#### Souls-adjacent

소울라이크의 일부 전투 문법을 사용하지만 제작사와 시장이 액션 RPG, 캐릭터 액션, 하드코어 액션 등으로 소개하는 작품을 가리킨다.

`Black Myth: Wukong`, `Stellar Blade`, `Phantom Blade Zero` 등을 엄격한 시장규모에 그대로 넣지 않는다.

`Monster Hunter`, `Hades`, `Cuphead`, `Assassin's Creed`처럼 일부 전투 요소만 유사한 작품은 이 인접 장르보다도 더 바깥에 있다. VGI 표본에 이 작품들이 들어간 것은 Steam 사용자 태그 기반 집계의 오염 사례로 따로 취급한다.

### 1.2 증거 등급

| 등급 | 자료 | 사용 방식 |
| --- | --- | --- |
| A | 제작사·퍼블리셔 IR, 공식 판매 발표 | 공개 판매량의 하한과 최신 제품 상태 |
| B | Newzoo PC·콘솔 시장 자료 | 전체 모시장과 프리미엄 본편 시장 |
| C | SteamDB, Steam 공식 검색 | 출시작 수와 카탈로그 공급 비중 |
| D | Sensor Tower/VGI | Steam 태그 기반 판매 추정과 장기 경향 |
| E | 본 문서의 계산 | 엄격한 시장의 기획용 범위와 표본 내 비중 |

Valve는 태그별 매출을 공개하지 않고 Newzoo도 공개 자료에서 소울라이크를 독립 장르로 집계하지 않는다. 따라서 이 문서의 6억~10억 달러는 감사된 시장 통계가 아니라, 비교와 예산 검토를 위한 **잠정 기획 시나리오**다.

---

## 2. 시장 규모

### 2.1 모시장

| 기준 | 규모 | 성격 |
| --- | ---: | --- |
| 2025년 세계 게임 시장 | 2,016억 달러 | 모바일 포함 확정치 |
| 2025년 PC·콘솔 시장 | 883억 달러 | PC 436억+콘솔 447억 |
| 2026년 PC·콘솔 시장 | 943억 달러 | 전망치 |
| 2028년 PC·콘솔 시장 | 1,037억 달러 | 전망치 |
| 2025년 PC·콘솔 프리미엄 본편 | 약 352억 달러 | 2026년 2월 보고서의 2025년 추정치. PC 125억+콘솔 227억 |

Newzoo의 명목 소비자 지출 기준에는 패키지·디지털 본편, DLC, 인게임 결제, 구독이 포함되며 하드웨어, 광고, 세금은 제외된다. 프리미엄 본편 시장은 전체 PC·콘솔 시장 중 본편 판매만 분리한 값이다.

출처:

- [Newzoo PC & Console Gaming Report 2026](https://newzoo.com/reports/pc-console-gaming-report-2026)
- [Newzoo 2025 세계 게임 시장 결산](https://newzoo.com/articles/global-games-market-2025)
- [Newzoo 2026 보고서 공개 PDF](https://files.gameindustrylibrary.com/documents/pc-console-gaming-report-2026.pdf)

2025년 전체 PC·콘솔 883억 달러는 2026년 6월 갱신된 확정치다. 프리미엄 본편 352억 달러는 2026년 2월 보고서의 2025년 전망·추정치이며, 6월 확정 총액에 맞춘 프리미엄 세부 갱신치는 공개되지 않았다.

### 2.2 엄격한 소울라이크의 기획용 매출 시나리오

공식 장르 매출이 없으므로 2025년 프리미엄 본편 추정 시장 352억 달러에 가정 비중을 적용한다. 아래 1.0~4.0%와 그 안의 중심 구간은 관측된 점유율이 아니다.

| 시나리오 | 프리미엄 본편 내 비중 | 연간 소비자 매출 |
| --- | ---: | ---: |
| 보수적 | 1.0% | 약 3.5억 달러 |
| 중심 하단 | 1.7% | 약 6.0억 달러 |
| 중심 상단 | 2.8% | 약 10.0억 달러 |
| 확장 | 4.0% | 약 14.1억 달러 |

따라서 본 문서는 다음 두 기획값을 구분해 사용한다.

- **잠정 중심 시나리오:** 연 6억~10억 달러
- **민감도 전체 범위:** 연 3.5억~14억 달러

중심 범위는 2025년 전체 PC·콘솔 시장 대비 약 0.7~1.1%, 2026년 전망치 대비 약 0.6~1.1%다. 프리미엄 본편 시장 안에서는 약 1.7~2.8%다.

이 범위는 다음 자료와 충돌하지 않는지 확인하는 용도이지, 각 자료를 단순 합산해 만든 값이 아니다.

- 핵심 작품의 공식 누적 판매 공개 하한 약 1억 장
- 광의의 Steam 태그 출시 코호트 누적 판매 추정 약 2억 장
- Steam 전체 출시작 중 Souls-like 태그 비중 약 1.6~2.0%
- 흥행작이 수백만~수천만 장, 다수 작품이 장기 꼬리에 남는 집중 구조

### 2.3 광의의 Steam 태그 시장

Sensor Tower/VGI는 2015년부터 2025년 10월까지 출시된 Steam `Souls-like` 태그 게임의 누적 판매량을 약 **2억 530만 장**으로 추정했다.

| 주요 출시 코호트 발췌 | 보고서 시점까지의 누적 Steam 판매 추정 |
| --- | ---: |
| 2015 | 680만 장 |
| 2019 | 2,800만 장 |
| 2020 | 950만 장 |
| 2021 | 940만 장 |
| 2022 | 2,680만 장 |
| 2024 | 3,900만 장 |
| 2025 YTD | 2,040만 장 |

이 수치는 해당 연도의 실제 연간 판매량이 아니라 **그해 출시된 작품들이 보고서 작성 시점까지 누적으로 판매한 수량**이다.

또한 추정 상위권에 `Black Myth: Wukong`, `Monster Hunter: World`, `Hades`, `Cuphead`, `Assassin's Creed Valhalla` 등이 들어간다. Steam 사용자 태그를 기준으로 했기 때문에 엄격한 소울라이크 시장보다 범위가 매우 넓다.

출처: [Sensor Tower/VGI Souls-like Report, October 2025](https://app.sensortower.com/vgi/assets/reports/VGI_Soulslike_Report_October_2025.pdf)

---

## 3. 공급 비중과 경쟁 강도

SteamDB 기준 `Souls-like` 태그 출시작은 현재 누계 약 1,932개다. Steam 공식 검색과의 집계 차이를 고려하면 검색 가능한 현재 카탈로그는 약 1,860~1,930개 범위로 보는 것이 안전하다.

| 연도 | Souls-like 태그 출시 | Steam 전체 출시 | 출시 비중 |
| --- | ---: | ---: | ---: |
| 2020 | 113개 | - | - |
| 2021 | 198개 | - | - |
| 2022 | 224개 | - | - |
| 2023 | 259개 | - | - |
| 2024 | 367개 | 18,496개 | 1.98% |
| 2025 | 381개 | 21,382개 | 1.78% |
| 2026 YTD | 218개 | 13,444개 | 1.62% |

출처:

- [SteamDB Souls-like 출시 통계](https://steamdb.info/stats/releases/?tagid=29482)
- [SteamDB 전체 출시 통계](https://steamdb.info/stats/releases/)
- [Steam 공식 Souls-like 게임 검색](https://store.steampowered.com/search/?sort_by=Released_DESC&tags=29482&category1=998)

### 해석

- 소울라이크 출시작의 절대 수는 빠르게 늘었다.
- 2025년 출시작 수는 2024년보다 증가했지만 Steam 전체 출시가 더 빠르게 늘어 비중은 내려갔다.
- 2026년 YTD 비중도 1.62%로, 장르의 공급 희소성은 이미 사라졌다.
- 사용자 태그에는 인접 장르가 섞이지만, 플레이어가 소울라이크로 인식하는 경쟁작이 그만큼 넓다는 뜻이기도 하다.
- “소울라이크”라는 장르명만으로 검색 노출과 차별성을 얻기 어려운 시장이다.

---

## 4. 주요 게임의 공개 판매·출하 표본 비중

다음 표는 공식 판매·출하량이 공개된 핵심 작품만 모은 누적 표본이다.

이 비중은 **연간 시장점유율이 아니라 공개 누적 판매·출하 표본 내 산술 비중**이다. 작품별 발표일, 판매와 출하의 정의, 시리즈와 단일 작품의 단위가 다르다.

| 작품 | 최신 공개 누적치 | 표본 내 비중 |
| --- | ---: | ---: |
| [Dark Souls 시리즈](https://www.bandainamco.co.jp/en/ir/library/assets/pdf/2025/factbook2025_en.pdf) | 3,971만 장 | 39.2% |
| [Elden Ring](https://www.fromsoftware.jp/jp/pressrelease/20250530_eldenring_nightreign_salesdata.html) | 3,000만 장 이상 | 29.6% |
| [Sekiro](https://www.fromsoftware.jp/ww/pressrelease/20230926_sekiro_salesdata.html) | 1,000만 장 이상 | 9.9% |
| [Nioh 시리즈](https://www.koeitecmoamerica.com/news/nioh-3-reaches-over-1-million-copies-sold-worldwide-fastest-in-franchise-history/) | 1,000만 장 이상 | 9.9% |
| [Lies of P](https://cdn.neowiz.com/neowiz-site/assets/ir-library/Q4-2025_NWZ-IR-PPT_EN_F-icogfblh.pdf) | 400만 장 이상 | 4.0% |
| [Lords of the Fallen (2023)](https://cigames.com/ci-games-se-15-2026-wyniki-sprzedazy-gry-lords-of-the-fallen-osiagniecie-zwrotu-z-inwestycji/) | 250만 장 이상 | 2.5% |
| [Bloodborne](https://blog.playstation.com/2015/09/15/bloodborne-expansion-the-old-hunters-out-november-24th/) | 200만 장 이상 | 2.0% |
| [Remnant II](https://embracer.com/releases/embracer-group-publishes-interim-report-q2-july-september-2023-net-sales-increased-by-13-to-sek-10831-million/) | 200만 장 이상 | 2.0% |
| [Wo Long: Fallen Dynasty](https://www.koeitecmoeurope.com/news/koei-tecmo-announces-wo-long-fallen-dynasty-complete-edition-coming-to-nintendo-switch-2-on-3rd-september-2026/) | 100만 장 이상 | 1.0% |
| 합계 | 약 1억 121만 장 | 100% |

### 4.1 집중도

- FromSoftware 개발 계열은 `Dark Souls + Elden Ring + Sekiro + Bloodborne` 합계 약 8,171만 장이다.
- 핵심 공개 표본에서 FromSoftware 계열 비중은 약 **80.7%**다.
- `Dark Souls + Elden Ring` 두 행만으로 약 **68.9%**다.
- 별도 유료 협동작인 `Elden Ring Nightreign` 500만 장을 포함하면 FromSoftware 계열 비중은 약 **81.6%**가 된다.
- `Shadow of the Erdtree`는 1,000만 장이지만 DLC이므로 본편과의 중복을 피하기 위해 표본에서 제외한다.

`Nightreign`과 DLC 판매량 출처: [Bandai Namco·FromSoftware 2025-07-24 발표](https://www.bandainamcoent.co.jp/pdfs/HP_20250724_EN.pdf)

### 4.2 표본의 주의점

- `Dark Souls` 3,971만 장은 Bandai Namco가 원판매자인 해외분으로, 일본 내 FromSoftware 직접 판매분이 빠진 하한이다.
- `Bloodborne`은 2015년 200만 장 이후 공식 갱신치가 없다.
- `Lies of P` 400만 장은 본편과 DLC 판매 단위를 합친 수치이므로 본편의 고유 구매자 수와 같지 않다.
- `Wo Long`의 500만 플레이어, `Lies of P`의 700만 플레이어처럼 구독 서비스 이용자를 포함한 플레이어 수는 판매량과 합산하지 않는다.
- 공개에 성공한 작품이 과대표집되고 판매량을 공개하지 않은 작품이 빠져 있다.

### 4.3 인접 장르 포함 민감도

`Black Myth: Wukong`과 `Stellar Blade`를 소울라이크에 포함할지는 논쟁적이다. 두 작품 모두 공식 제품 분류는 액션 RPG에 가깝다.

- `Black Myth: Wukong`은 개발사 직접 발표 하한 1,000만 장, Guinness 확인치 2,000만 장, 2026년 중국 공공기관 계열 보도 3,000만 장으로 출처 시점이 다르다.
- `Stellar Blade`는 PS5와 PC 합계 300만 장 이상이다.
- 두 작품을 확장 표본에 넣고 `Wukong`을 1,000만~3,000만 장으로 적용하면 FromSoftware 계열 비중은 약 **61~72%**로 낮아진다.

출처:

- [Black Myth: Wukong 개발사 발표 1,000만 장](https://x.com/BlackMythGame/status/1826985302592049599)
- [Guinness 확인 2,000만 장](https://www.guinnessworldrecords.com/world-records/775494-fastest-selling-videogame-based-on-a-classic-novel)
- [중국 공공기관 계열 2026년 보도 3,000만 장](https://www.gd.news.cn/20260706/aac501890f9d4bda8732c955051a5c73/c.html)
- [Shift Up, Stellar Blade 300만 장 발표](https://shiftup.co.kr/news/news.php?category=&code=news&idx=275&ptype=view)

장르 경계를 넓히면 시장은 크게 보이고 FromSoftware 비중은 낮아진다. 따라서 외부 보고서에서는 항상 **핵심 소울라이크와 Souls-adjacent를 별도 표기**해야 한다.

---

## 5. 2009~2026년 시장 흐름

| 시기 | 시장 변화 | 대표 사례 |
| --- | --- | --- |
| 2009~2012 | 장르 문법 형성 | `Demon's Souls`, `Dark Souls` |
| 2013~2016 | 공식 정착과 첫 외부 모방 | `Dark Souls II·III`, `Bloodborne`, `Lords of the Fallen` |
| 2017~2021 | 다른 전투·미학·협동 구조와 결합 | `Nioh`, `Remnant`, `Code Vein`, `Sekiro` |
| 2022 | 블록버스터 시장으로 대중화 | `Elden Ring` |
| 2023~2024 | 외부 스튜디오 성공과 지역 문화 다변화 | `Lies of P`, `Remnant II`, `Wo Long`, `Black Myth`, `Another Crab's Treasure` |
| 2025~2026 | 세션형 협동과 장르 혼합이 제품 전략으로 이동 | `Nightreign`, `Nioh 3`, `The Duskbloods`, `Death Howl`, `Valor Mortis` |

### 5.1 2009~2012: 장르 문법의 형성

`Demon's Souls`가 사망 리스크, 제한적 회복, 체크포인트, 비동기 온라인을 결합했다. `Dark Souls`는 연결된 세계, 위험한 탐색, 반복 학습과 성취를 세계 시장에 정착시켰다.

출처:

- [Demon's Souls 공식 제품 정보](https://www.fromsoftware.jp/ww/detail.html?csm=070)
- [Dark Souls 공식 제품 정보](https://www.fromsoftware.jp/ww/detail.html?csm=086)

### 5.2 2013~2016: 장르 문법의 정착과 변형

`Dark Souls II`, `Bloodborne`, `Dark Souls III`로 장르가 시리즈 수준으로 고착됐다. `Bloodborne`은 방패 중심의 신중한 전투를 더 빠르고 공격적인 전투로 변주했다. 2014년 `Lords of the Fallen`부터 외부 스튜디오의 상업적 모방도 본격화됐다.

출처: [FromSoftware 제품 연혁](https://www.fromsoftware.jp/ww/products.html)

### 5.3 2017~2021: 재조합 가능한 설계 문법으로 확장

- `Nioh`: 스탠스, 자세 전환, 전리품 파밍
- `Remnant: From the Ashes`: 총기, 절차 생성, 3인 협동
- `Code Vein`: 애니메이션 미학과 동료
- `Sekiro`: 체간, 받아치기, 점프, 갈고리

이 시기부터 소울라이크는 하나의 중세 판타지 형식이 아니라, 다른 장르에 이식할 수 있는 전투·성장 문법이 됐다.

출처:

- [Nioh 공식](https://teamninja-studio.com/nioh/)
- [Remnant 공식](https://fromtheashes.remnantgame.com/en/about)
- [Sekiro 공식](https://www.fromsoftware.jp/ww/pressrelease/20230926_sekiro_salesdata.html)

### 5.4 2022: 대중화의 분수령

`Elden Ring`은 광대한 필드와 높은 탐색 자유도를 결합했고 누적 3,000만 장 이상 판매됐다. 소울라이크가 수백만 장급 하드코어 장르에서 수천만 장급 블록버스터 시장으로 이동한 결정적인 시점이다.

출처: [Elden Ring 3,000만 장 발표](https://www.fromsoftware.jp/jp/pressrelease/20250530_eldenring_nightreign_salesdata.html)

### 5.5 2023~2024: 외부 스튜디오 검증과 문화권 확장

`Lies of P`, `Remnant II`, `Lords of the Fallen`, `Wo Long`이 각각 100만~400만 장 이상의 공개 성과를 냈다. `Another Crab's Treasure`, 이탈리아 민속 기반 `Enotria`, 중국 신화 기반 `Black Myth: Wukong`처럼 미학과 문화권도 넓어졌다.

이 시기부터 유럽 중세 다크 판타지는 유일한 상업 문법이 아니게 됐다.

### 5.6 2025~2026: 장르 혼합 단계

[`Nightreign`](https://www.bandainamcoent.com/games/nightreign/)의 3인 세션형 협동, `Nioh 3`의 오픈 필드와 사무라이·닌자 전환, `The Duskbloods`의 최대 8인 PvPvE처럼 “Dark Souls를 얼마나 잘 복제했는가”보다 **소울식 학습 루프를 무엇과 결합했는가**가 핵심 경쟁 요소가 됐다.

---

## 6. 2026년 출시 지형

2026년 7월 26일 기준으로 확인된 대표 작품만 정리한다.

| 작품 | 상태 | 시장상 의미 |
| --- | --- | --- |
| [Code Vein II](https://www.bandainamcoent.com/news/code-vein-ii-available-now-on-consoles-and-pc) | 1월 30일 출시 | 동료, 넓은 환경, 이동수단을 통한 접근성 |
| [Nioh 3](https://teamninja-studio.com/nioh3/us/index.html) | 2월 6일 출시 | 오픈 필드, 사무라이·닌자 즉시 전환, 최대 3인 |
| [The Relic: First Guardian](https://store.playstation.com/en-us/product/UP3589-PPSA20945_00-0345580252799819) | 7월 31일 예정 | 한국 문화, 공격 스태미나 제거, Relic 중심 성장 |
| [Mortal Shell II](https://mortalshell.com/) | 8월 20일 예정 | 밀도 높은 오픈월드, 스태미나 제거 |
| [The Duskbloods](https://www.fromsoftware.jp/ww/pressrelease/20250402_theduskbloods_debut.html) | 2026년 예정 | FromSoftware의 최대 8인 PvPvE |

`Lords of the Fallen II`는 초기 2026년 일정과 달리 2026년 6월 23일 공식 공시에서 **2027년 1분기**로 연기됐다. 2026년 경쟁작 수에 포함하지 않는다.

출처: [CI Games 일정 변경 공시](https://cigames.com/en/ci-games-se-25-2026-update-on-the-development-schedule-of-lords-of-the-fallen-ii/)

---

## 7. 2026년 핵심 트렌드

### 7.1 협동이 부가 소환에서 제품 구조로 이동

`Remnant II`의 드롭인 3인 협동, `Nightreign`의 3인 반복 세션, `Nioh 3`의 최대 3인, `The Duskbloods`의 8인 PvPvE가 하나의 흐름을 만든다.

시장이 전부 멀티플레이로 전환된 것은 아니다. 전통적 싱글플레이 축과 세션형 멀티플레이 축으로 분화되는 중이다.

### 7.2 다른 장르와의 결합이 차별화 수단

- [`Nightreign`](https://www.bandainamcoent.com/games/nightreign/): 로그라이트형 반복 세션과 변동 맵
- `The Duskbloods`: PvPvE
- [`Valor Mortis`](https://news.xbox.com/en-us/2026/06/07/xbox-games-showcase-2026-recap-everything-announced/): 1인칭 전투
- [`Death Howl`](https://blog.playstation.com/2026/02/16/how-open-world-soulslike-deckbuilder-death-howl-was-built/): 덱빌딩
- [`No Rest for the Wicked`](https://norestforthewicked.com/news/no-rest-for-the-wicked-1-0-launch-month-revealed): 아이소메트릭 ARPG

2026년의 차별화는 난도 자체보다 소울식 학습·실패 루프를 다른 플레이 세션에 이식하는 방식에서 나온다.

### 7.3 거대한 오픈월드보다 밀도 높은 오픈 필드와 빠른 이동

`Elden Ring` 이후 무조건 면적을 키우기보다 탐색 중단 시간을 줄이고 전투 진입 속도를 높이는 방향이 나타난다.

- `Nioh 3`: 점프, 공중 이동, 스타일 즉시 전환
- `Code Vein II`: 동료와 이동수단
- `Mortal Shell II`: 밀도 높은 연결형 오픈월드
- `The Relic`: 공격 스태미나 제거

### 7.4 접근성은 쉬운 게임화보다 선택지 확장

`Lies of P: Overture`는 3단계 난이도, 보스 재도전, 색각 보조 등 접근성 기능을 도입했다. 기본 난이도의 정체성을 유지하면서 진입 장벽을 선택적으로 낮추는 방식이다.

출처: [Lies of P: Overture 공식 패치 노트](https://www.liesofp.com/en-us/news/launch-patch-notes-lies-of-p-overture)

### 7.5 아시아가 제작과 소비 양쪽의 중심으로 이동

Sensor Tower/VGI의 광의 표본에서는 2025년 상위 20개 작품 개발사 중 아시아·태평양 비중이 80%였다. 상위 작품 플레이어 표본에서는 중국이 47%였지만 `Black Myth: Wukong`의 중국 비중에 크게 영향을 받은 값이다.

Newzoo는 중국이 2025년 글로벌 PC 매출 성장의 42%를 기여한 것으로 분석했다.

출처: [Newzoo 중국 PC 시장 분석](https://newzoo.com/articles/chinas-pc-market)

### 7.6 가격은 AA와 AAA의 두 경로로 나뉜다

Newzoo에 따르면 PC 프리미엄 시장에서는 30~50달러 구간이 가장 빠르게 성장한다. 반면 콘솔 프리미엄 매출의 약 80%는 여전히 50달러 이상 제품에서 발생한다.

- 30~50달러: 선명한 전투 아이디어와 제한된 범위를 파는 AA 전략
- 60~80달러: 콘텐츠 규모와 높은 연출·기술 품질을 요구받는 AAA 전략

출처: [Newzoo PC·콘솔 프리미엄 가격대 분석](https://newzoo.com/articles/post-pandemic-growth-pc-console)

### 7.7 신작 판매만큼 카탈로그와 IP 확장이 중요

`Shadow of the Erdtree` DLC 1,000만 장, `Nightreign` 500만 장은 성공한 전투 IP가 본편 이후에도 확장될 수 있음을 보여준다. PC 시장은 콘솔보다 상위 20개 바깥의 장기 꼬리 매출과 플레이타임 비중도 높다.

출처: [Newzoo 상위 20개 바깥의 플레이타임과 매출](https://newzoo.com/articles/playtime-revenue-beyond-top-20)

---

## 8. Maverick에 대한 시사점

이 절은 시장 자료에서 도출한 제품·전투 설계 해석이며 관측 통계가 아니다.

### 8.1 “또 하나의 소울라이크”를 피해야 한다

2025년에만 Steam `Souls-like` 태그 출시작이 381개였다. 장르 문법의 충실한 재현만으로는 제품을 설명하기 어렵다.

Maverick은 다음 전투 문장을 제품 훅으로 검증할 수 있다.

> **간결한 기본공격으로 교전하고, 역할이 분명한 네 스킬로 상황을 풀며, 적극적으로 적중시켜 채운 필살기로 전투를 결산한다.**

### 8.2 현재 S1~S4 구조는 역할 가독성에 강점이 있다

- S1 Tempo: 기본공격과 상호작용하며 자주 순환
- S2 Area: 다수전과 공간 문제 해결
- S3 Response: 기본 회피와 구분되는 받아치기·특수 회피
- S4 Signature: 시간 대기보다 유효 적중으로 준비하는 결산기

이 구조는 “어려운 게임”보다 “전투 리듬이 무엇인 게임인지”를 설명하기 쉽다. 무기별 스킬 내용이 달라도 슬롯 역할과 런타임 생명주기를 통합하기에도 유리하다.

### 8.3 기본공격 단순화는 스킬 순환을 보여주는 방향이어야 한다

약공격과 차지공격만 남기는 결정은 입력 복잡도를 줄이지만, 약공격 연타가 모든 문제를 해결하면 네 스킬의 시장 차별성이 사라진다.

플레이테스트에서는 다음을 확인해야 한다.

- S1 최대 Charge 상태로 대기하는 시간과 분당 사용 횟수
- S2의 사용당 대상 수와 단일 대상 대비 효율
- S3 위협 기회 대비 사용률·성공률과 기본 회피 사용률
- S4 준비까지 걸린 시간, 유효 적중 수와 전투당 사용 횟수
- 약공격·차지공격·스킬별 피해, 그로기, 행동 취소 기여

### 8.4 Perfect Dodge Step은 빠른 전투의 시각적 표지가 될 수 있다

락온 여부와 무관하게 Roll을 기본으로 하고 정확한 타이밍의 회피만 짧은 Step으로 결산하면, 기본 이동 규칙을 늘리지 않고도 숙련 성공을 명확히 보여줄 수 있다.

S3는 이 공용 회피 보상과 경쟁하지 않도록 무기별 반격, 특수 회피, 위치 우위 같은 별도 결과를 가져야 한다.

### 8.5 협동은 추세가 아니라 별도 제품 결정이다

협동과 PvPvE가 성장 중이지만 현재 싱글플레이 전투에 사후 추가하면 다음 비용이 함께 발생한다.

- 적 AI와 위협 선택 재설계
- 레벨·보스 공간과 타깃 전환 재설계
- 네트워크 권한과 적중 판정
- 부활, 진행 공유, 난도 스케일링
- 카메라, 락온, UI와 이펙트 가독성

카타나 3+1 수직 단면과 기본 전투가 검증되기 전에는 시장 유행만으로 협동을 범위에 추가하지 않는다.

### 8.6 접근성은 기본 난이도 외곽에서 먼저 설계할 수 있다

기본 전투의 의도는 유지하면서 다음 선택지를 검토할 수 있다.

- 전체 입력 리매핑과 Tap/Hold 조절
- 색각·명도·위협 표시 옵션
- 튜토리얼 훈련과 보스 패턴 재연습
- 카메라 흔들림과 히트스톱 강도 조절
- 기본 모드와 분리된 타이밍·피해 보조 옵션

### 8.7 가격은 콘텐츠 범위와 함께 결정한다

AA 규모라면 시장 흐름상 30~50달러 구간이 현실적인 후보지만, 가격은 다음 비교 없이 먼저 정하지 않는다.

- 예상 플레이타임과 보스·지역 수
- 전투 애니메이션과 적 재사용률
- 연출·그래픽·성능 완성도
- 출시 플랫폼과 현지화 범위
- 출시 후 콘텐츠와 반복 플레이 구조

---

## 9. 제품 판단 요약

| 시장 관찰 | Maverick의 판단 |
| --- | --- |
| 출시 공급이 이미 많음 | 소울라이크 태그가 아니라 전투 순환과 미학으로 한 문장 차별화 |
| FromSoftware와 소수 IP에 판매 집중 | 넓은 기능보다 첫 30분과 첫 보스의 완성도 우선 |
| 협동·로그라이트 혼합 증가 | 현재 싱글플레이 수직 단면 검증 후 별도 제품 축으로 판단 |
| 빠른 이동과 능동 방어 증가 | Perfect Step과 S3 Response의 역할 차이를 시각적으로 강화 |
| 접근성 선택지 확대 | 기본 난도 밖에서 입력·가독성·훈련 옵션 제공 |
| 아시아 제작·소비 비중 증가 | 한국적 정체성, 중국어권 현지화와 PC 성능을 초기부터 고려 |
| 30~50달러 구간 성장 | AA 범위라면 후보 가격대로 검토하되 콘텐츠 벤치마크 선행 |
| 성공 IP의 긴 수명 | S1~S4 공통 구조를 무기·보스·후속 콘텐츠 확장 기반으로 설계 |

최종적으로 Maverick의 다음 마일스톤은 장르 기능을 넓히는 단계가 아니라 아래 세 가지를 증명하는 단계다.

1. 약공격·차지공격과 S1~S4가 서로 다른 전투 질문에 답한다.
2. 그 순환이 첫 보스 이전에도 플레이어에게 읽힌다.
3. 같은 전투 구조가 무기별 스킬 차이를 수용하면서도 공통 런타임으로 관리된다.
