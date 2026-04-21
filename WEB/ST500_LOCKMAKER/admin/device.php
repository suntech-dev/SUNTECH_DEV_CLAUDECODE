<?php
require_once __DIR__ . '/lib/db.php';

// ── AJAX 엔드포인트 ──────────────────────────────────────────────
if (isset($_GET['ajax'])) {
    header('Content-Type: application/json; charset=utf-8');

    $state  = trim($_GET['state']  ?? '');
    $search = trim($_GET['search'] ?? '');
    $page   = max(1, (int)($_GET['page'] ?? 1));
    $limit  = 25;
    $offset = ($page - 1) * $limit;

    $pdo   = get_pdo();
    $conds = [];
    $binds = [];

    if ($state !== '') {
        $conds[] = 'state = ?';
        $binds[] = $state;
    }
    if ($search !== '') {
        $conds[] = '(device_id LIKE ? OR name LIKE ?)';
        $binds[] = "%{$search}%";
        $binds[] = "%{$search}%";
    }

    $where = $conds ? 'WHERE ' . implode(' AND ', $conds) : '';

    $stmt = $pdo->prepare("SELECT COUNT(*) FROM lm_device {$where}");
    $stmt->execute($binds);
    $total = (int)$stmt->fetchColumn();

    $stmt = $pdo->prepare(
        "SELECT idx, device_id, name, state, LEFT(reg_date, 10) AS reg_date
         FROM lm_device {$where}
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

// ── 상태 변경 (POST ?action=update) ─────────────────────────────
if ($_SERVER['REQUEST_METHOD'] === 'POST' && ($_GET['action'] ?? '') === 'update') {
    header('Content-Type: application/json; charset=utf-8');

    $idx   = (int)trim($_POST['idx']   ?? 0);
    $state = trim($_POST['state'] ?? '');

    if (!$idx || !in_array($state, ['Y', 'N', 'D'], true)) {
        echo json_encode(['result' => 'fail', 'error' => 'invalid_params']);
        exit;
    }

    $pdo  = get_pdo();
    $stmt = $pdo->prepare("UPDATE lm_device SET state = ?, update_date = ? WHERE idx = ?");
    $stmt->execute([$state, date('Y-m-d H:i:s'), $idx]);

    echo json_encode($stmt->rowCount() ? ['result' => 'ok'] : ['result' => 'fail', 'error' => 'not_found']);
    exit;
}
?>
<!DOCTYPE html>
<html lang="ko">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Device — ST500 Admin</title>
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
        .filter-bar input[type=text] { background: var(--surface2); color: var(--text); border: 1px solid var(--border); border-radius: var(--radius); padding: 5px 10px; font-size: 13px; height: 32px; }
        .filter-bar input[type=text]:focus { outline: none; border-color: var(--accent); }

        .radio-group { display: flex; gap: 4px; flex-wrap: wrap; }
        .radio-btn { position: relative; }
        .radio-btn input { position: absolute; opacity: 0; width: 0; height: 0; }
        .radio-btn label { display: inline-block; padding: 4px 14px; border-radius: 20px; background: var(--surface2); border: 1px solid var(--border); color: var(--muted); cursor: pointer; font-size: 12px; line-height: 24px; }
        .radio-btn input:checked + label { background: var(--accent); color: #fff; border-color: var(--accent); }

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
        td { padding: 9px 12px; border-bottom: 1px solid var(--border); white-space: nowrap; }
        tbody tr:hover { background: var(--surface2); }
        tbody tr:last-child td { border-bottom: none; }
        .num { color: var(--muted); font-size: 12px; }
        .mono { font-family: monospace; font-size: 12px; }

        .badge { display: inline-block; padding: 2px 9px; border-radius: 10px; font-size: 11px; font-weight: 600; }
        .badge-wait    { background: rgba(210,153,34,.2);  color: var(--warn); }
        .badge-approve { background: rgba(63,185,80,.2);   color: var(--green); }
        .badge-delete  { background: rgba(139,148,158,.15); color: var(--muted); }

        .btn-edit { background: none; border: 1px solid var(--border); color: var(--warn); border-radius: var(--radius); padding: 3px 10px; cursor: pointer; font-size: 12px; }
        .btn-edit:hover { background: rgba(210,153,34,.1); border-color: var(--warn); }

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

        /* ── Modal ── */
        .overlay { display: none; position: fixed; inset: 0; background: rgba(0,0,0,.65); z-index: 200; place-items: center; }
        .overlay.open { display: grid; }
        .modal { background: var(--surface); border: 1px solid var(--border); border-radius: 8px; padding: 24px; width: min(460px, 90vw); }
        .modal-hd { display: flex; justify-content: space-between; align-items: center; margin-bottom: 20px; }
        .modal-hd h4 { font-size: 15px; font-weight: 600; }
        .modal-close { background: none; border: none; cursor: pointer; color: var(--muted); font-size: 20px; line-height: 1; padding: 0 4px; }
        .modal-close:hover { color: var(--danger); }
        .field { margin-bottom: 14px; }
        .field label { display: block; font-size: 11px; color: var(--muted); margin-bottom: 5px; text-transform: uppercase; letter-spacing: .04em; }
        .field input, .field select { width: 100%; background: var(--surface2); color: var(--text); border: 1px solid var(--border); border-radius: var(--radius); padding: 8px 10px; font-size: 13px; }
        .field input:disabled { opacity: .5; cursor: not-allowed; }
        .field select:focus { outline: none; border-color: var(--accent); }
        .modal-ft { display: flex; justify-content: flex-end; gap: 8px; margin-top: 20px; border-top: 1px solid var(--border); padding-top: 16px; }
        .btn-save { background: var(--green); color: #fff; border: none; border-radius: var(--radius); padding: 7px 20px; cursor: pointer; font-size: 13px; }
        .btn-save:hover { opacity: .85; }
        .btn-cancel { background: var(--surface2); color: var(--text); border: 1px solid var(--border); border-radius: var(--radius); padding: 7px 16px; cursor: pointer; font-size: 13px; }
        .btn-cancel:hover { border-color: var(--muted); }

        footer { text-align: center; padding: 20px; color: var(--muted); font-size: 12px; border-top: 1px solid var(--border); margin-top: 8px; }
    </style>
</head>
<body>

<!-- Nav -->
<nav class="nav">
    <a class="nav-brand" href="device.php">ST-500 LOCKMAKER ADMIN</a>
    <div class="nav-links" id="navLinks">
        <a class="nav-link active" href="device.php">DEVICE</a>
        <a class="nav-link" href="log.php">LOG</a>
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
    <div class="radio-group">
        <div class="radio-btn"><input type="radio" name="fs" id="fs_all" value="" checked><label for="fs_all">ALL</label></div>
        <div class="radio-btn"><input type="radio" name="fs" id="fs_n" value="N"><label for="fs_n">대기</label></div>
        <div class="radio-btn"><input type="radio" name="fs" id="fs_y" value="Y"><label for="fs_y">승인</label></div>
        <div class="radio-btn"><input type="radio" name="fs" id="fs_d" value="D"><label for="fs_d">삭제</label></div>
    </div>
    <input type="text" id="searchInput" placeholder="DEVICE ID / NAME 검색" style="width:220px">
    <button class="btn btn-primary" onclick="loadData(1)">Search</button>
    <button class="btn btn-secondary" onclick="loadData(currentPage)">Refresh</button>
</div>

<!-- Table -->
<div class="section">
    <div class="card">
        <div class="card-header">
            <h3>Device List</h3>
            <span class="count-badge" id="countBadge">—</span>
        </div>
        <div class="table-wrap">
            <table>
                <thead>
                    <tr>
                        <th>NO</th>
                        <th>DEVICE ID</th>
                        <th>NAME</th>
                        <th>STATUS</th>
                        <th>REG DATE</th>
                        <th>ACTION</th>
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

<!-- Edit Modal -->
<div class="overlay" id="overlay" onclick="if(event.target===this)closeModal()">
    <div class="modal">
        <div class="modal-hd">
            <h4>EDIT DEVICE</h4>
            <button class="modal-close" onclick="closeModal()">&#215;</button>
        </div>
        <div class="field">
            <label>IDX</label>
            <input type="text" id="m_idx" disabled>
        </div>
        <div class="field">
            <label>DEVICE ID</label>
            <input type="text" id="m_device_id" class="mono" disabled>
        </div>
        <div class="field">
            <label>NAME</label>
            <input type="text" id="m_name" disabled>
        </div>
        <div class="field">
            <label>STATUS</label>
            <select id="m_state">
                <option value="N">대기 (N)</option>
                <option value="Y">승인 (Y)</option>
                <option value="D">삭제 (D)</option>
            </select>
        </div>
        <div class="modal-ft">
            <button class="btn-cancel" onclick="closeModal()">Cancel</button>
            <button class="btn-save" onclick="saveDevice()">SAVE</button>
        </div>
    </div>
</div>

<footer>&copy; 2026 SUNTECH. All Rights Reserved.</footer>

<script>
(function () {
    let currentPage = 1;
    let totalRecords = 0;
    const perPage = 25;
    let editIdx = 0;

    window.currentPage = 1;

    function buildUrl(page) {
        const state  = document.querySelector('input[name=fs]:checked').value;
        const search = document.getElementById('searchInput').value.trim();
        return `?ajax=1&state=${encodeURIComponent(state)}&search=${encodeURIComponent(search)}&page=${page}`;
    }

    function stateBadge(s) {
        const map = { N: ['badge-wait','대기'], Y: ['badge-approve','승인'], D: ['badge-delete','삭제'] };
        const [cls, label] = map[s] || ['badge-delete', s];
        return `<span class="badge ${cls}">${label}</span>`;
    }

    async function loadData(page) {
        currentPage = page;
        window.currentPage = page;
        document.getElementById('tableBody').innerHTML =
            '<tr class="empty-row"><td colspan="6"><span class="spinner"></span>Loading...</td></tr>';
        try {
            const res  = await fetch(buildUrl(page));
            const json = await res.json();
            totalRecords = json.total;
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
            <td class="mono">${r.device_id}</td>
            <td>${r.name}</td>
            <td>${stateBadge(r.state)}</td>
            <td class="num">${r.reg_date}</td>
            <td><button class="btn-edit"
                    data-idx="${r.idx}"
                    data-device="${r.device_id}"
                    data-name="${r.name}"
                    data-state="${r.state}"
                    onclick="openModal(this)">편집</button></td>
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

    // 모달
    window.openModal = function (btn) {
        editIdx = btn.dataset.idx;
        document.getElementById('m_idx').value       = btn.dataset.idx;
        document.getElementById('m_device_id').value = btn.dataset.device;
        document.getElementById('m_name').value      = btn.dataset.name;
        document.getElementById('m_state').value     = btn.dataset.state;
        document.getElementById('overlay').classList.add('open');
    };

    window.closeModal = function () {
        document.getElementById('overlay').classList.remove('open');
    };

    window.saveDevice = async function () {
        const state = document.getElementById('m_state').value;
        try {
            const fd = new FormData();
            fd.append('idx',   editIdx);
            fd.append('state', state);
            const res  = await fetch('?action=update', { method: 'POST', body: fd });
            const json = await res.json();
            if (json.result === 'ok') {
                closeModal();
                loadData(currentPage);
            } else {
                alert(json.error || '저장 실패');
            }
        } catch (e) { alert('요청 실패'); }
    };

    // Enter 검색
    document.getElementById('searchInput').addEventListener('keydown', e => { if (e.key === 'Enter') loadData(1); });
    document.querySelectorAll('input[name=fs]').forEach(r => r.addEventListener('change', () => loadData(1)));

    loadData(1);
})();
</script>
</body>
</html>
