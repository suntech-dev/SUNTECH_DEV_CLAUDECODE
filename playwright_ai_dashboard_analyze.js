/**
 * Playwright AI Dashboard 4 분석 스크립트
 * ai_dashboard_4.php — "Last 30 Days" 필터 기준 전체 섹션 검증
 */

const { chromium } = require('playwright');
const path = require('path');
const fs   = require('fs');

const SCREENSHOT_DIR = 'C:\\SUNTECH_DEV_CLAUDECODE\\.playwright_screen_shot';
const BASE_URL       = 'http://localhost/dev/OEE_SCI/OEE_SCI_V2/page/data/ai_dashboard_4.php';

// 스크린샷 저장 폴더 확인
if (!fs.existsSync(SCREENSHOT_DIR)) {
  fs.mkdirSync(SCREENSHOT_DIR, { recursive: true });
}

// API 응답 캡처 저장소
const apiResponses = {};

(async () => {
  const browser = await chromium.launch({
    headless: false,
    args: ['--start-maximized'],
  });

  const context = await browser.newContext({
    viewport: { width: 1920, height: 1080 },
  });

  const page = await context.newPage();

  // ── Network Intercept: AI API 응답 캡처 ──────────────────────
  page.on('response', async (response) => {
    const url = response.url();
    const aiApis = [
      'ai_oee_prediction.php',
      'ai_anomaly.php',
      'ai_maintenance.php',
      'ai_optimization.php',
      'ai_stream_analysis.php',
    ];
    for (const api of aiApis) {
      if (url.includes(api)) {
        try {
          const json = await response.json().catch(() => null);
          if (json) {
            apiResponses[api] = json;
            console.log(`\n[API 응답] ${api}`);
            console.log(JSON.stringify(json, null, 2).substring(0, 1500));
          }
        } catch (_) {}
      }
    }
  });

  // ── 1. 페이지 접속 ────────────────────────────────────────────
  console.log('\n[단계 1] 페이지 접속 중...');
  await page.goto(BASE_URL, { waitUntil: 'domcontentloaded' });

  // 5초 대기 (초기 로드)
  console.log('[단계 1] 초기 로드 대기 5초...');
  await page.waitForTimeout(5000);

  // ── 2. 초기 상태 스크린샷 ─────────────────────────────────────
  console.log('[단계 2] 초기 상태 스크린샷 촬영...');
  await page.screenshot({
    path: path.join(SCREENSHOT_DIR, 'ai_dashboard4_initial.jpeg'),
    type: 'jpeg',
    quality: 90,
    fullPage: true,
  });
  console.log('  -> ai_dashboard4_initial.jpeg 저장 완료');

  // ── 3. dateRangeSelect → "30d" 변경 ──────────────────────────
  console.log('\n[단계 3] 날짜 필터 "Last 30 Days" (30d) 로 변경...');
  await page.selectOption('#dateRangeSelect', '30d');
  console.log('  -> 30d 선택 완료, 8초 대기 (API 응답 대기)...');
  await page.waitForTimeout(8000);

  // ── 4. 30일 필터 전체 스크린샷 ───────────────────────────────
  console.log('\n[단계 4] 30일 필터 전체 스크린샷 촬영...');
  await page.screenshot({
    path: path.join(SCREENSHOT_DIR, 'ai_dashboard4_30d_full.jpeg'),
    type: 'jpeg',
    quality: 90,
    fullPage: true,
  });
  console.log('  -> ai_dashboard4_30d_full.jpeg 저장 완료');

  // ── 5. Row A 클로즈업 ─────────────────────────────────────────
  console.log('\n[단계 5] Row A (Summary 카드 5개) 클로즈업...');
  const rowA = await page.$('.ai-signage-row-a');
  if (rowA) {
    await rowA.screenshot({
      path: path.join(SCREENSHOT_DIR, 'ai_dashboard4_30d_row_a.jpeg'),
      type: 'jpeg',
      quality: 90,
    });
    console.log('  -> ai_dashboard4_30d_row_a.jpeg 저장 완료');
  } else {
    console.log('  [경고] .ai-signage-row-a 요소를 찾지 못했습니다.');
  }

  // ── 6. Row B 클로즈업 ─────────────────────────────────────────
  console.log('\n[단계 6] Row B (OEE Forecast + Anomaly) 클로즈업...');
  const rowB = await page.$('.ai-signage-row-b');
  if (rowB) {
    await rowB.screenshot({
      path: path.join(SCREENSHOT_DIR, 'ai_dashboard4_30d_row_b.jpeg'),
      type: 'jpeg',
      quality: 90,
    });
    console.log('  -> ai_dashboard4_30d_row_b.jpeg 저장 완료');
  } else {
    console.log('  [경고] .ai-signage-row-b 요소를 찾지 못했습니다.');
  }

  // ── 7. Row C 클로즈업 ─────────────────────────────────────────
  console.log('\n[단계 7] Row C (Line Health + Maintenance) 클로즈업...');
  const rowC = await page.$('.ai-signage-row-c');
  if (rowC) {
    await rowC.screenshot({
      path: path.join(SCREENSHOT_DIR, 'ai_dashboard4_30d_row_c.jpeg'),
      type: 'jpeg',
      quality: 90,
    });
    console.log('  -> ai_dashboard4_30d_row_c.jpeg 저장 완료');
  } else {
    console.log('  [경고] .ai-signage-row-c 요소를 찾지 못했습니다.');
  }

  // ── 8. Row D 클로즈업 ─────────────────────────────────────────
  console.log('\n[단계 8] Row D (AI Stream + Optimization) 클로즈업...');
  const rowD = await page.$('.ai-signage-row-d');
  if (rowD) {
    await rowD.screenshot({
      path: path.join(SCREENSHOT_DIR, 'ai_dashboard4_30d_row_d.jpeg'),
      type: 'jpeg',
      quality: 90,
    });
    console.log('  -> ai_dashboard4_30d_row_d.jpeg 저장 완료');
  } else {
    console.log('  [경고] .ai-signage-row-d 요소를 찾지 못했습니다.');
  }

  // ── 9. DOM 값 검사 ───────────────────────────────────────────
  console.log('\n[단계 9] DOM 요소 값 검사...');

  const domValues = await page.evaluate(() => {
    const getText = (id) => {
      const el = document.getElementById(id);
      return el ? el.textContent.trim() : '[not found]';
    };
    return {
      aiRealtimeOee:     getText('aiRealtimeOee'),
      aiRealtimeSub:     getText('aiRealtimeSub'),
      aiRealtimeBadge:   getText('aiRealtimeBadge'),
      aiPredForecastOee: getText('aiPredForecastOee'),
      aiPredSub:         getText('aiPredSub'),
      aiPredTrendBadge:  getText('aiPredTrendBadge'),
      aiAnomalyTotal:    getText('aiAnomalyTotal'),
      aiAnomalySub:      getText('aiAnomalySub'),
      aiMaintDanger:     getText('aiMaintDanger'),
      aiMaintSub:        getText('aiMaintSub'),
      aiHealthAvg:       getText('aiHealthAvg'),
      aiHealthSub:       getText('aiHealthSub'),
      aiStreamStatus:    getText('aiStreamStatus'),
      aiStreamCount:     getText('aiStreamCount'),
      aiLastUpdateTime:  getText('aiLastUpdateTime'),
      dateRangeValue:    document.getElementById('dateRangeSelect')?.value || '[not found]',
    };
  });

  console.log('\n=== DOM 값 ===');
  Object.entries(domValues).forEach(([k, v]) => {
    console.log(`  ${k}: "${v}"`);
  });

  // ── 10. API 응답 최종 요약 ────────────────────────────────────
  console.log('\n=== 캡처된 API 응답 키 ===');
  Object.keys(apiResponses).forEach(api => {
    const d = apiResponses[api];
    console.log(`\n[${api}]`);
    if (d.code !== undefined) console.log(`  code: ${d.code}`);
    if (d.current_oee !== undefined) console.log(`  current_oee: ${d.current_oee}`);
    if (d.current_hour !== undefined) console.log(`  current_hour: ${d.current_hour}`);
    if (d.trend !== undefined) console.log(`  trend: ${d.trend}`);
    if (d.forecast) console.log(`  forecast 개수: ${d.forecast.length}`);
    if (d.summary) console.log(`  summary: ${JSON.stringify(d.summary)}`);
    if (d.machines) console.log(`  machines 개수: ${d.machines.length}`);
    if (d.opportunities) console.log(`  opportunities 개수: ${d.opportunities.length}`);
    if (d.anomalies) console.log(`  anomalies 개수: ${d.anomalies.length}`);
  });

  // ── 11. 스크린샷 목록 확인 ────────────────────────────────────
  const screenshots = fs.readdirSync(SCREENSHOT_DIR).filter(f => f.endsWith('.jpeg'));
  console.log('\n=== 저장된 스크린샷 목록 ===');
  screenshots.forEach(f => {
    const size = fs.statSync(path.join(SCREENSHOT_DIR, f)).size;
    console.log(`  ${f} (${Math.round(size/1024)}KB)`);
  });

  await browser.close();
  console.log('\n[완료] 브라우저 종료. Playwright 분석 완료.');
})();
