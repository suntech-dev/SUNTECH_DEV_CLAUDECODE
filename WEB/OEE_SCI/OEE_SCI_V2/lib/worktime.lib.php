<?php
class Worktime
{

    private $pdo;

    public function __construct($pdo)
    {
        $this->pdo = $pdo;
    }

    public function getDayShift($date = '', $factory_id = '', $line_id = '')
    {

        if (!preg_match("/^([0-9]{4})-([0-9]{2})-([0-9]{2})$/", $date)) {
            return "Invalid date format";
        }

        $week_cnt = date('w', strtotime($date));                  // 요일 : 0~6 값 (0:일요일, 6:토요일)

        $qry = array();
        $qry[] = "status = 'Y'";
        $qry[] = "work_sdate <= '" . $date . "'";
        $qry[] = "work_edate >= '" . $date . "'";
        if ($factory_id) {
            $qry[] = "(factory_idx = '{$factory_id}' AND line_idx = '{$line_id}' OR factory_idx = 0 OR factory_idx = '{$factory_id}' AND line_idx = 0)";
        }

        $stmt = $this->pdo->prepare("SELECT * FROM `info_work_time` WHERE " . implode(' AND ', $qry) . " ORDER BY kind DESC, reg_date DESC");
        $stmt->execute();
        $result = $stmt->fetchAll(PDO::FETCH_ASSOC);

        $ret = array();

        foreach ($result as $row => $val) {
            if ($val['status'] == 'N') continue;
            if ($val['kind'] == '2' && substr($val['week_yn'], $week_cnt, 1) == '0') continue;

            $ret = $val;

            ## shift time 가져오기
            $stmt = $this->pdo->prepare("SELECT * FROM `info_work_time_shift` WHERE work_time_idx=? ORDER BY shift_idx");
            $stmt->execute([$val['idx']]);
            $shift = $stmt->fetchAll(PDO::FETCH_ASSOC);
            foreach ($shift as $shift_row) {
                $ret['shift'][$shift_row['shift_idx']] = $shift_row;
            }
            break;
        }
        return $ret;
    }

    public function getDayOfWeekShift() {}

    public function getPeriodShift() {}
}
