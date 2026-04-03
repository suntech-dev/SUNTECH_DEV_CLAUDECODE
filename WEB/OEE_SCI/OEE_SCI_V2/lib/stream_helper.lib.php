<?php

/**
 * Stream Helper Library — Common SSE Utilities
 * Created: 2026-03-07 (Phase 3-A)
 *
 * Shared functions extracted from all SSE stream scripts to eliminate
 * code duplication across data_oee_stream, log_oee_stream,
 * data_downtime_stream, data_andon_stream, data_defective_stream.
 *
 * Usage:
 *   require_once(__DIR__ . '/stream_helper.lib.php');
 *
 * parseFilterParams call per file:
 *   data_oee_stream.php    : parseFilterParams('do', 'work_date', true,  '7 DAY')
 *   log_oee_stream.php     : parseFilterParams('do', 'work_date', true,  '7 DAY')
 *   data_downtime_stream   : parseFilterParams('dd', 'reg_date',  false, '2 DAY')
 *   data_andon_stream      : parseFilterParams('da', 'reg_date',  false, '2 DAY')
 *   data_defective_stream  : parseFilterParams('dd', 'reg_date',  false, '2 DAY')
 */

// ---------------------------------------------------------------------------
// SSE output helper
// ---------------------------------------------------------------------------

/**
 * Send a single SSE event to the client and flush immediately.
 *
 * @param string $eventType  SSE event name (e.g. 'connected', 'oee_data')
 * @param mixed  $data       Data to JSON-encode and send
 */
function sendSSEData($eventType, $data)
{
    echo "event: {$eventType}\n";
    echo "data: " . json_encode($data, JSON_UNESCAPED_UNICODE | JSON_UNESCAPED_SLASHES) . "\n\n";
    flush();
}

// ---------------------------------------------------------------------------
// Filter parameter parser
// ---------------------------------------------------------------------------

/**
 * Parse GET filter parameters and build a parameterized WHERE clause.
 *
 * @param string $tableAlias     SQL table alias prefix (e.g. 'do', 'dd', 'da')
 * @param string $dateColumn     Date column name ('work_date' or 'reg_date')
 * @param bool   $isDateOnly     true  → DATE column (no time appended, uses CURDATE())
 *                               false → DATETIME column (appends 00:00:00/23:59:59, uses NOW())
 * @param string $defaultInterval  MySQL interval for default range, e.g. '7 DAY', '2 DAY'
 *
 * @return array ['where_sql' => string, 'params' => array]
 */
function parseFilterParams($tableAlias, $dateColumn, $isDateOnly = false, $defaultInterval = '2 DAY')
{
    $params = [];
    $where_clauses = [];
    $p = $tableAlias . '.';

    if (!empty($_GET['factory_filter'])) {
        $where_clauses[] = "{$p}factory_idx = ?";
        $params[] = $_GET['factory_filter'];
    }

    if (!empty($_GET['line_filter'])) {
        $where_clauses[] = "{$p}line_idx = ?";
        $params[] = $_GET['line_filter'];
    }

    if (!empty($_GET['machine_filter'])) {
        $where_clauses[] = "{$p}machine_idx = ?";
        $params[] = $_GET['machine_filter'];
    }

    if (!empty($_GET['shift_filter'])) {
        $where_clauses[] = "{$p}shift_idx = ?";
        $params[] = $_GET['shift_filter'];
    }

    if ($isDateOnly) {
        // DATE column: compare date strings directly
        if (!empty($_GET['start_date'])) {
            $where_clauses[] = "{$p}{$dateColumn} >= ?";
            $params[] = $_GET['start_date'];
        }
        if (!empty($_GET['end_date'])) {
            $where_clauses[] = "{$p}{$dateColumn} <= ?";
            $params[] = $_GET['end_date'];
        }
        if (empty($_GET['start_date']) && empty($_GET['end_date'])) {
            $where_clauses[] = "{$p}{$dateColumn} >= DATE_SUB(CURDATE(), INTERVAL {$defaultInterval})";
        }
    } else {
        // DATETIME column: append time to bound the full day
        if (!empty($_GET['start_date'])) {
            $where_clauses[] = "{$p}{$dateColumn} >= ?";
            $params[] = $_GET['start_date'] . ' 00:00:00';
        }
        if (!empty($_GET['end_date'])) {
            $where_clauses[] = "{$p}{$dateColumn} <= ?";
            $params[] = $_GET['end_date'] . ' 23:59:59';
        }
        if (empty($_GET['start_date']) && empty($_GET['end_date'])) {
            $where_clauses[] = "{$p}{$dateColumn} >= DATE_SUB(NOW(), INTERVAL {$defaultInterval})";
        }
    }

    $where_sql = count($where_clauses) > 0 ? ' WHERE ' . implode(' AND ', $where_clauses) : '';

    return ['where_sql' => $where_sql, 'params' => $params];
}

// ---------------------------------------------------------------------------
// Shift / work-hours helper
// ---------------------------------------------------------------------------

/**
 * Get the earliest shift start and latest shift end for a given date.
 * Returns null when no shift data is available.
 *
 * @param PDO    $pdo
 * @param string $targetDate  'Y-m-d'
 * @return array|null
 *   [
 *     'start_time'    => 'HH:MM',
 *     'end_time'      => 'HH:MM',
 *     'start_minutes' => int,
 *     'end_minutes'   => int,   // may exceed 1440 for overnight shifts
 *     'shifts'        => array
 *   ]
 */
function getWorkHoursForDate($pdo, $targetDate)
{
    try {
        $worktime  = new Worktime($pdo);
        $dayShifts = $worktime->getDayShift($targetDate, '', '');

        if (!$dayShifts || empty($dayShifts['shift'])) return null;

        $earliestStart = 24 * 60;
        $latestEnd     = 0;

        foreach ($dayShifts['shift'] as $shift) {
            if (empty($shift['available_stime']) || empty($shift['available_etime'])) continue;

            list($sh, $sm) = explode(':', $shift['available_stime']);
            list($eh, $em) = explode(':', $shift['available_etime']);
            $startMin = (int)$sh * 60 + (int)$sm;
            $endMin   = (int)$eh * 60 + (int)$em + (int)($shift['over_time'] ?? 0);

            if ($endMin <= $startMin) $endMin += 24 * 60;

            if ($startMin < $earliestStart) $earliestStart = $startMin;
            if ($endMin   > $latestEnd)     $latestEnd     = $endMin;
        }

        return [
            'start_time'    => sprintf('%02d:%02d', floor($earliestStart / 60), $earliestStart % 60),
            'end_time'      => sprintf('%02d:%02d', floor($latestEnd / 60) % 24, $latestEnd % 60),
            'start_minutes' => $earliestStart,
            'end_minutes'   => $latestEnd,
            'shifts'        => array_values($dayShifts['shift'])
        ];
    } catch (Exception $e) {
        error_log("Work hours query error: " . $e->getMessage());
        return null;
    }
}
