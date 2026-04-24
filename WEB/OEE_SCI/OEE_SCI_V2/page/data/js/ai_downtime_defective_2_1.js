/**
 * ============================================================
 * 파일명: ai_downtime_defective_2_1.js
 * 목  적: ai_dashboard_2_1 전용 — Downtime Top5 + Defective Top5 차트
 *         - proc/downtime_defective_top5.php 폴링 (60초 간격)
 *         - getFilterParams() 공통 필터 사용
 *         - 데이터가 5개 미만이어도 5칸 공간 유지
 * ============================================================
 */
(function () {
  "use strict";

  var POLL_INTERVAL = 60000; // 60초
  var dtChart = null;
  var defChart = null;

  // ── 색상 팔레트 ───────────────────────────────────────────
  var DT_COLORS = ["rgba(231,76,60,0.75)", "rgba(230,126,34,0.75)", "rgba(241,196,15,0.75)", "rgba(52,152,219,0.75)", "rgba(155,89,182,0.75)"];
  var DEF_COLORS = ["rgba(52,152,219,0.75)", "rgba(46,204,113,0.75)", "rgba(241,196,15,0.75)", "rgba(230,126,34,0.75)", "rgba(231,76,60,0.75)"];
  var DT_BORDERS = DT_COLORS.map(function (c) {
    return c.replace("0.75", "1");
  });
  var DEF_BORDERS = DEF_COLORS.map(function (c) {
    return c.replace("0.75", "1");
  });

  // ── Chart.js 공통 옵션 ───────────────────────────────────
  function baseOptions(unit) {
    return {
      responsive: true,
      maintainAspectRatio: false,
      animation: { duration: 400 },
      plugins: {
        legend: { display: false },
        tooltip: {
          callbacks: {
            label: function (ctx) {
              var v = ctx.parsed.y;
              return " " + (v === 0 && ctx.label === "" ? "-" : v + " " + unit);
            },
          },
        },
      },
      scales: {
        x: {
          ticks: {
            // color: "#e6edf3",
            color: "#264a69",
            font: { size: 11 },
            callback: function (val, idx) {
              var label = this.getLabelForValue(val);
              return label === "" ? "—" : label;
            },
          },
          grid: { display: false },
        },
        y: {
          beginAtZero: true,
          ticks: {
            color: "#8b949e",
            font: { size: 11 },
            maxTicksLimit: 5,
          },
          grid: { color: "rgba(255,255,255,0.06)" },
        },
      },
    };
  }

  // ── 차트 초기화 ──────────────────────────────────────────
  function initCharts() {
    var dtCanvas = document.getElementById("aiDtTop5Chart");
    var defCanvas = document.getElementById("aiDefTop5Chart");
    if (!dtCanvas || !defCanvas) return;

    dtChart = new Chart(dtCanvas.getContext("2d"), {
      type: "bar",
      data: {
        labels: ["", "", "", "", ""],
        datasets: [
          {
            data: [0, 0, 0, 0, 0],
            backgroundColor: DT_COLORS,
            borderColor: DT_BORDERS,
            borderWidth: 1,
            borderRadius: 3,
            barThickness: "flex",
            maxBarThickness: 28,
          },
        ],
      },
      options: baseOptions("min"),
    });

    defChart = new Chart(defCanvas.getContext("2d"), {
      type: "bar",
      data: {
        labels: ["", "", "", "", ""],
        datasets: [
          {
            data: [0, 0, 0, 0, 0],
            backgroundColor: DEF_COLORS,
            borderColor: DEF_BORDERS,
            borderWidth: 1,
            borderRadius: 3,
            barThickness: "flex",
            maxBarThickness: 28,
          },
        ],
      },
      // options: baseOptions('건')
      options: baseOptions(""),
    });
  }

  // ── 데이터 로드 ──────────────────────────────────────────
  function load() {
    if (!dtChart || !defChart) return;

    var params = typeof getFilterParams === "function" ? getFilterParams() : {};

    $.getJSON("proc/downtime_defective_top5.php", params, function (data) {
      if (data.code !== "00") return;

      // Downtime 차트 업데이트
      var dtLabels = data.downtime.map(function (r) {
        return r.downtime_name || "";
      });
      var dtValues = data.downtime.map(function (r) {
        return parseFloat(r.total_duration_min) || 0;
      });
      dtChart.data.labels = dtLabels;
      dtChart.data.datasets[0].data = dtValues;
      dtChart.update("none");

      // Defective 차트 업데이트
      var defLabels = data.defective.map(function (r) {
        return r.defective_name || "";
      });
      var defValues = data.defective.map(function (r) {
        return parseInt(r.count) || 0;
      });
      defChart.data.labels = defLabels;
      defChart.data.datasets[0].data = defValues;
      defChart.update("none");
    });
  }

  // ── refreshAll 에 훅 ─────────────────────────────────────
  // ai_dashboard_2.js 의 refreshAll() 이 호출될 때 함께 갱신
  var _origRefreshAll = window.refreshAll;
  window.refreshAll = function () {
    if (typeof _origRefreshAll === "function") _origRefreshAll.apply(this, arguments);
    load();
  };

  // ── 초기화 & 주기 갱신 ───────────────────────────────────
  document.addEventListener("DOMContentLoaded", function () {
    initCharts();
    setTimeout(load, 500); // 다른 카드 로드 후 실행
    setInterval(load, POLL_INTERVAL);
  });
})();
