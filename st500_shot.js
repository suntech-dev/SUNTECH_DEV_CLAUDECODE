const { chromium } = require('playwright');

(async () => {
    const browser = await chromium.launch({ headless: true });
    const page = await browser.newPage();
    await page.setViewportSize({ width: 430, height: 860 });
    const base = 'C:\\SUNTECH_DEV_CLAUDECODE\\.palywright_screen_shot\\';

    // Home
    await page.goto('http://localhost/app/ST500_LOCKMAKER_V1/#/');
    await page.waitForLoadState('networkidle');
    await page.screenshot({ path: base + 'st500_home_new.jpeg', type: 'jpeg', fullPage: true });
    console.log('home done');

    // LockMake
    await page.goto('http://localhost/app/ST500_LOCKMAKER_V1/#/make');
    await page.waitForLoadState('networkidle');
    await page.screenshot({ path: base + 'st500_make_new.jpeg', type: 'jpeg', fullPage: true });
    console.log('make done');

    // Setting
    await page.goto('http://localhost/app/ST500_LOCKMAKER_V1/#/setting');
    await page.waitForLoadState('networkidle');
    await page.screenshot({ path: base + 'st500_setting_new.jpeg', type: 'jpeg', fullPage: true });
    console.log('setting done');

    await browser.close();
})();
