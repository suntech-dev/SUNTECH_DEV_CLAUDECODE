<?php

/** 공통 시그니지 헤더 필터 컨트롤 — data 모니터링 페이지 공용 */ ?>
<select id="factoryFilterSelect" class="fiori-select">
    <option value="">All Factory</option>
</select>
<select id="factoryLineFilterSelect" class="fiori-select" disabled>
    <option value="">All Line</option>
</select>
<select id="factoryLineMachineFilterSelect" class="fiori-select" disabled>
    <option value="">All Machine</option>
</select>
<select id="timeRangeSelect" class="fiori-select">
    <option value="today" selected>Today</option>
    <option value="yesterday">Yesterday</option>
    <option value="1w">Last Week</option>
    <option value="1m">Last Month</option>
</select>
<input type="text" id="dateRangePicker" class="fiori-input date-range-input" readonly placeholder="Select date range">
<select id="shiftSelect" class="fiori-select">
    <option value="">All Shift</option>
    <option value="1" selected>Shift 1</option>
    <option value="2">Shift 2</option>
    <option value="3">Shift 3</option>
</select>