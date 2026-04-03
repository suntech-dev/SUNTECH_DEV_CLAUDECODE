<?php
// Embroidery Data Log — data_embroidery 테이블 뷰어
require_once(__DIR__ . '/lib/db.php');

// ── Ajax 엔드포인트 ─────────────────────────────────────────────────────────
if (isset($_GET['ajax'])) {
    header('Content-Type: application/json; charset=utf-8');

    $machineNo = trim($_GET['machine_no'] ?? '');
    $mac       = trim($_GET['mac']        ?? '');
    $range     = trim($_GET['range']      ?? 'today');
    $page      = max(1, (int)($_GET['page'] ?? 1));
    $limit     = 50;
    $offset    = ($page - 1) * $limit;

    // 날짜 범위 계산
    switch ($range) {
        case 'yesterday':
            $dateWhere = "DATE(de.created_at) = DATE_SUB(CURDATE(), INTERVAL 1 DAY)";
            break;
        case '1w':
            $dateWhere = "de.created_at >= DATE_SUB(NOW(), INTERVAL 7 DAY)";
            break;
        default:
            $dateWhere = "DATE(de.created_at) = CURDATE()";
            break;
    }

    $join   = " LEFT JOIN info_machine im ON de.mac = im.mac";
    $where  = " WHERE $dateWhere";
    $params = [];

    if ($machineNo !== '') {
        $where   .= " AND im.machine_no = ?";
        $params[] = $machineNo;
    }
    if ($mac !== '') {
        $where   .= " AND de.mac = ?";
        $params[] = $mac;
    }

    $from = "FROM data_embroidery de$join$where";

    // 전체 건수
    $stmt = $pdo->prepare("SELECT COUNT(*) $from");
    $stmt->execute($params);
    $total = (int)$stmt->fetchColumn();

    // 통계
    $stmt = $pdo->prepare(
        "SELECT SUM(de.ct) AS total_ct, SUM(de.mrt) AS total_mrt, SUM(de.tb) AS total_tb, SUM(de.actual_qty) AS total_actual
         $from"
    );
    $stmt->execute($params);
    $stats = $stmt->fetch(PDO::FETCH_ASSOC);

    // 데이터
    $stmt = $pdo->prepare(
        "SELECT de.idx, de.mac, im.machine_no, de.actual_qty, de.ct, de.tb, de.mrt, de.created_at
         $from
         ORDER BY de.idx DESC
         LIMIT $limit OFFSET $offset"
    );
    $stmt->execute($params);
    $rows = $stmt->fetchAll(PDO::FETCH_ASSOC);

    echo json_encode([
        'total'     => $total,
        'page'      => $page,
        'per_page'  => $limit,
        'stats'     => [
            'total_ct'        => $stats['total_ct']  !== null ? round((float)$stats['total_ct'],  1) : null,
            'total_mrt'       => $stats['total_mrt'] !== null ? round((float)$stats['total_mrt'], 1) : null,
            'total_tb'      => (int)($stats['total_tb'] ?? 0),
            'total_actual'  => (int)($stats['total_actual'] ?? 0),
        ],
        'rows'      => $rows,
    ], JSON_UNESCAPED_UNICODE);
    exit;
}

// Machine No 목록 (필터용)
$stmt = $pdo->query("SELECT DISTINCT machine_no FROM info_machine WHERE machine_no IS NOT NULL ORDER BY machine_no");
$machineNoList = $stmt->fetchAll(PDO::FETCH_COLUMN);

// MAC 목록 (필터용)
$stmt = $pdo->query("SELECT DISTINCT mac FROM data_embroidery ORDER BY mac");
$macList = $stmt->fetchAll(PDO::FETCH_COLUMN);
?>
<!DOCTYPE html>
<html lang="ko">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Embroidery Data Log - SunTech</title>
    <style>
        *,
        *::before,
        *::after {
            box-sizing: border-box;
            margin: 0;
            padding: 0;
        }

        :root {
            --bg: #0d1117;
            --surface: #161b22;
            --surface2: #21262d;
            --border: #30363d;
            --text: #e6edf3;
            --muted: #8b949e;
            --accent: #58a6ff;
            --green: #3fb950;
            --warn: #d29922;
            --danger: #f85149;
            --radius: 6px;
        }

        body {
            background: var(--bg);
            color: var(--text);
            font-family: '72', 'Helvetica Neue', Arial, sans-serif;
            font-size: 14px;
            min-height: 100vh;
        }

        /* ── Header ── */
        .header {
            background: var(--surface);
            border-bottom: 1px solid var(--border);
            padding: 0 24px;
            height: 48px;
            display: flex;
            align-items: center;
            justify-content: space-between;
        }

        .header h1 {
            font-size: 16px;
            font-weight: 600;
            color: var(--text);
        }

        .header-meta {
            font-size: 12px;
            color: var(--muted);
        }

        /* ── Filter Bar ── */
        .filter-bar {
            background: var(--surface);
            border-bottom: 1px solid var(--border);
            padding: 10px 24px;
            display: flex;
            flex-wrap: wrap;
            gap: 8px;
            align-items: center;
        }

        .filter-bar select,
        .filter-bar input {
            background: var(--surface2);
            color: var(--text);
            border: 1px solid var(--border);
            border-radius: var(--radius);
            padding: 5px 10px;
            font-size: 13px;
            height: 32px;
            cursor: pointer;
        }

        .filter-bar select:focus,
        .filter-bar input:focus {
            outline: none;
            border-color: var(--accent);
        }

        .btn {
            padding: 5px 14px;
            border-radius: var(--radius);
            border: none;
            cursor: pointer;
            font-size: 13px;
            height: 32px;
        }

        .btn-primary {
            background: var(--accent);
            color: #fff;
        }

        .btn-secondary {
            background: var(--surface2);
            color: var(--text);
            border: 1px solid var(--border);
        }

        .btn-danger {
            background: var(--danger);
            color: #fff;
        }

        .btn:hover {
            opacity: .85;
        }

        .auto-label {
            display: flex;
            align-items: center;
            gap: 6px;
            font-size: 13px;
            color: var(--muted);
            white-space: nowrap;
        }

        .auto-label input[type=checkbox] {
            cursor: pointer;
        }

        /* ── Stats Grid ── */
        .stats-grid {
            display: grid;
            grid-template-columns: repeat(4, 1fr);
            gap: 12px;
            padding: 16px 24px;
        }

        .stat-card {
            background: var(--surface);
            border: 1px solid var(--border);
            border-radius: var(--radius);
            padding: 14px 16px;
        }

        .stat-value {
            font-size: 24px;
            font-weight: 700;
            color: var(--accent);
        }

        .stat-label {
            font-size: 12px;
            color: var(--muted);
            margin-top: 4px;
        }

        .stat-card.green .stat-value {
            color: var(--green);
        }

        .stat-card.warn .stat-value {
            color: var(--warn);
        }

        .stat-card.red .stat-value {
            color: var(--danger);
        }

        /* ── Table Section ── */
        .section {
            padding: 0 24px 24px;
        }

        .card {
            background: var(--surface);
            border: 1px solid var(--border);
            border-radius: var(--radius);
            overflow: hidden;
        }

        .card-header {
            padding: 12px 16px;
            border-bottom: 1px solid var(--border);
            display: flex;
            align-items: center;
            justify-content: space-between;
        }

        .card-header h3 {
            font-size: 14px;
            font-weight: 600;
        }

        .status-dot {
            width: 8px;
            height: 8px;
            border-radius: 50%;
            background: var(--green);
            display: inline-block;
            margin-right: 6px;
            animation: pulse 2s infinite;
        }

        .status-dot.paused {
            background: var(--muted);
            animation: none;
        }

        @keyframes pulse {

            0%,
            100% {
                opacity: 1
            }

            50% {
                opacity: .4
            }
        }

        .last-update {
            font-size: 12px;
            color: var(--muted);
        }

        .table-wrap {
            overflow-x: auto;
        }

        table {
            width: 100%;
            border-collapse: collapse;
            font-size: 13px;
        }

        thead tr {
            background: var(--surface2);
        }

        th {
            padding: 10px 12px;
            text-align: left;
            font-weight: 600;
            color: var(--muted);
            white-space: nowrap;
            border-bottom: 1px solid var(--border);
        }

        td {
            padding: 9px 12px;
            border-bottom: 1px solid var(--border);
            white-space: nowrap;
        }

        tbody tr:hover {
            background: var(--surface2);
        }

        tbody tr:last-child td {
            border-bottom: none;
        }

        .num {
            color: var(--muted);
            font-size: 12px;
        }

        .badge {
            display: inline-block;
            padding: 2px 8px;
            border-radius: 10px;
            font-size: 11px;
            font-weight: 600;
        }

        .badge-blue {
            background: rgba(88, 166, 255, .15);
            color: var(--accent);
        }

        .badge-green {
            background: rgba(63, 185, 80, .15);
            color: var(--green);
        }

        .badge-warn {
            background: rgba(210, 153, 34, .15);
            color: var(--warn);
        }

        .badge-red {
            background: rgba(248, 81, 73, .15);
            color: var(--danger);
        }

        /* ── Pagination ── */
        .pagination {
            display: flex;
            align-items: center;
            gap: 6px;
            padding: 12px 16px;
            border-top: 1px solid var(--border);
            flex-wrap: wrap;
        }

        .page-info {
            font-size: 12px;
            color: var(--muted);
            flex: 1;
        }

        .page-btn {
            background: var(--surface2);
            color: var(--text);
            border: 1px solid var(--border);
            border-radius: var(--radius);
            padding: 4px 10px;
            cursor: pointer;
            font-size: 12px;
        }

        .page-btn:hover {
            border-color: var(--accent);
            color: var(--accent);
        }

        .page-btn.active {
            background: var(--accent);
            color: #fff;
            border-color: var(--accent);
        }

        .page-btn:disabled {
            opacity: .4;
            cursor: default;
        }

        /* ── Loading / Empty ── */
        .empty-row td {
            text-align: center;
            padding: 32px;
            color: var(--muted);
        }

        .spinner {
            display: inline-block;
            width: 16px;
            height: 16px;
            border: 2px solid var(--border);
            border-top-color: var(--accent);
            border-radius: 50%;
            animation: spin .6s linear infinite;
            margin-right: 8px;
        }

        @keyframes spin {
            to {
                transform: rotate(360deg);
            }
        }

        /* ── UART Raw ── */
        .uart-raw {
            font-family: monospace;
            color: var(--green);
            font-size: 12px;
        }

        /* ── Footer ── */
        footer {
            text-align: center;
            padding: 20px;
            color: var(--muted);
            font-size: 12px;
            border-top: 1px solid var(--border);
            margin-top: 8px;
        }
    </style>
</head>

<body>

    <div class="header">
        <h1>SunTech Embroidery IoT Data Log</h1>
        <span class="header-meta">Embroidery IoT &mdash; CTP280_API</span>
    </div>

    <div class="filter-bar">
        <select id="machineNoFilter">
            <option value="">All Machine</option>
            <?php foreach ($machineNoList as $mn): ?>
                <option value="<?= htmlspecialchars($mn) ?>"><?= htmlspecialchars($mn) ?></option>
            <?php endforeach; ?>
        </select>

        <select id="macFilter">
            <option value="">All MAC</option>
            <?php foreach ($macList as $m): ?>
                <option value="<?= htmlspecialchars($m) ?>"><?= htmlspecialchars($m) ?></option>
            <?php endforeach; ?>
        </select>

        <select id="rangeFilter">
            <option value="today" selected>Today</option>
            <option value="yesterday">Yesterday</option>
            <option value="1w">Last 7 Days</option>
        </select>

        <button class="btn btn-primary" id="searchBtn">Search</button>
        <button class="btn btn-secondary" id="refreshBtn">Refresh</button>

        <label class="auto-label">
            <input type="checkbox" id="autoRefresh" checked>
            Auto-refresh (10s)
        </label>

        <span id="countBadge" class="badge badge-blue" style="margin-left:auto">-</span>
    </div>

    <!-- Stats -->
    <div class="stats-grid">
        <!-- <div class="stat-card">
            <div class="stat-value" id="statTotal">-</div>
            <div class="stat-label">Total Records</div>
        </div> -->
        <div class="stat-card">
            <div class="stat-value" id="statActual">-</div>
            <div class="stat-label">Total production quantity</div>
        </div>
        <div class="stat-card green">
            <div class="stat-value" id="statCt">-</div>
            <div class="stat-label">Total Cycle Time (sec)</div>
        </div>
        <div class="stat-card warn">
            <div class="stat-value" id="statMrt">-</div>
            <div class="stat-label">Total Motor Runtime (sec)</div>
        </div>
        <div class="stat-card red">
            <div class="stat-value" id="statTb">-</div>
            <div class="stat-label">Total Thread Break</div>
        </div>
    </div>

    <!-- Table -->
    <div class="section">
        <div class="card">
            <div class="card-header">
                <h3><span class="status-dot" id="statusDot"></span>Embroidery Packet Log</h3>
                <span class="last-update" id="lastUpdate">-</span>
            </div>
            <div class="table-wrap">
                <table>
                    <thead>
                        <tr>
                            <th>idx</th>
                            <th>Machine No</th>
                            <th>MAC</th>
                            <th>actual_qty</th>
                            <th>CT (s)</th>
                            <th>TB</th>
                            <th>MRT (s)</th>
                            <th>UART DATA</th>
                            <th>created_datetime</th>
                        </tr>
                    </thead>
                    <tbody id="tableBody">
                        <tr class="empty-row">
                            <td colspan="9"><span class="spinner"></span>Loading...</td>
                        </tr>
                    </tbody>
                </table>
            </div>
            <div class="pagination" id="pagination"></div>
        </div>
    </div>

    <footer>&copy; 2026 SUNTECH. All Rights Reserved.</footer>

    <script>
        (function() {
            let currentPage = 1;
            let totalRecords = 0;
            const perPage = 50;
            let autoTimer = null;

            const $ = id => document.getElementById(id);

            function buildUrl(page) {
                const machineNo = $('machineNoFilter').value;
                const mac = $('macFilter').value;
                const range = $('rangeFilter').value;
                return `?ajax=1&machine_no=${encodeURIComponent(machineNo)}&mac=${encodeURIComponent(mac)}&range=${range}&page=${page}`;
            }

            async function loadData(page) {
                page = page || currentPage;
                try {
                    const res = await fetch(buildUrl(page));
                    const json = await res.json();
                    currentPage = json.page;
                    totalRecords = json.total;
                    renderStats(json.stats, json.total);
                    renderTable(json.rows);
                    renderPagination(json.total, json.page);
                    $('lastUpdate').textContent = 'Last updated: ' + new Date().toLocaleTimeString();
                    $('countBadge').textContent = json.total + ' rows';
                    $('statusDot').className = 'status-dot';
                } catch (e) {
                    $('statusDot').className = 'status-dot paused';
                    console.error('Load error:', e);
                }
            }

            function renderStats(s, total) {
                // $('statTotal').textContent = total.toLocaleString();
                $('statCt').textContent = s.total_ct !== undefined ? s.total_ct : '-';
                $('statMrt').textContent = s.total_mrt !== undefined ? s.total_mrt : '-';
                $('statTb').textContent = s.total_tb !== undefined ? s.total_tb : '-';
                $('statActual').textContent = s.total_actual !== undefined ? s.total_actual : '-';
            }

            /* function renderTable(rows) {
                const tbody = $('tableBody');
                if (!rows || rows.length === 0) {
                    tbody.innerHTML = '<tr class="empty-row"><td colspan="9">No data found.</td></tr>';
                    return;
                }
                tbody.innerHTML = rows.map(r => {
                    const tbClass = r.tb > 0 ? 'badge badge-red' : 'badge badge-green';
                    const aqClass = r.actual_qty > 0 ? 'badge badge-blue' : 'badge badge-warn';
                    const mnLabel = r.machine_no ? r.machine_no : '<span style="color:var(--muted)">-</span>';
                    return `<tr>
                <td class="num">${r.idx}</td>
                <td>${mnLabel}</td>
                <td><span class="badge badge-blue">${r.mac}</span></td>
                <td><span class="${aqClass}">${r.actual_qty}</span></td>
                <td>${r.ct}</td>
                <td><span class="${tbClass}">${r.tb}</span></td>
                <td>${r.mrt}</td>
                <td class="num uart-raw">${r.actual_qty};${Math.round(r.ct*1000)};${r.tb};${Math.round(r.mrt*1000)};</td>
                <td class="num">${r.created_at}</td>
            </tr>`;
                }).join('');
            } */
            function renderTable(rows) {
                const tbody = $('tableBody');
                if (!rows || rows.length === 0) {
                    tbody.innerHTML = '<tr class="empty-row"><td colspan="9">No data found.</td></tr>';
                    return;
                }
                tbody.innerHTML = rows.map(r => {
                    const tbClass = r.tb > 0 ? 'badge badge-red' : 'badge badge-green';
                    const aqClass = r.actual_qty > 0 ? 'badge badge-blue' : 'badge badge-warn';
                    const mnLabel = r.machine_no ? r.machine_no : '<span style="color:var(--muted)">-</span>';
                    return `<tr>
                <td class="num">${r.idx}</td>
                <td>${mnLabel}</td>
                <td><span class="badge badge-blue">${r.mac}</span></td>
                <td><span class="${aqClass}">${r.actual_qty}</span></td>
                <td>${r.ct}</td>
                <td><span class="${tbClass}">${r.tb}</span></td>
                <td>${r.mrt}</td>
                <td class="num uart-raw">${r.actual_qty};${Math.round(r.ct)};${r.tb};${Math.round(r.mrt)};</td>
                <td class="num">${r.created_at}</td>
            </tr>`;
                }).join('');
            }

            function renderPagination(total, page) {
                const pg = $('pagination');
                const last = Math.ceil(total / perPage);
                if (last <= 1) {
                    pg.innerHTML = '';
                    return;
                }

                const pageInfo = `<span class="page-info">Page ${page} / ${last} &nbsp; (${total} records)</span>`;
                let btns = '';

                if (page > 1) btns += `<button class="page-btn" onclick="goPage(1)">&laquo;</button><button class="page-btn" onclick="goPage(${page - 1})">&lsaquo;</button>`;

                // window of 5 pages
                const start = Math.max(1, page - 2);
                const end = Math.min(last, page + 2);
                for (let i = start; i <= end; i++) {
                    btns += `<button class="page-btn${i === page ? ' active' : ''}" onclick="goPage(${i})">${i}</button>`;
                }

                if (page < last) btns += `<button class="page-btn" onclick="goPage(${page + 1})">&rsaquo;</button><button class="page-btn" onclick="goPage(${last})">&raquo;</button>`;

                pg.innerHTML = pageInfo + btns;
            }

            window.goPage = function(p) {
                loadData(p);
            };

            function resetAutoTimer() {
                clearInterval(autoTimer);
                if ($('autoRefresh').checked) {
                    autoTimer = setInterval(() => loadData(currentPage), 10000);
                }
            }

            $('searchBtn').addEventListener('click', () => {
                currentPage = 1;
                loadData(1);
            });
            $('refreshBtn').addEventListener('click', () => loadData(currentPage));
            $('autoRefresh').addEventListener('change', resetAutoTimer);
            $('rangeFilter').addEventListener('change', () => {
                currentPage = 1;
                loadData(1);
            });
            $('machineNoFilter').addEventListener('change', () => {
                currentPage = 1;
                loadData(1);
            });
            $('macFilter').addEventListener('change', () => {
                currentPage = 1;
                loadData(1);
            });

            // 초기 로드
            loadData(1);
            resetAutoTimer();
        })();
    </script>
</body>

</html>