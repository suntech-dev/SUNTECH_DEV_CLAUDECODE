<?php

/*
 * ============================================================
 * 파일명  : ai_dashboard_2_1_ina.php
 * 목  적  : 1920×1080 사이니지 AI 인텔리전스 대시보드 v2_1 (인도네시아어)
 *           ai_dashboard_2_ina.php 기반 — Row D 변경:
 *             Streaming + Production Optimization 제거
 *             → Downtime Top5 (지속시간) + Defective Top5 (건수) 실시간 차트
 * 연관 파일:
 *   - css/ai_dashboard_2_1.css            : Row D 레이아웃 재정의
 *   - js/ai_dashboard_2_ina.js            : AI 카드·차트 렌더링 (INA 공통)
 *   - js/ai_downtime_defective_2_1.js     : Downtime/Defective Top5 차트
 *   - proc/downtime_defective_top5.php    : Top5 데이터 API
 * ============================================================
 */

$page_title = 'AI Intelligence Dashboard v2.1 - Signage (INA)';

$page_css_files = [
    '../../assets/css/fiori-page.css',
    'css/dashboard.css',
    'css/ai_dashboard.css',
    'css/ai_dashboard_2_1.css',
];

require_once(__DIR__ . '/../../inc/head.php');
?>

<?php
$nav_context = 'data';
$nav_active  = 'ai_dashboard_ina';
require_once(__DIR__ . '/../../inc/nav-drawer-manage.php'); ?>

<!-- Signage Header -->
<div class="signage-header">
    <button id="navDrawerBtn" class="nav-drawer-btn" aria-label="Menu">&#9776;</button>
    <span class="signage-header__title">
        AI Intelligence Dashboard
        <span class="ai-badge">AI POWERED</span>
        <span class="ina-badge">INA</span>
    </span>

    <div class="signage-header__filters">
        <select id="factoryFilterSelect" class="fiori-select">
            <option value="">Semua Pabrik</option>
        </select>
        <select id="factoryLineFilterSelect" class="fiori-select" disabled>
            <option value="">Semua Line</option>
        </select>
        <select id="factoryLineMachineFilterSelect" class="fiori-select" disabled>
            <option value="">Semua Mesin</option>
        </select>
        <select id="dateRangeSelect" class="fiori-select date-range-select">
            <option value="today" selected>Hari Ini</option>
            <option value="yesterday">Kemarin</option>
            <option value="7d">7 Hari Terakhir</option>
            <option value="30d">30 Hari Terakhir</option>
        </select>
        <button id="aiRefreshBtn" class="fiori-btn fiori-btn--tertiary">Segarkan</button>
        <button id="exportReportBtn" class="fiori-btn fiori-btn--ghost">Ekspor</button>
        <a href="ai_dashboard_manual.html" target="_blank" class="fiori-btn fiori-btn--ghost" style="text-decoration:none;">Bantuan</a>
    </div>

    <img src="../../assets/images/logo_suntech.png" alt="SunTech" style="height:18px;width:auto;flex-shrink:0;opacity:0.85;">

    <div class="signage-header__status">
        <div class="ai-pulse-dot"></div>
        <span id="aiLastUpdateTime">Menginisialisasi...</span>
    </div>

    <div class="signage-header__clock" id="signageClock"></div>
</div>

<!-- AI Signage Main: 4행 CSS Grid -->
<div class="ai-signage-main">

    <!-- Row A: AI Summary 카드 5개 -->
    <div class="ai-signage-row-a">
        <div class="ai-summary-grid">

            <div class="ai-summary-card ai-summary-card--realtime">
                <div>
                    <div class="ai-realtime-live">
                        <div class="ai-realtime-live__dot"></div>
                        LANGSUNG
                    </div>
                    <span class="ai-summary-card__label" style="margin-top:2px;">OEE Real-time</span>
                </div>
                <div class="ai-summary-card__value" id="aiRealtimeOee">--</div>
                <div class="ai-summary-card__sub-row">
                    <div class="ai-summary-card__sub" id="aiRealtimeSub"><span class="ai-spinner"></span></div>
                    <span id="aiRealtimeBadge" class="ai-status-badge" style="display:none;"></span>
                </div>
            </div>

            <div class="ai-summary-card ai-summary-card--prediction">
                <span class="ai-summary-card__label">Prediksi AI 4 Jam ke Depan</span>
                <div class="ai-summary-card__value" id="aiPredForecastOee">--</div>
                <div class="ai-summary-card__sub-row">
                    <div class="ai-summary-card__sub" id="aiPredSub"><span class="ai-spinner"></span></div>
                    <span class="ai-trend-badge ai-trend-badge--stable" id="aiPredTrendBadge">--</span>
                </div>
            </div>

            <div class="ai-summary-card ai-summary-card--anomaly">
                <span class="ai-summary-card__label">Deteksi Anomali</span>
                <div class="ai-summary-card__value" id="aiAnomalyTotal">--</div>
                <div class="ai-summary-card__sub-row">
                    <div class="ai-summary-card__sub" id="aiAnomalySub"><span class="ai-spinner"></span></div>
                    <span id="aiAnomalyCriticalBadge" class="ai-status-badge" style="display:none;"></span>
                </div>
            </div>

            <div class="ai-summary-card ai-summary-card--maintenance">
                <span class="ai-summary-card__label">Mesin Berisiko Tinggi</span>
                <div class="ai-summary-card__value" id="aiMaintDanger">--</div>
                <div class="ai-summary-card__sub-row">
                    <div class="ai-summary-card__sub" id="aiMaintSub"><span class="ai-spinner"></span></div>
                    <span id="aiMaintWarnBadge" class="ai-status-badge ai-status-badge--warning" style="display:none;"></span>
                </div>
            </div>

            <div class="ai-summary-card ai-summary-card--health">
                <span class="ai-summary-card__label">Indeks Kesehatan Line (Rata-rata)</span>
                <div class="ai-summary-card__value" id="aiHealthAvg">--</div>
                <div class="ai-summary-card__sub-row">
                    <div class="ai-summary-card__sub" id="aiHealthSub"><span class="ai-spinner"></span></div>
                    <span id="aiHealthStatusBadge" class="ai-status-badge" style="display:none;"></span>
                </div>
            </div>

        </div>
    </div><!-- /ai-signage-row-a -->

    <!-- Row B: OEE Forecast (2fr) + Anomaly Detection (1fr) -->
    <div class="ai-signage-row-b">

        <div class="fiori-card">
            <div class="fiori-card__header">
                <div class="card-title-row">
                    <h3 class="fiori-card__title fiori-text-primary">Tren OEE &amp; Prediksi AI</h3>
                    <span class="card-subtitle-inline fiori-text-secondary">Garis Penuh = Aktual &nbsp;/&nbsp; Garis Putus = Prediksi AI (90% CI)</span>
                </div>
            </div>
            <div class="fiori-card__content">
                <div class="ai-prediction-chart-wrap">
                    <canvas id="aiOeeForecastChart"></canvas>
                </div>
                <div class="ai-chart-legend">
                    <div class="ai-chart-legend__item">
                        <div class="ai-chart-legend__dot ai-chart-legend__dot--actual"></div>
                        <span>OEE Aktual</span>
                    </div>
                    <div class="ai-chart-legend__item">
                        <div class="ai-chart-legend__dot ai-chart-legend__dot--forecast"></div>
                        <span>Prediksi AI</span>
                    </div>
                    <div class="ai-chart-legend__item">
                        <div class="ai-chart-legend__dot ai-chart-legend__dot--ci" style="width:20px;height:8px;border-radius:2px;"></div>
                        <span>Interval Kepercayaan (90%)</span>
                    </div>
                </div>
            </div>
        </div>

        <div class="fiori-card">
            <div class="fiori-card__header">
                <div class="card-title-row">
                    <h3 class="fiori-card__title fiori-text-primary">Deteksi Anomali</h3>
                    <span class="card-subtitle-inline fiori-text-secondary">Deteksi real-time berbasis Z-Score</span>
                </div>
                <div id="aiAnomalyHeaderCount" class="ai-last-update" style="display:none;">
                    <div class="ai-pulse-dot" style="background:var(--sap-negative);"></div>
                    <span id="aiAnomalyHeaderText"></span>
                </div>
            </div>
            <div class="fiori-card__content">
                <div class="ai-anomaly-list" id="aiAnomalyList">
                    <div class="ai-empty-state">
                        <div class="ai-spinner"></div>
                        <span>Menganalisis anomali...</span>
                    </div>
                </div>
            </div>
        </div>

    </div><!-- /ai-signage-row-b -->

    <!-- Row C: Line Health (1fr) + Predictive Maintenance (1fr) -->
    <div class="ai-signage-row-c">

        <div class="fiori-card">
            <div class="fiori-card__header">
                <div class="card-title-row">
                    <h3 class="fiori-card__title fiori-text-primary">Indeks Kesehatan Line</h3>
                    <span class="card-subtitle-inline ai-health-subtitle">Berdasarkan rata-rata OEE 7 hari terakhir</span>
                </div>
            </div>
            <div class="fiori-card__content">
                <div class="ai-health-list" id="aiHealthList">
                    <div class="ai-empty-state">
                        <div class="ai-spinner"></div>
                        <span>Menghitung indeks kesehatan...</span>
                    </div>
                </div>
            </div>
        </div>

        <div class="fiori-card">
            <div class="fiori-card__header">
                <div class="card-title-row">
                    <h3 class="fiori-card__title fiori-text-primary">Predictive Maintenance</h3>
                    <span class="card-subtitle-inline fiori-text-secondary">Urutan mesin berdasarkan tingkat risiko kerusakan</span>
                </div>
            </div>
            <div class="fiori-card__content">
                <div class="ai-maintenance-list" id="aiMaintenanceList">
                    <div class="ai-empty-state">
                        <div class="ai-spinner"></div>
                        <span>Menghitung skor risiko...</span>
                    </div>
                </div>
            </div>
        </div>

    </div><!-- /ai-signage-row-c -->

    <!-- Row D: Streaming (1fr) + Downtime Top5 (1fr) + Defective Top5 (1fr) -->
    <div class="ai-signage-row-d">

        <!-- Streaming AI Real-time -->
        <div class="fiori-card">
            <div class="fiori-card__header">
                <div class="card-title-row">
                    <h3 class="fiori-card__title fiori-text-primary">
                        Streaming AI Real-time
                        <span class="ai-badge">LANGSUNG</span>
                    </h3>
                    <span class="card-subtitle-inline fiori-text-secondary">Kejadian anomali, downtime, risiko perawatan</span>
                </div>
                <div class="ai-last-update">
                    <div class="ai-pulse-dot" id="aiStreamDot" style="background:#e67e22;"></div>
                    <span id="aiStreamStatus">Menghubungkan...</span>
                    <span id="aiStreamCount" style="font-size:0.75rem;color:var(--sap-text-secondary);margin-left:6px;">0 kejadian</span>
                </div>
            </div>
            <div class="fiori-card__content">
                <div id="aiStreamFeed" class="ai-stream-feed">
                    <div class="ai-stream-empty">
                        <span class="ai-spinner"></span> Menghubungkan ke aliran AI...
                    </div>
                </div>
            </div>
        </div>

        <!-- Downtime Top 5 -->
        <div class="fiori-card">
            <div class="fiori-card__header">
                <div class="card-title-row">
                    <h3 class="fiori-card__title fiori-text-primary">Downtime Top 5</h3>
                    <span class="card-subtitle-inline fiori-text-secondary">Berdasarkan durasi (menit) · periode filter</span>
                </div>
            </div>
            <div class="fiori-card__content">
                <div class="ai-top5-chart-wrap">
                    <canvas id="aiDtTop5Chart"></canvas>
                </div>
            </div>
        </div>

        <!-- Defective Top 5 -->
        <div class="fiori-card">
            <div class="fiori-card__header">
                <div class="card-title-row">
                    <h3 class="fiori-card__title fiori-text-primary">Cacat Top 5</h3>
                    <span class="card-subtitle-inline fiori-text-secondary">Berdasarkan jumlah kejadian · periode filter</span>
                </div>
            </div>
            <div class="fiori-card__content">
                <div class="ai-top5-chart-wrap">
                    <canvas id="aiDefTop5Chart"></canvas>
                </div>
            </div>
        </div>

    </div><!-- /ai-signage-row-d -->

</div><!-- /ai-signage-main -->

<!-- JavaScript Libraries -->
<script src="../../assets/js/chart.js"></script>
<script src="../../assets/js/jquery-3.6.1.min.js"></script>
<script src="../../assets/js/moment.min.js"></script>
<script src="../../assets/js/common.js"></script>

<!-- AI Dashboard JS (인도네시아어) -->
<script src="js/ai_dashboard_2_ina.js"></script>
<script src="js/ai_stream_monitor_2_ina.js"></script>
<script src="js/ai_downtime_defective_2_1.js?v=<?php echo filemtime(__DIR__.'/js/ai_downtime_defective_2_1.js'); ?>"></script>

<!-- 날짜 필터 getFilterParams() 확장 -->
<script>
(function () {
    var _orig = getFilterParams;
    getFilterParams = function () {
        var p = _orig();
        var sel = document.getElementById('dateRangeSelect');
        if (sel && sel.value) p.date_range = sel.value;
        return p;
    };
    document.getElementById('dateRangeSelect').addEventListener('change', function () {
        if (typeof refreshAll === 'function') refreshAll();
    });
})();
</script>

<!-- loadPrediction() 오버라이드 -->
<script>
(function () {
    loadPrediction = function () {
        $.getJSON('proc/ai_oee_prediction_dash_2.php', getFilterParams(), function (data) {
            if (data.code !== '00') {
                $('#aiRealtimeSub').text('Kesalahan API');
                $('#aiPredSub').text('Kesalahan API');
                return;
            }

            var curOee = (data.today_realtime_oee !== null && data.today_realtime_oee !== undefined)
                ? parseFloat(data.today_realtime_oee) : null;

            if (curOee !== null) {
                var colorClass = curOee >= 85 ? 'ai-oee-good' : (curOee >= 60 ? 'ai-oee-warning' : 'ai-oee-danger');
                $('#aiRealtimeOee').text(curOee + '%').removeClass('ai-oee-good ai-oee-warning ai-oee-danger').addClass(colorClass);
                var badgeText = curOee >= 85 ? 'BAIK' : (curOee >= 60 ? 'PERINGATAN' : 'KRITIS');
                var badgeCls  = curOee >= 85 ? 'ai-status-badge--normal' : (curOee >= 60 ? 'ai-status-badge--warning' : 'ai-status-badge--danger');
                $('#aiRealtimeBadge').show().text(badgeText).removeClass('ai-status-badge--normal ai-status-badge--warning ai-status-badge--danger').addClass(badgeCls);
            } else {
                $('#aiRealtimeOee').text('--').removeClass('ai-oee-good ai-oee-warning ai-oee-danger');
                $('#aiRealtimeBadge').hide();
            }
            $('#aiRealtimeSub').text('Hari Ini · Langsung');

            if (data.forecast && data.forecast.length > 0) {
                var sum = 0;
                data.forecast.forEach(function (f) { sum += parseFloat(f.oee); });
                var avg = (sum / data.forecast.length).toFixed(1);
                var ciMin = Math.min.apply(null, data.forecast.map(function (f) { return parseFloat(f.lower); })).toFixed(1);
                var ciMax = Math.max.apply(null, data.forecast.map(function (f) { return parseFloat(f.upper); })).toFixed(1);
                $('#aiPredForecastOee').text(avg + '%');
                $('#aiPredSub').text('CI: ' + ciMin + '% ~ ' + ciMax + '%');
            } else {
                $('#aiPredForecastOee').text('--');
                $('#aiPredSub').text('Data tidak mencukupi');
            }

            var trendMap = {
                up:     { cls: 'ai-trend-badge--up',     text: 'Tren Naik' },
                down:   { cls: 'ai-trend-badge--down',   text: 'Tren Turun' },
                stable: { cls: 'ai-trend-badge--stable', text: 'Stabil' },
            };
            var trend = trendMap[data.trend] || trendMap.stable;
            $('#aiPredTrendBadge').removeClass('ai-trend-badge--up ai-trend-badge--down ai-trend-badge--stable').addClass(trend.cls).text(trend.text);

            renderForecastChart(data);
            loadLineHealth();
        }).fail(function () {
            $('#aiRealtimeSub').text('Kesalahan API');
            $('#aiPredSub').text('Kesalahan API');
        });
    };
})();
</script>

<!-- Export Report Modal (인도네시아어) -->
<div id="exportModal" style="display:none;position:fixed;inset:0;z-index:9999;background:rgba(0,0,0,.6);align-items:center;justify-content:center;">
    <div style="background:#161b22;border:1px solid #30363d;border-radius:10px;padding:24px 28px;min-width:340px;box-shadow:0 8px 32px #000a;">
        <div style="font-size:1rem;font-weight:600;color:#58a6ff;margin-bottom:12px;">Ekspor Laporan — Pilih Periode</div>
        <div style="display:flex;gap:6px;margin-bottom:16px;align-items:center;">
            <span style="font-size:.8rem;color:#8b949e;margin-right:4px;">Bahasa:</span>
            <button class="export-lang fiori-btn fiori-btn--emphasized" data-lang="en" style="font-size:.8rem;padding:3px 12px;">ENG</button>
            <button class="export-lang fiori-btn fiori-btn--tertiary"   data-lang="ko" style="font-size:.8rem;padding:3px 12px;">KOR</button>
        </div>
        <div style="display:flex;gap:8px;margin-bottom:16px;flex-wrap:wrap;">
            <button class="export-preset fiori-btn fiori-btn--tertiary" data-range="today">Hari Ini</button>
            <button class="export-preset fiori-btn fiori-btn--tertiary" data-range="yesterday">Kemarin</button>
            <button class="export-preset fiori-btn fiori-btn--tertiary" data-range="1w">7 Hari Terakhir</button>
            <button class="export-preset fiori-btn fiori-btn--tertiary" data-range="1m">30 Hari Terakhir</button>
        </div>
        <div style="font-size:.8rem;color:#8b949e;margin-bottom:6px;">Rentang Kustom</div>
        <div style="display:flex;gap:8px;align-items:center;margin-bottom:20px;">
            <input type="date" id="exportDateFrom" class="fiori-input" style="flex:1;background:#21262d;color:#e6edf3;border:1px solid #30363d;border-radius:4px;padding:5px 8px;">
            <span style="color:#8b949e;">~</span>
            <input type="date" id="exportDateTo" class="fiori-input" style="flex:1;background:#21262d;color:#e6edf3;border:1px solid #30363d;border-radius:4px;padding:5px 8px;">
        </div>
        <div style="display:flex;gap:8px;justify-content:flex-end;">
            <button id="exportCancelBtn" class="fiori-btn fiori-btn--ghost">Batal</button>
            <button id="exportConfirmBtn" class="fiori-btn fiori-btn--primary">Ekspor</button>
        </div>
    </div>
</div>

<script>
(function () {
    var modal = document.getElementById('exportModal');
    function fmtDate(d) { return d.toISOString().slice(0, 10); }
    function calcRange(range) {
        var now = new Date(), ms = 86400000;
        var map = {
            today:     { from: fmtDate(now), to: fmtDate(now) },
            yesterday: { from: fmtDate(new Date(now - ms)), to: fmtDate(new Date(now - ms)) },
            '1w':      { from: fmtDate(new Date(now - 6 * ms)), to: fmtDate(now) },
            '1m':      { from: fmtDate(new Date(now - 29 * ms)), to: fmtDate(now) },
        };
        return map[range] || map['today'];
    }
    function setPreset(range) {
        var r = calcRange(range);
        document.getElementById('exportDateFrom').value = r.from;
        document.getElementById('exportDateTo').value   = r.to;
        document.querySelectorAll('.export-preset').forEach(function (b) {
            b.classList.toggle('fiori-btn--emphasized', b.dataset.range === range);
            b.classList.toggle('fiori-btn--tertiary',   b.dataset.range !== range);
        });
    }
    document.querySelectorAll('.export-lang').forEach(function (btn) {
        btn.addEventListener('click', function () {
            document.querySelectorAll('.export-lang').forEach(function (b) {
                b.classList.remove('fiori-btn--emphasized'); b.classList.add('fiori-btn--tertiary');
            });
            this.classList.remove('fiori-btn--tertiary'); this.classList.add('fiori-btn--emphasized');
        });
    });
    document.getElementById('exportReportBtn').addEventListener('click', function () { setPreset('today'); modal.style.display = 'flex'; });
    document.querySelectorAll('.export-preset').forEach(function (btn) {
        btn.addEventListener('click', function () { setPreset(this.dataset.range); });
    });
    ['exportDateFrom', 'exportDateTo'].forEach(function (id) {
        document.getElementById(id).addEventListener('change', function () {
            document.querySelectorAll('.export-preset').forEach(function (b) {
                b.classList.remove('fiori-btn--emphasized'); b.classList.add('fiori-btn--tertiary');
            });
        });
    });
    document.getElementById('exportConfirmBtn').addEventListener('click', function () {
        var from = document.getElementById('exportDateFrom').value;
        var to   = document.getElementById('exportDateTo').value;
        if (!from || !to) { alert('Silakan pilih rentang tanggal.'); return; }
        var p = getFilterParams();
        p.range = 'custom'; p.date_from = from; p.date_to = to;
        var activeLang = document.querySelector('.export-lang.fiori-btn--emphasized');
        p.lang = activeLang ? activeLang.dataset.lang : 'en';
        window.open('proc/ai_report_export_2.php?' + new URLSearchParams(p), '_blank');
        modal.style.display = 'none';
    });
    document.getElementById('exportCancelBtn').addEventListener('click', function () { modal.style.display = 'none'; });
    modal.addEventListener('click', function (e) { if (e.target === modal) modal.style.display = 'none'; });
})();
</script>

<!-- Jam Real-time -->
<script>
(function () {
    function updateClock() {
        var now = new Date();
        var h = String(now.getHours()).padStart(2, '0');
        var m = String(now.getMinutes()).padStart(2, '0');
        var s = String(now.getSeconds()).padStart(2, '0');
        var el = document.getElementById('signageClock');
        if (el) el.textContent = h + ':' + m + ':' + s;
    }
    updateClock();
    setInterval(updateClock, 1000);
})();
</script>

</body>
</html>
