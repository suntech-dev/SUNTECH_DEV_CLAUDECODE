<?php
require_once __DIR__ . '/lib/db.php';

// ── AJAX 엔드포인트 ──────────────────────────────────────────────
if (isset($_GET['ajax'])) {
    header('Content-Type: application/json; charset=utf-8');

    $code   = trim($_GET['code']   ?? '');
    $search = trim($_GET['search'] ?? '');
    $page   = max(1, (int)($_GET['page'] ?? 1));
    $limit  = 50;
    $offset = ($page - 1) * $limit;

    $pdo   = get_pdo();
    $conds = [];
    $binds = [];

    if ($code !== '') {
        $conds[] = 'code = ?';
        $binds[] = $code;
    }
    if ($search !== '') {
        $like    = "%{$search}%";
        $conds[] = '(log_data LIKE ? OR log_result LIKE ? OR reg_ip LIKE ?)';
        $binds[] = $like; $binds[] = $like; $binds[] = $like;
    }

    $where = $conds ? 'WHERE ' . implode(' AND ', $conds) : '';

    $stmt = $pdo->prepare("SELECT COUNT(*) FROM lm_logs {$where}");
    $stmt->execute($binds);
    $total = (int)$stmt->fetchColumn();

    $stmt = $pdo->prepare(
        "SELECT idx, code, log_data, log_result, reg_ip, reg_date
         FROM lm_logs {$where}
         ORDER BY idx DESC
         LIMIT {$limit} OFFSET {$offset}"
    );
    $stmt->execute($binds);
    $rows = $stmt->fetchAll(PDO::FETCH_ASSOC);

    $no = $total - $offset;
    foreach ($rows as &$row) { $row['no'] = $no--; }
    unset($row);

    echo json_encode(['total' => $total, 'page' => $page, 'per_page' => $limit, 'rows' => $rows], JSON_UNESCAPED_UNICODE);
    exit;
}
?>
<!DOCTYPE html>
<html lang="ko">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Log — ST500 Admin</title>
    <style>
        *, *::before, *::after { box-sizing: border-box; margin: 0; padding: 0; }

        :root {
            --bg:      #0d1117;
            --surface: #161b22;
            --surface2:#21262d;
            --border:  #30363d;
            --text:    #e6edf3;
            --muted:   #8b949e;
            --accent:  #58a6ff;
            --green:   #3fb950;
            --warn:    #d29922;
            --danger:  #f85149;
            --cyan:    #00BCD4;
            --radius:  6px;
        }

        body { background: var(--bg); color: var(--text); font-family: 'Helvetica Neue', Arial, sans-serif; font-size: 14px; min-height: 100vh; }

        /* ── Nav ── */
        .nav { background: var(--surface); border-bottom: 1px solid var(--border); height: 48px; display: flex; align-items: center; padding: 0 20px; position: sticky; top: 0; z-index: 100; }
        .nav-brand { font-size: 15px; font-weight: 700; color: var(--cyan); text-decoration: none; white-space: nowrap; }
        .nav-links { display: flex; gap: 4px; margin-left: auto; }
        .nav-link { color: var(--muted); text-decoration: none; padding: 6px 14px; border-radius: var(--radius); font-size: 13px; }
        .nav-link:hover { background: var(--surface2); color: var(--text); }
        .nav-link.active { background: var(--surface2); color: var(--accent); }
        .nav-burger { display: none; background: none; border: none; cursor: pointer; color: var(--text); padding: 4px; margin-left: auto; }
        @media (max-width: 600px) {
            .nav-links { display: none; position: absolute; top: 48px; left: 0; right: 0; background: var(--surface); border-bottom: 1px solid var(--border); flex-direction: column; padding: 8px; }
            .nav-links.open { display: flex; }
            .nav-burger { display: flex; align-items: center; }
        }

        /* ── Filter Bar ── */
        .filter-bar { background: var(--surface); border-bottom: 1px solid var(--border); padding: 10px 20px; display: flex; flex-wrap: wrap; gap: 8px; align-items: center; }
        .filter-bar select, .filter-bar input[type=text] { background: var(--surface2); color: var(--text); border: 1px solid var(--border); border-radius: var(--radius); padding: 5px 10px; font-size: 13px; height: 32px; }
        .filter-bar select:focus, .filter-bar input[type=text]:focus { outline: none; border-color: var(--accent); }
        .btn { padding: 5px 14px; border-radius: var(--radius); border: none; cursor: pointer; font-size: 13px; height: 32px; }
        .btn-primary { background: var(--accent); color: #fff; }
        .btn-secondary { background: var(--surface2); color: var(--text); border: 1px solid var(--border); }
        .btn:hover { opacity: .85; }

        /* ── Table ── */
        .section { padding: 16px 20px 24px; }
        .card { background: var(--surface); border: 1px solid var(--border); border-radius: var(--radius); overflow: hidden; }
        .card-header { padding: 12px 16px; border-bottom: 1px solid var(--border); display: flex; align-items: center; justify-content: space-between; }
        .card-header h3 { font-size: 14px; font-weight: 600; }
        .count-badge { font-size: 12px; color: var(--muted); }
        .table-wrap { overflow-x: auto; }
        table { width: 100%; border-collapse: collapse; font-size: 13px; }
        thead tr { background: var(--surface2); }
        th { padding: 10px 12px; text-align: left; font-weight: 600; color: var(--muted); white-space: nowrap; border-bottom: 1px solid var(--border); }
        td { padding: 9px 12px; border-bottom: 1px solid var(--border); max-width: 240px; overflow: hidden; text-overflow: ellipsis; white-space: nowrap; }
        tbody tr:hover { background: var(--surface2); }
        tbody tr:last-child td { border-bottom: none; }
        .num { color: var(--muted); font-size: 12px; }
        .mono { font-family: monospace; font-size: 12px; }

        .badge { display: inline-block; padding: 2px 9px; border-radius: 10px; font-size: 11px; font-weight: 600; }
        .badge-send  { background: rgba(88,166,255,.15); color: var(--accent); }
        .badge-get   { background: rgba(63,185,80,.15);  color: var(--green); }
        .badge-other { background: rgba(139,148,158,.15); color: var(--muted); }

        /* ── Pagination ── */
        .pagination { display: flex; align-items: center; gap: 4px; padding: 12px 16px; border-top: 1px solid var(--border); flex-wrap: wrap; }
        .page-info { font-size: 12px; color: var(--muted); flex: 1; min-width: 120px; }
        .page-btn { background: var(--surface2); color: var(--text); border: 1px solid var(--border); border-radius: var(--radius); padding: 4px 10px; cursor: pointer; font-size: 12px; }
        .page-btn:hover { border-color: var(--accent); color: var(--accent); }
        .page-btn.active { background: var(--accent); color: #fff; border-color: var(--accent); }
        .page-btn:disabled { opacity: .4; cursor: default; }

        /* ── Loading / Empty ── */
        .empty-row td { text-align: center; padding: 32px; color: var(--muted); }
        .spinner { display: inline-block; width: 14px; height: 14px; border: 2px solid var(--border); border-top-color: var(--accent); border-radius: 50%; animation: spin .6s linear infinite; margin-right: 6px; vertical-align: middle; }
        @keyframes spin { to { transform: rotate(360deg); } }

        footer { text-align: center; padding: 20px; color: var(--muted); font-size: 12px; border-top: 1px solid var(--border); margin-top: 8px; }
    </style>
</head>
<body>

<!-- Nav -->
<nav class="nav">
    <a class="nav-brand" href="device.php">ST-500 LOCKMAKER ADMIN</a>
    <div class="nav-links" id="navLinks">
        <a class="nav-link" href="device.php">DEVICE</a>
        <a class="nav-link active" href="log.php">LOG</a>
        <a class="nav-link" href="../parameter/index.php">PARAMETER</a>
    </div>
    <button class="nav-burger" onclick="document.getElementById('navLinks').classList.toggle('open')">
        <svg width="20" height="20" viewBox="0 0 20 20" fill="currentColor">
            <rect y="3" width="20" height="2" rx="1"/><rect y="9" width="20" height="2" rx="1"/><rect y="15" width="20" height="2" rx="1"/>
        </svg>
    </button>
</nav>

<!-- Filter Bar -->
<div class="filter-bar">
    <select id="codeFilter">
        <option value="">전체 코드</option>
        <option value="send_device">send_device</option>
        <option value="get_device">get_device</option>
    </select>
    <input type="text" id="searchInput" placeholder="요청/응답/IP 검색" style="width:220px">
    <button class="btn btn-primary" onclick="loadData(1)">Search</button>
    <button class="btn btn-secondary" onclick="loadData(currentPage)">Refresh</button>
</div>

<!-- Table -->
<div class="section">
    <div class="card">
        <div class="card-header">
            <h3>API Log</h3>
            <span class="count-badge" id="countBadge">—</span>
        </div>
        <div class="table-wrap">
            <table>
                <thead>
                    <tr>
                        <th>NO</th>
                        <th>CODE</th>
                        <th>REQUEST</th>
                        <th>RESPONSE</th>
                        <th>IP</th>
                        <th>DATE</th>
                    </tr>
                </thead>
                <tbody id="tableBody">
                    <tr class="empty-row"><td colspan="6"><span class="spinner"></span>Loading...</td></tr>
                </tbody>
            </table>
        </div>
        <div class="pagination" id="pagination"></div>
    </div>
</div>

<footer>&copy; 2026 SUNTECH. All Rights Reserved.</footer>

<script>
(function () {
    let currentPage = 1;
    const perPage = 50;
    window.currentPage = 1;

    function esc(s) {
        return String(s).replace(/&/g,'&amp;').replace(/</g,'&lt;').replace(/>/g,'&gt;');
    }

    function codeBadge(code) {
        if (code === 'send_device') return `<span class="badge badge-send">${code}</span>`;
        if (code === 'get_device')  return `<span class="badge badge-get">${code}</span>`;
        return `<span class="badge badge-other">${esc(code)}</span>`;
    }

    function buildUrl(page) {
        const code   = document.getElementById('codeFilter').value;
        const search = document.getElementById('searchInput').value.trim();
        return `?ajax=1&code=${encodeURIComponent(code)}&search=${encodeURIComponent(search)}&page=${page}`;
    }

    async function loadData(page) {
        currentPage = page;
        window.currentPage = page;
        document.getElementById('tableBody').innerHTML =
            '<tr class="empty-row"><td colspan="6"><span class="spinner"></span>Loading...</td></tr>';
        try {
            const res  = await fetch(buildUrl(page));
            const json = await res.json();
            document.getElementById('countBadge').textContent = json.total + '건';
            renderTable(json.rows);
            renderPagination(json.total, json.page);
        } catch (e) {
            document.getElementById('tableBody').innerHTML =
                '<tr class="empty-row"><td colspan="6">로드 실패. 서버를 확인하세요.</td></tr>';
        }
    }

    function renderTable(rows) {
        const tbody = document.getElementById('tableBody');
        if (!rows || rows.length === 0) {
            tbody.innerHTML = '<tr class="empty-row"><td colspan="6">데이터가 없습니다.</td></tr>';
            return;
        }
        tbody.innerHTML = rows.map(r => `<tr>
            <td class="num">${r.no}</td>
            <td>${codeBadge(r.code)}</td>
            <td class="mono" title="${esc(r.log_data)}">${esc(r.log_data)}</td>
            <td class="mono" title="${esc(r.log_result)}">${esc(r.log_result)}</td>
            <td class="num">${esc(r.reg_ip || '')}</td>
            <td class="num">${esc(r.reg_date)}</td>
        </tr>`).join('');
    }

    function renderPagination(total, page) {
        const pg   = document.getElementById('pagination');
        const last = Math.ceil(total / perPage);
        if (last <= 1) { pg.innerHTML = `<span class="page-info">${total}건</span>`; return; }

        let html = `<span class="page-info">${page} / ${last} 페이지 (${total}건)</span>`;
        if (page > 1)    html += `<button class="page-btn" onclick="loadData(1)">&laquo;</button><button class="page-btn" onclick="loadData(${page-1})">&lsaquo;</button>`;
        const s = Math.max(1, page-2), e = Math.min(last, page+2);
        for (let i = s; i <= e; i++)
            html += `<button class="page-btn${i===page?' active':''}" onclick="loadData(${i})">${i}</button>`;
        if (page < last) html += `<button class="page-btn" onclick="loadData(${page+1})">&rsaquo;</button><button class="page-btn" onclick="loadData(${last})">&raquo;</button>`;

        pg.innerHTML = html;
    }

    document.getElementById('searchInput').addEventListener('keydown', e => { if (e.key === 'Enter') loadData(1); });
    document.getElementById('codeFilter').addEventListener('change', () => loadData(1));

    loadData(1);
})();
</script>
</body>
</html>
