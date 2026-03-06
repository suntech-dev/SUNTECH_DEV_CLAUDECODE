/**
 * pdf.js - PDF export module
 * Uses jsPDF + autoTable to save simulation results as PDF
 * Dynamic support for 2/3 machine modes
 */
const PdfExport = (() => {
    let fontLoading = false;

    /**
     * Get machine position label
     */
    function getMachineLabel(id) {
        const count = Config.getMachineCount();
        if (count === 2) {
            if (id === 1) return 'Machine 1 (Left)';
            if (id === 2) return 'Machine 2 (Right)';
        } else {
            if (id === 1) return 'Machine 1 (Left)';
            if (id === 2) return 'Machine 2 (Top)';
            if (id === 3) return 'Machine 3 (Right)';
        }
        return 'Machine ' + id;
    }

    /**
     * Generate and download simulation results as PDF
     */
    async function exportPdf(results, machineConfigs) {
        if (typeof window.jspdf === 'undefined') {
            alert('PDF library is loading. Please try again shortly.');
            return;
        }

        if (fontLoading) return;
        fontLoading = true;

        const btn = document.getElementById('btn-pdf');
        const originalText = btn.textContent;
        btn.textContent = 'Generating...';
        btn.disabled = true;

        try {
            const machineCount = Config.getMachineCount();
            const machineIds = Config.MACHINE_IDS;

            const { jsPDF } = window.jspdf;
            const doc = new jsPDF('p', 'mm', 'a4');
            const pageWidth = doc.internal.pageSize.getWidth();
            let y = 15;

            // === Title ===
            doc.setFontSize(18);
            doc.setTextColor(37, 99, 235);
            doc.text('SunTech Co-Robot Sewing Simulation Report', pageWidth / 2, y, { align: 'center' });
            y += 8;

            doc.setFontSize(11);
            doc.setTextColor(100, 116, 139);
            doc.text('( ' + machineCount + '-Machine Mode )', pageWidth / 2, y, { align: 'center' });
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

            // === Divider ===
            doc.setDrawColor(226, 232, 240);
            doc.setLineWidth(0.5);
            doc.line(15, y, pageWidth - 15, y);
            y += 8;

            // === 1. Basic Settings ===
            const basicSettings = Config.readBasicSettings();
            doc.setFontSize(13);
            doc.setTextColor(30, 41, 59);
            doc.text('1. Basic Settings', 15, y);
            y += 6;

            const basicHeaders = [['Item', 'Value']];
            const basicBody = [
                ['Workers', basicSettings.workerCount + ''],
                ['Pallet Prep Time', basicSettings.palletPrepTime + 's'],
                ['Pallet Count', basicSettings.palletCount + ''],
            ];

            doc.autoTable({
                startY: y,
                head: basicHeaders,
                body: basicBody,
                theme: 'grid',
                styles: { fontSize: 9, cellPadding: 3 },
                headStyles: {
                    fillColor: [100, 116, 139],
                    textColor: 255,
                    fontStyle: 'bold',
                },
                columnStyles: {
                    0: { fontStyle: 'bold', fillColor: [248, 250, 252], cellWidth: 50 },
                    1: { halign: 'center' },
                },
                margin: { left: 15, right: 15 },
                tableWidth: 120,
            });
            y = doc.lastAutoTable.finalY + 10;

            // === 2. Input Settings ===
            doc.setFontSize(13);
            doc.setTextColor(30, 41, 59);
            doc.text('2. Input Settings', 15, y);
            y += 6;

            const inputHeaders = [[''].concat(machineIds.map(id => getMachineLabel(id)))];
            const inputBody = [
                ['Initial Load Time (s)'].concat(machineIds.map(id => machineConfigs[id].initLoadTime)),
                ['Unload & Load Time (s)'].concat(machineIds.map(id => machineConfigs[id].unloadLoadTime)),
                ['Return Time (s)'].concat(machineIds.map(id => machineConfigs[id].returnTime)),
                ['Sewing Time (s)'].concat(machineIds.map(id => machineConfigs[id].sewingTime)),
            ];

            doc.autoTable({
                startY: y,
                head: inputHeaders,
                body: inputBody,
                theme: 'grid',
                styles: { fontSize: 9, cellPadding: 3, halign: 'center' },
                headStyles: {
                    fillColor: [37, 99, 235],
                    textColor: 255,
                    fontStyle: 'bold',
                },
                columnStyles: {
                    0: { halign: 'left', fontStyle: 'bold', fillColor: [248, 250, 252] },
                },
                margin: { left: 15, right: 15 },
            });
            y = doc.lastAutoTable.finalY + 10;

            // === 3. Simulation Results ===
            doc.setFontSize(13);
            doc.setTextColor(30, 41, 59);
            doc.text('3. Simulation Results', 15, y);
            y += 6;

            const resultHeaders = [[''].concat(machineIds.map(id => 'Machine ' + id))];
            const resultBody = [
                ['Production'].concat(machineIds.map(id => results.machines[id].produced)),
                ['Wait Time (s)'].concat(machineIds.map(id => results.machines[id].totalWaitTime)),
                ['Efficiency (%)'].concat(machineIds.map(id => calcEfficiency(results.machines[id]))),
            ];

            doc.autoTable({
                startY: y,
                head: resultHeaders,
                body: resultBody,
                theme: 'grid',
                styles: { fontSize: 9, cellPadding: 3, halign: 'center' },
                headStyles: {
                    fillColor: [22, 163, 74],
                    textColor: 255,
                    fontStyle: 'bold',
                },
                columnStyles: {
                    0: { halign: 'left', fontStyle: 'bold', fillColor: [248, 250, 252] },
                },
                margin: { left: 15, right: 15 },
            });
            y = doc.lastAutoTable.finalY + 10;

            // === 4. Summary ===
            doc.setFontSize(13);
            doc.setTextColor(30, 41, 59);
            doc.text('4. Summary', 15, y);
            y += 6;

            const simTimeHours = results.simTime / 3600;
            const pph = simTimeHours > 0
                ? Math.round((results.totalProduced / simTimeHours / basicSettings.workerCount) * 10) / 10
                : 0;

            const summaryHeaders = [['Item', 'Value']];
            const summaryBody = [
                ['Total Production', results.totalProduced + ''],
                ['Robot Utilization', results.robotUtilization + '%'],
                ['Robot Active Time', formatTimePdf(results.robotBusyTime)],
                ['Simulation Time', formatTimePdf(results.simTime)],
                ['PPH', pph + ''],
                ['Machine Utilization', results.machineUtilization + '%'],
                ['Machine Active Time', formatTimePdf(results.totalMachineSewingTime)],
            ];
            for (const id of machineIds) {
                summaryBody.push([
                    'Machine ' + id + ' Efficiency',
                    calcEfficiency(results.machines[id]) + '%'
                ]);
            }

            doc.autoTable({
                startY: y,
                head: summaryHeaders,
                body: summaryBody,
                theme: 'grid',
                styles: { fontSize: 9, cellPadding: 3 },
                headStyles: {
                    fillColor: [124, 58, 237],
                    textColor: 255,
                    fontStyle: 'bold',
                },
                columnStyles: {
                    0: { fontStyle: 'bold', fillColor: [248, 250, 252], cellWidth: 50 },
                    1: { halign: 'center' },
                },
                margin: { left: 15, right: 15 },
                tableWidth: 120,
            });
            y = doc.lastAutoTable.finalY + 10;

            // === 5. Event Log (Full) ===
            if (results.eventLog && results.eventLog.length > 0) {
                if (y > 240) {
                    doc.addPage();
                    y = 15;
                }

                doc.setFontSize(13);
                doc.setTextColor(30, 41, 59);
                const totalLogs = results.eventLog.length;
                doc.text('5. Event Log (Total ' + totalLogs + ' entries)', 15, y);
                y += 6;

                const logHeaders = [['Time(s)', 'Event', 'Machine', 'Description']];
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
                    styles: { fontSize: 7, cellPadding: 2 },
                    headStyles: {
                        fillColor: [100, 116, 139],
                        textColor: 255,
                        fontStyle: 'bold',
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

            // === Footer (each page) ===
            const pageCount = doc.internal.getNumberOfPages();
            const pageHeight = doc.internal.pageSize.getHeight();
            for (let i = 1; i <= pageCount; i++) {
                doc.setPage(i);
                doc.setFontSize(8);
                doc.setTextColor(148, 163, 184);
                doc.text(
                    'Co-Robot Sewing Simulation (' + machineCount + '-Machine Mode) - Page ' + i + ' / ' + pageCount,
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

            // === Save File ===
            const filename = 'CoRobot_' + machineCount + 'M_Report_' +
                now.getFullYear() +
                String(now.getMonth() + 1).padStart(2, '0') +
                String(now.getDate()).padStart(2, '0') + '_' +
                String(now.getHours()).padStart(2, '0') +
                String(now.getMinutes()).padStart(2, '0') +
                '.pdf';
            doc.save(filename);

        } catch (err) {
            alert('Error generating PDF:\n' + err.message);
            console.error('PDF export error:', err);
        } finally {
            fontLoading = false;
            btn.textContent = originalText;
            btn.disabled = false;
        }
    }

    /**
     * Calculate efficiency (produced / theoretical max * 100)
     */
    function calcEfficiency(machineResult) {
        if (machineResult.theoreticalMax === 0) return '0.0';
        return (machineResult.produced / machineResult.theoreticalMax * 100).toFixed(1);
    }

    /**
     * Format time
     */
    function formatTimePdf(seconds) {
        const s = Math.round(seconds);
        if (s < 60) return s + 's';
        if (s < 3600) return Math.floor(s / 60) + 'min ' + (s % 60) + 's';
        const h = Math.floor(s / 3600);
        const m = Math.floor((s % 3600) / 60);
        return h + 'hr ' + m + 'min ' + (s % 60) + 's';
    }

    return { exportPdf };
})();
