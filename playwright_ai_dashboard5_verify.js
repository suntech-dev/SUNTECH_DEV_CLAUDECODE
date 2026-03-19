const { chromium } = require('playwright');
const path = require('path');

(async () => {
  const browser = await chromium.launch({ headless: true });
  const page = await browser.newPage();
  await page.setViewportSize({ width: 1920, height: 1080 });

  const SHOT_DIR = 'C:\\SUNTECH_DEV_CLAUDECODE\\.playwright_screen_shot';
  const BASE_URL = 'http://localhost/dev/OEE_SCI/OEE_SCI_V2/page/data/ai_dashboard_5.php';

  // API 응답 캡처용
  const apiResponses = {};
  page.on('response', async (resp) => {
    const url = resp.url();
    if (url.includes('ai_oee_prediction_5') || url.includes('ai_maintenance_5') || url.includes('ai_optimization_5')) {
      try {
        const json = await resp.json();
        const key = url.split('/').pop().split('?')[0];
        apiResponses[key] = json;
        console.log('[API]', key, JSON.stringify(json).substring(0, 300));
      } catch(e) {}
    }
  });

  // 페이지 접속
  await page.goto(BASE_URL);
  await page.waitForTimeout(5000);

  // 초기 스크린샷
  await page.screenshot({ path: path.join(SHOT_DIR, 'v5_initial.jpeg'), type: 'jpeg', fullPage: false });

  // 30일 필터 선택
  await page.selectOption('#dateRangeSelect', '30d');
  await page.waitForTimeout(8000);

  // 전체 스크린샷
  await page.screenshot({ path: path.join(SHOT_DIR, 'v5_30d_full.jpeg'), type: 'jpeg', fullPage: false });

  // Row A 클로즈업
  const rowA = page.locator('.ai-signage-row-a');
  await rowA.screenshot({ path: path.join(SHOT_DIR, 'v5_30d_row_a.jpeg'), type: 'jpeg' });

  // Row B 클로즈업
  const rowB = page.locator('.ai-signage-row-b');
  await rowB.screenshot({ path: path.join(SHOT_DIR, 'v5_30d_row_b.jpeg'), type: 'jpeg' });

  // Row C 클로즈업
  const rowC = page.locator('.ai-signage-row-c');
  await rowC.screenshot({ path: path.join(SHOT_DIR, 'v5_30d_row_c.jpeg'), type: 'jpeg' });

  // Row D 클로즈업
  const rowD = page.locator('.ai-signage-row-d');
  await rowD.screenshot({ path: path.join(SHOT_DIR, 'v5_30d_row_d.jpeg'), type: 'jpeg' });

  // DOM 값 수집
  const domValues = await page.evaluate(() => {
    return {
      realtimeOee: document.getElementById('aiRealtimeOee')?.textContent,
      realtimeSub: document.getElementById('aiRealtimeSub')?.textContent,
      realtimeBadge: document.getElementById('aiRealtimeBadge')?.textContent,
      forecastOee: document.getElementById('aiPredForecastOee')?.textContent,
      forecastSub: document.getElementById('aiPredSub')?.textContent,
      anomalyTotal: document.getElementById('aiAnomalyTotal')?.textContent,
      maintDanger: document.getElementById('aiMaintDanger')?.textContent,
      healthAvg: document.getElementById('aiHealthAvg')?.textContent,
      healthSubtitle: document.querySelector('.ai-health-subtitle')?.textContent,
      optSummary: document.getElementById('aiOptSummary')?.textContent?.trim(),
    };
  });

  console.log('\n=== DOM 값 (30d 필터) ===');
  console.log(JSON.stringify(domValues, null, 2));

  console.log('\n=== API 응답 요약 ===');
  console.log(JSON.stringify(apiResponses, null, 2));

  await browser.close();
  console.log('\n완료. 스크린샷 저장됨:', SHOT_DIR);
})();
