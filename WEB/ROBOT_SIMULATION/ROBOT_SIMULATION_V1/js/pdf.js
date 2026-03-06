/**
 * pdf.js - PDF 내보내기 모듈
 * jsPDF + autoTable + NanumGothic 한글 폰트를 사용하여 시뮬레이션 결과를 PDF로 저장
 * 재봉기 2대/3대 모드 동적 지원
 */
const PdfExport = (() => {
    // 한글 폰트 캐시
    let cachedFontBase64 = null;
    let fontLoading = false;

    /**
     * NanumGothic TTF 폰트를 CDN에서 동적 로드
     * @returns {Promise<string>} base64 인코딩된 폰트 데이터
     */
    async function loadKoreanFont() {
        if (cachedFontBase64) return cachedFontBase64;

        const url = 'https://cdn.jsdelivr.net/gh/fonts-archive/NanumGothic/NanumGothic.ttf';
        const response = await fetch(url);
        if (!response.ok) throw new Error('폰트 다운로드 실패: ' + response.status);

        const buffer = await response.arrayBuffer();
        const bytes = new Uint8Array(buffer);
        let binary = '';
        for (let i = 0; i < bytes.length; i++) {
            binary += String.fromCharCode(bytes[i]);
        }
        cachedFontBase64 = btoa(binary);
        return cachedFontBase64;
    }

    /**
     * jsPDF 문서에 한글 폰트 등록
     */
    function registerFont(doc, fontBase64) {
        doc.addFileToVFS('NanumGothic.ttf', fontBase64);
        doc.addFont('NanumGothic.ttf', 'NanumGothic', 'normal');
        doc.setFont('NanumGothic');
    }

    /**
     * 재봉기 위치 레이블 반환
     */
    function getMachineLabel(id) {
        const count = Config.getMachineCount();
        if (count === 2) {
            if (id === 1) return '재봉기 1 (좌측)';
            if (id === 2) return '재봉기 2 (우측)';
        } else {
            if (id === 1) return '재봉기 1 (좌측)';
            if (id === 2) return '재봉기 2 (상단)';
            if (id === 3) return '재봉기 3 (우측)';
        }
        return '재봉기 ' + id;
    }

    /**
     * 시뮬레이션 결과를 PDF로 생성 및 다운로드
     * @param {Object} results - engine.getResults() 반환값
     * @param {Object} machineConfigs - Config.getConfig().machines
     */
    async function exportPdf(results, machineConfigs) {
        if (typeof window.jspdf === 'undefined') {
            alert('PDF 라이브러리를 로딩 중입니다. 잠시 후 다시 시도해 주세요.');
            return;
        }

        if (fontLoading) return;
        fontLoading = true;

        // 로딩 표시
        const btn = document.getElementById('btn-pdf');
        const originalText = btn.textContent;
        btn.textContent = '폰트 로딩...';
        btn.disabled = true;

        try {
            const fontBase64 = await loadKoreanFont();
            const machineCount = Config.getMachineCount();
            const machineIds = Config.MACHINE_IDS;

            const { jsPDF } = window.jspdf;
            const doc = new jsPDF('p', 'mm', 'a4');
            const pageWidth = doc.internal.pageSize.getWidth();
            let y = 15;

            // 한글 폰트 등록
            registerFont(doc, fontBase64);

            // === 제목 ===
            doc.setFontSize(18);
            doc.setTextColor(37, 99, 235);
            doc.text('SunTech Co-Robot Sewing Simulation 리포트', pageWidth / 2, y, { align: 'center' });
            y += 8;

            doc.setFontSize(11);
            doc.setTextColor(100, 116, 139);
            doc.text('( 재봉기 ' + machineCount + '대 모드 )', pageWidth / 2, y, { align: 'center' });
            y += 6;

            doc.setFontSize(10);
            doc.setTextColor(100, 116, 139);
            const now = new Date();
            const dateStr = now.getFullYear() + '-' +
                String(now.getMonth() + 1).padStart(2, '0') + '-' +
                String(now.getDate()).padStart(2, '0') + ' ' +
                String(now.getHours()).padStart(2, '0') + ':' +
                String(now.getMinutes()).padStart(2, '0');
            doc.text(dateStr, pageWidth / 2, y, { align: 'center' });
            y += 10;

            // === 구분선 ===
            doc.setDrawColor(226, 232, 240);
            doc.setLineWidth(0.5);
            doc.line(15, y, pageWidth - 15, y);
            y += 8;

            // autoTable 공통 한글 폰트 설정
            const fontStyle = { font: 'NanumGothic' };

            // === 0. 기본 설정 ===
            const basicSettings = Config.readBasicSettings();
            doc.setFontSize(13);
            doc.setTextColor(30, 41, 59);
            doc.text('1. 기본 설정', 15, y);
            y += 6;

            const basicHeaders = [['항목', '값']];
            const basicBody = [
                ['작업인원', basicSettings.workerCount + '명'],
                ['Pallet 준비시간', basicSettings.palletPrepTime + '초'],
                ['Pallet 수량', basicSettings.palletCount + '개'],
            ];

            doc.autoTable({
                startY: y,
                head: basicHeaders,
                body: basicBody,
                theme: 'grid',
                styles: { fontSize: 9, cellPadding: 3, ...fontStyle },
                headStyles: {
                    fillColor: [100, 116, 139],
                    textColor: 255,
                    fontStyle: 'bold',
                    ...fontStyle,
                },
                columnStyles: {
                    0: { fontStyle: 'bold', fillColor: [248, 250, 252], cellWidth: 50 },
                    1: { halign: 'center' },
                },
                margin: { left: 15, right: 15 },
                tableWidth: 120,
            });
            y = doc.lastAutoTable.finalY + 10;

            // === 1. 입력 설정값 테이블 ===
            doc.setFont('NanumGothic');
            doc.setFontSize(13);
            doc.setTextColor(30, 41, 59);
            doc.text('2. 입력 설정값', 15, y);
            y += 6;

            const inputHeaders = [[''].concat(machineIds.map(id => getMachineLabel(id)))];
            const inputBody = [
                ['초기 적재 시간 (초)'].concat(machineIds.map(id => machineConfigs[id].initLoadTime)),
                ['Unload & Load 시간 (초)'].concat(machineIds.map(id => machineConfigs[id].unloadLoadTime)),
                ['복귀 시간 (초)'].concat(machineIds.map(id => machineConfigs[id].returnTime)),
                ['재봉 시간 (초)'].concat(machineIds.map(id => machineConfigs[id].sewingTime)),
            ];

            doc.autoTable({
                startY: y,
                head: inputHeaders,
                body: inputBody,
                theme: 'grid',
                styles: { fontSize: 9, cellPadding: 3, halign: 'center', ...fontStyle },
                headStyles: {
                    fillColor: [37, 99, 235],
                    textColor: 255,
                    fontStyle: 'bold',
                    ...fontStyle,
                },
                columnStyles: {
                    0: { halign: 'left', fontStyle: 'bold', fillColor: [248, 250, 252] },
                },
                margin: { left: 15, right: 15 },
            });
            y = doc.lastAutoTable.finalY + 10;

            // === 3. 시뮬레이션 결과 테이블 ===
            doc.setFont('NanumGothic');
            doc.setFontSize(13);
            doc.setTextColor(30, 41, 59);
            doc.text('3. 시뮬레이션 결과', 15, y);
            y += 6;

            const resultHeaders = [[''].concat(machineIds.map(id => '재봉기 ' + id))];
            const resultBody = [
                ['생산수량'].concat(machineIds.map(id => results.machines[id].produced)),
                ['대기시간 (초)'].concat(machineIds.map(id => results.machines[id].totalWaitTime)),
                ['효율 (%)'].concat(machineIds.map(id => calcEfficiency(results.machines[id]))),
            ];

            doc.autoTable({
                startY: y,
                head: resultHeaders,
                body: resultBody,
                theme: 'grid',
                styles: { fontSize: 9, cellPadding: 3, halign: 'center', ...fontStyle },
                headStyles: {
                    fillColor: [22, 163, 74],
                    textColor: 255,
                    fontStyle: 'bold',
                    ...fontStyle,
                },
                columnStyles: {
                    0: { halign: 'left', fontStyle: 'bold', fillColor: [248, 250, 252] },
                },
                margin: { left: 15, right: 15 },
            });
            y = doc.lastAutoTable.finalY + 10;

            // === 4. 종합 요약 ===
            doc.setFont('NanumGothic');
            doc.setFontSize(13);
            doc.setTextColor(30, 41, 59);
            doc.text('4. 종합 요약', 15, y);
            y += 6;

            const simTimeHours = results.simTime / 3600;
            const pph = simTimeHours > 0
                ? Math.round((results.totalProduced / simTimeHours / basicSettings.workerCount) * 10) / 10
                : 0;

            const summaryHeaders = [['항목', '값']];
            const summaryBody = [
                ['총 생산수량', results.totalProduced + '개'],
                ['로봇 가동률', results.robotUtilization + '%'],
                ['로봇 총 가동시간', formatTimePdf(results.robotBusyTime)],
                ['시뮬레이션 시간', formatTimePdf(results.simTime)],
                // ['PPH', pph + '개/시간/인'],
                ['PPH', pph],
                ['재봉기 총 가동률', results.machineUtilization + '%'],
                ['재봉기 총 가동시간', formatTimePdf(results.totalMachineSewingTime)],
            ];
            // 각 재봉기 효율 추가
            for (const id of machineIds) {
                summaryBody.push([
                    '재봉기 ' + id + ' 효율',
                    calcEfficiency(results.machines[id]) + '%'
                ]);
            }

            doc.autoTable({
                startY: y,
                head: summaryHeaders,
                body: summaryBody,
                theme: 'grid',
                styles: { fontSize: 9, cellPadding: 3, ...fontStyle },
                headStyles: {
                    fillColor: [124, 58, 237],
                    textColor: 255,
                    fontStyle: 'bold',
                    ...fontStyle,
                },
                columnStyles: {
                    0: { fontStyle: 'bold', fillColor: [248, 250, 252], cellWidth: 50 },
                    1: { halign: 'center' },
                },
                margin: { left: 15, right: 15 },
                tableWidth: 120,
            });
            y = doc.lastAutoTable.finalY + 10;

            // === 4. 이벤트 로그 (전체) ===
            if (results.eventLog && results.eventLog.length > 0) {
                if (y > 240) {
                    doc.addPage();
                    y = 15;
                }

                doc.setFont('NanumGothic');
                doc.setFontSize(13);
                doc.setTextColor(30, 41, 59);
                const totalLogs = results.eventLog.length;
                doc.text('5. 이벤트 로그 (전체 ' + totalLogs + '건)', 15, y);
                y += 6;

                const logHeaders = [['시간(초)', '이벤트', '재봉기', '설명']];
                const logBody = results.eventLog.map(entry => [
                    entry.time.toFixed(1),
                    entry.type,
                    entry.machineId ? 'M' + entry.machineId : '-',
                    entry.description,
                ]);

                doc.autoTable({
                    startY: y,
                    head: logHeaders,
                    body: logBody,
                    theme: 'striped',
                    styles: { fontSize: 7, cellPadding: 2, ...fontStyle },
                    headStyles: {
                        fillColor: [100, 116, 139],
                        textColor: 255,
                        fontStyle: 'bold',
                        ...fontStyle,
                    },
                    columnStyles: {
                        0: { cellWidth: 20, halign: 'right' },
                        1: { cellWidth: 35 },
                        2: { cellWidth: 15, halign: 'center' },
                        3: { cellWidth: 'auto' },
                    },
                    margin: { left: 15, right: 15 },
                });
            }

            // === 푸터 (각 페이지) ===
            const pageCount = doc.internal.getNumberOfPages();
            const pageHeight = doc.internal.pageSize.getHeight();
            for (let i = 1; i <= pageCount; i++) {
                doc.setPage(i);
                doc.setFont('NanumGothic');
                doc.setFontSize(8);
                doc.setTextColor(148, 163, 184);
                doc.text(
                    'Co-Robot 재봉 시뮬레이션 (' + machineCount + '대 모드) - ' + i + ' / ' + pageCount + ' 페이지',
                    pageWidth / 2,
                    pageHeight - 12,
                    { align: 'center' }
                );
                doc.text(
                    '\u00A9 SUNTECH 2026',
                    pageWidth / 2,
                    pageHeight - 7,
                    { align: 'center' }
                );
            }

            // === 파일 저장 ===
            const filename = 'CoRobot_' + machineCount + 'M_Report_' +
                now.getFullYear() +
                String(now.getMonth() + 1).padStart(2, '0') +
                String(now.getDate()).padStart(2, '0') + '_' +
                String(now.getHours()).padStart(2, '0') +
                String(now.getMinutes()).padStart(2, '0') +
                '.pdf';
            doc.save(filename);

        } catch (err) {
            alert('PDF 생성 중 오류가 발생했습니다:\n' + err.message);
            console.error('PDF export error:', err);
        } finally {
            fontLoading = false;
            btn.textContent = originalText;
            btn.disabled = false;
        }
    }

    /**
     * 효율 계산 (생산량 / 이론최대 * 100)
     */
    function calcEfficiency(machineResult) {
        if (machineResult.theoreticalMax === 0) return '0.0';
        return (machineResult.produced / machineResult.theoreticalMax * 100).toFixed(1);
    }

    /**
     * 시간 포맷
     */
    function formatTimePdf(seconds) {
        const s = Math.round(seconds);
        if (s < 60) return s + '초';
        if (s < 3600) return Math.floor(s / 60) + '분 ' + (s % 60) + '초';
        const h = Math.floor(s / 3600);
        const m = Math.floor((s % 3600) / 60);
        return h + '시간 ' + m + '분 ' + (s % 60) + '초';
    }

    return { exportPdf };
})();
