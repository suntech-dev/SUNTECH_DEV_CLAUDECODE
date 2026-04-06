// 실행: node shot_report_eng2.js (C:\SUNTECH_DEV_CLAUDECODE\ 에서)
const { chromium } = require('playwright');

(async () => {
    const browser = await chromium.launch({ headless: true });
    const page = await browser.newPage();
    await page.setViewportSize({ width: 1920, height: 1080 });

    await page.goto('http://localhost/dev/OEE_SCI/OEE_SCI_V2/doc/OEE_SCI_V2_REDESIGN_REPORT_ENG_2.html');
    await page.waitForLoadState('networkidle');

    const ts = Date.now();
    const path1 = `C:\\SUNTECH_DEV_CLAUDECODE\\.palywright_screen_shot\\report_eng2_hero_${ts}.jpeg`;
    await page.screenshot({ path: path1, type: 'jpeg' });
    console.log('Hero:', path1);

    // IoT Firmware 섹션으로 스크롤 후 캡처
    await page.evaluate(() => {
        const el = document.getElementById('firmware');
        if (el) el.scrollIntoView({ behavior: 'instant' });
    });
    await page.waitForTimeout(500);
    const path2 = `C:\\SUNTECH_DEV_CLAUDECODE\\.palywright_screen_shot\\report_eng2_firmware_${ts}.jpeg`;
    await page.screenshot({ path: path2, type: 'jpeg' });
    console.log('Firmware:', path2);

    await browser.close();
})();
