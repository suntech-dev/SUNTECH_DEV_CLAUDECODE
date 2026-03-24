<?php
$page_title = 'Design Process Management';
$page_css_files = [
    '../../assets/css/fiori-page.css',
    'css/info_design_process_2.css',
];

require_once(__DIR__ . '/../../inc/head.php');
/* nav-fiori.php 제거 */
?>

<?php $nav_active = 'design_process'; require_once(__DIR__ . '/../../inc/nav-drawer-manage.php'); ?>

<!-- Signage Header -->
<div class="signage-header">
    <button id="navDrawerBtn" class="nav-drawer-btn" aria-label="Menu">&#9776;</button>
    <span class="signage-header__title">Design Process Management</span>

    <div class="signage-header__filters">
        <div class="search-container">
            <input type="text" id="realTimeSearch" class="fiori-input search-input" placeholder="Search process...">
            <button class="search-clear" id="searchClear" title="Clear search">&#10005;</button>
        </div>
        <select id="factoryFilterSelect" class="fiori-select">
            <!-- JS로 채워짐 -->
        </select>
        <select id="factoryLineFilterSelect" class="fiori-select">
            <!-- JS로 채워짐 -->
        </select>
        <select id="statusFilterSelect" class="fiori-select">
            <option value="">All Status</option>
            <option value="Y" selected>Used Only</option>
            <option value="N">Unused Only</option>
        </select>
        <div class="fiori-dropdown">
            <button id="columnToggleBtn" class="fiori-btn fiori-btn--secondary">Columns</button>
            <div id="columnToggleDropdown" class="fiori-dropdown__content"></div>
        </div>
        <button id="addBtn" class="fiori-btn fiori-btn--primary">Add Process</button>
        <button id="toggleSetProcessBtn" class="fiori-btn fiori-btn--secondary">Set Process</button>
        <button id="refreshBtn" class="fiori-btn fiori-btn--tertiary">Refresh</button>
    </div>
</div>

<!-- Main -->
<div class="manage-main">

    <!-- Process-Machine 매핑 패널 (토글) -->
    <div class="process-machine-panel" id="processMachinePanel">
        <div class="fiori-card">
            <div class="fiori-card__header">
                <div>
                    <h3 class="fiori-card__title">Process-Machine Assignment</h3>
                    <p class="fiori-card__subtitle">Assign Machines to Processes by dragging and dropping.</p>
                </div>
                <div style="display:flex; gap:6px;">
                    <button id="saveProcessMachineBtn" class="fiori-btn fiori-btn--primary">Save Changes</button>
                    <button id="cancelProcessMachineBtn" class="fiori-btn fiori-btn--tertiary">Close</button>
                </div>
            </div>

            <!-- Line 필터 버튼 바 -->
            <div class="line-filter-bar" id="lineFilterBar">
                <span class="line-filter-label">Filter by Line:</span>
                <!-- JS로 동적 생성 -->
            </div>

            <div class="fiori-card__content" style="padding:0; overflow:hidden;">
                <div id="processMachineContainer" class="process-machine-container">
                    <!-- JS로 동적 생성 -->
                </div>
            </div>
        </div>
    </div>

    <!-- 데이터 테이블 카드 -->
    <div class="fiori-card">
        <div class="fiori-card__header">
            <div>
                <h3 class="fiori-card__title">Design Process List</h3>
            </div>
            <div class="quick-actions">
                <button class="quick-action-btn active" data-filter="all">All</button>
                <button class="quick-action-btn" data-filter="used">Used</button>
                <button class="quick-action-btn" data-filter="unused">Unused</button>
            </div>
        </div>

        <div class="fiori-card__content fiori-p-0">
            <div style="overflow-x: auto; height: 100%;">
                <table class="fiori-table">
                    <thead id="tableHeader" class="fiori-table__header"></thead>
                    <tbody id="tableBody"></tbody>
                </table>
            </div>
            <div id="noDataMessage" class="text-center" style="padding: var(--sap-spacing-xl); color: var(--sap-text-secondary); display: none;">
                <p>No matching design processes found.</p>
            </div>
        </div>
    </div>

    <div id="pagination-controls" class="fiori-pagination"></div>

</div><!-- /manage-main -->


<!-- Design Process Modal -->
<div id="resourceModal" class="fiori-modal">
    <div class="fiori-modal__content">
        <div class="fiori-card">
            <div class="fiori-card__header">
                <div>
                    <h3 class="fiori-card__title" id="modalTitle">Design Process Information</h3>
                    <p class="fiori-card__subtitle">Enter and edit design process information</p>
                </div>
                <button type="button" class="fiori-btn fiori-btn--tertiary fiori-btn--sm close"
                        style="position: absolute; top: var(--sap-spacing-md); right: var(--sap-spacing-md);">
                    &#10005;
                </button>
            </div>
            <div class="fiori-card__content">
                <form id="resourceForm" class="fiori-form" enctype="multipart/form-data">
                    <input type="hidden" id="resourceId" name="idx">

                    <div style="display: flex; justify-content: space-between; align-items: center; margin-bottom: var(--sap-spacing-lg); padding-bottom: var(--sap-spacing-md); border-bottom: 1px solid var(--sap-border-subtle);">
                        <div style="display: flex; gap: var(--sap-spacing-sm); align-items: center;">
                            <div class="form-step active" data-step="1">1</div>
                            <div class="form-step-line"></div>
                            <div class="form-step" data-step="2">2</div>
                        </div>
                        <div style="font-size: var(--sap-font-size-sm); color: var(--sap-text-secondary);">
                            <span id="stepIndicator">Step 1 of 2</span>
                        </div>
                    </div>

                    <!-- Step 1 -->
                    <div class="form-section" data-section="1">
                        <h4 style="margin-bottom: var(--sap-spacing-md); color: var(--sap-text-primary);">Basic information</h4>

                        <div class="fiori-form__group">
                            <label for="factory_idx" class="fiori-form__label fiori-form__label--required">Factory</label>
                            <select id="factory_idx" name="factory_idx" class="fiori-select" required>
                                <option value="">Please select a factory</option>
                            </select>
                            <div class="fiori-form__help">Please select a factory</div>
                        </div>

                        <div class="fiori-form__group">
                            <label for="line_idx" class="fiori-form__label fiori-form__label--required">Line</label>
                            <select id="line_idx" name="line_idx" class="fiori-select" required disabled>
                                <option value="">Please select a line</option>
                            </select>
                            <div class="fiori-form__help">Select line after selecting factory</div>
                        </div>

                        <div class="fiori-form__group">
                            <label for="model_name" class="fiori-form__label">Model Name</label>
                            <input type="text" id="model_name" name="model_name" class="fiori-input"
                                   placeholder="Enter product model name">
                            <div class="fiori-form__help">Enter the product model name (e.g., AIR MAX MOTO 2K (W))</div>
                        </div>

                        <div class="fiori-form__group">
                            <label for="design_process" class="fiori-form__label fiori-form__label--required">Design Process Name</label>
                            <input type="text" id="design_process" name="design_process" class="fiori-input" required
                                   placeholder="Please enter the design process name">
                            <div class="fiori-form__help">Please enter a unique design process name (2-50 characters)</div>
                        </div>

                        <div class="fiori-form__group">
                            <label for="std_mc_needed" class="fiori-form__label">Standard MC Needed</label>
                            <input type="number" id="std_mc_needed" name="std_mc_needed" class="fiori-input" value="1" min="1"
                                   placeholder="Enter the number of machines needed">
                            <div class="fiori-form__help">Number of standard machines needed for this process (default: 1)</div>
                        </div>

                        <div class="fiori-form__group">
                            <label for="status" class="fiori-form__label">Status</label>
                            <select id="status" name="status" class="fiori-select">
                                <option value="Y">Used</option>
                                <option value="N">Unused</option>
                            </select>
                            <div class="fiori-form__help">Please select the current usage status.</div>
                        </div>
                    </div>

                    <!-- Step 2 -->
                    <div class="form-section" data-section="2" style="display: none;">
                        <h4 style="margin-bottom: var(--sap-spacing-md); color: var(--sap-text-primary);">Additional information</h4>

                        <div class="fiori-form__group">
                            <label for="file_upload" class="fiori-form__label">SOP File Upload</label>

                            <div id="existingFileInfo" style="display: none; margin-bottom: var(--sap-spacing-sm); padding: var(--sap-spacing-sm); background: var(--sap-surface-2); border-radius: var(--sap-radius-sm); border-left: 3px solid var(--sap-status-info);">
                                <div style="display: flex; align-items: center; justify-content: space-between;">
                                    <div>
                                        <span style="font-size: var(--sap-font-size-sm); color: var(--sap-text-secondary);">Current file:</span><br>
                                        <span id="existingFileName" style="font-weight: 500; color: var(--sap-text-primary);"></span>
                                    </div>
                                    <button type="button" id="viewExistingFileBtn" class="fiori-btn fiori-btn--tertiary fiori-btn--sm" style="min-width: auto;">
                                        View
                                    </button>
                                </div>
                            </div>

                            <input type="file" id="file_upload" name="file_upload" class="fiori-input"
                                   accept=".jpg,.jpeg,.png">
                            <div class="fiori-form__help">Upload SOP file (JPG, JPEG, PNG only, max 10MB). Leave empty to keep existing file.</div>
                        </div>

                        <div class="fiori-form__group">
                            <label for="remark" class="fiori-form__label">Remark</label>
                            <textarea id="remark" name="remark" class="fiori-input" rows="3"></textarea>
                            <div class="fiori-form__help">You can record design process information.</div>
                        </div>

                        <div style="padding: var(--sap-spacing-md); background: var(--sap-surface-2); border-radius: var(--sap-radius-md); margin-top: var(--sap-spacing-md);">
                            <h5 style="margin: 0 0 var(--sap-spacing-sm) 0; color: var(--sap-text-primary);">Confirm input information</h5>
                            <div style="font-size: var(--sap-font-size-sm); color: var(--sap-text-secondary);">
                                <div><strong>Process name:</strong> <span id="previewName">-</span></div>
                                <div><strong>Standard MC needed:</strong> <span id="previewStdMc">-</span></div>
                                <div><strong>Status:</strong> <span id="previewStatus">-</span></div>
                                <div><strong>File:</strong> <span id="previewFile">-</span></div>
                            </div>
                        </div>
                    </div>

                    <div class="fiori-form__actions">
                        <button type="button" class="fiori-btn fiori-btn--tertiary" id="modalCloseBtn">Cancel</button>
                        <button type="button" class="fiori-btn fiori-btn--secondary" id="prevStep" style="display: none;">Previous</button>
                        <button type="button" class="fiori-btn fiori-btn--primary" id="nextStep">Next</button>
                        <button type="submit" class="fiori-btn fiori-btn--primary" id="submitBtn" style="display: none;">Save Process</button>
                    </div>
                </form>
            </div>
        </div>
    </div>
</div>


<!-- SOP 이미지 미리보기 모달 -->
<div id="imagePreviewModal" class="fiori-modal" style="z-index: 9999;">
    <div class="fiori-modal__content" style="max-width: 90vw; max-height: 90vh; width: auto; height: auto;">
        <div class="fiori-card" style="height: 100%; display: flex; flex-direction: column;">
            <div class="fiori-card__header">
                <div>
                    <h3 class="fiori-card__title">SOP File Preview</h3>
                    <p class="fiori-card__subtitle" id="imageFileName">Image preview</p>
                </div>
                <button type="button" class="fiori-btn fiori-btn--tertiary fiori-btn--sm" id="imageModalClose"
                        style="position: absolute; top: var(--sap-spacing-md); right: var(--sap-spacing-md);">
                    &#10005;
                </button>
            </div>
            <div class="fiori-card__content" style="flex: 1; display: flex; align-items: center; justify-content: center; padding: var(--sap-spacing-lg);">
                <img id="previewImage" src="" alt="SOP Preview"
                     style="max-width: 100%; max-height: 100%; object-fit: contain; border-radius: var(--sap-radius-md); box-shadow: 0 4px 12px rgba(0,0,0,0.15);">
            </div>
            <div class="fiori-card__content" style="padding-top: 0; text-align: center;">
                <button type="button" class="fiori-btn fiori-btn--primary" id="imageModalCloseBtn">Close</button>
            </div>
        </div>
    </div>
</div>


<script type="module">
    import { createResourceManager } from '../../assets/js/resource-manager.js';
    import { initAdvancedFeatures, designProcessConfig } from './js/info_design_process_2.js';

    document.addEventListener('DOMContentLoaded', function() {
        const resourceManager = createResourceManager(designProcessConfig);

        setTimeout(() => {
            initAdvancedFeatures(resourceManager);
            initSopFileUpload();
            initProcessMachineMapping(resourceManager);
        }, 100);
    });

    // SOP 파일 업로드 기능
    function initSopFileUpload() {
        const fileInput          = document.getElementById('file_upload');
        const imageModalClose    = document.getElementById('imageModalClose');
        const imageModalCloseBtn = document.getElementById('imageModalCloseBtn');
        const imagePreviewModal  = document.getElementById('imagePreviewModal');
        const existingFileInfo   = document.getElementById('existingFileInfo');
        const existingFileName   = document.getElementById('existingFileName');
        const viewExistingFileBtn = document.getElementById('viewExistingFileBtn');
        const previewImage       = document.getElementById('previewImage');
        const imageFileName      = document.getElementById('imageFileName');

        let currentExistingFile = null;

        fileInput.addEventListener('change', function(e) {
            const file = e.target.files[0];
            if (!file) return;
            const allowed = ['image/jpeg', 'image/jpg', 'image/png'];
            if (!allowed.includes(file.type)) {
                alert('Only JPG, JPEG, PNG files are allowed.');
                fileInput.value = '';
                return;
            }
            if (file.size > 10 * 1024 * 1024) {
                alert('File size exceeds 10MB.');
                fileInput.value = '';
            }
        });

        if (viewExistingFileBtn) {
            viewExistingFileBtn.addEventListener('click', function() {
                if (currentExistingFile) {
                    previewImage.src = `../../upload/sop/${currentExistingFile}`;
                    imageFileName.textContent = currentExistingFile;
                    imagePreviewModal.style.display = 'flex';
                    setTimeout(() => imagePreviewModal.classList.add('show'), 10);
                }
            });
        }

        function closeImagePreviewModal() {
            imagePreviewModal.classList.remove('show');
            setTimeout(() => { imagePreviewModal.style.display = 'none'; }, 300);
        }

        [imageModalClose, imageModalCloseBtn].forEach(btn => btn.addEventListener('click', closeImagePreviewModal));
        imagePreviewModal.addEventListener('click', function(e) {
            if (e.target === imagePreviewModal) closeImagePreviewModal();
        });

        window.updateExistingFileInfo = function(filename) {
            currentExistingFile = filename;
            if (filename && filename.trim() !== '') {
                existingFileName.textContent = filename;
                existingFileInfo.style.display = 'block';
            } else {
                existingFileInfo.style.display = 'none';
            }
        };

        window.resetExistingFileInfo = function() {
            currentExistingFile = null;
            existingFileInfo.style.display = 'none';
            fileInput.value = '';
        };
    }

    // Process-Machine 매핑 기능 초기화
    function initProcessMachineMapping(resourceManager) {
        const toggleBtn    = document.getElementById('toggleSetProcessBtn');
        const panel        = document.getElementById('processMachinePanel');
        const managMain    = panel.closest('.manage-main');
        const container    = document.getElementById('processMachineContainer');
        const lineFilterBar = document.getElementById('lineFilterBar');
        const saveBtn      = document.getElementById('saveProcessMachineBtn');
        const cancelBtn    = document.getElementById('cancelProcessMachineBtn');

        let mappingData        = null;
        let currentAssignments = {};
        let selectedLine       = null;

        let autoScrollInterval = null;
        const SCROLL_ZONE_SIZE = 80;
        const SCROLL_SPEED     = 10;

        toggleBtn.addEventListener('click', async function() {
            if (!panel.classList.contains('is-open')) {
                panel.classList.add('is-open');
                managMain.classList.add('panel-open');
                toggleBtn.textContent = 'Hide Process';
                await loadMappingData();
            } else {
                panel.classList.remove('is-open');
                managMain.classList.remove('panel-open');
                toggleBtn.textContent = 'Set Process';
            }
        });

        cancelBtn.addEventListener('click', function() {
            panel.classList.remove('is-open');
            managMain.classList.remove('panel-open');
            toggleBtn.textContent = 'Set Process';
        });

        saveBtn.addEventListener('click', async function() {
            await saveMachineAssignments();
        });

        async function loadMappingData() {
            try {
                const response = await fetch('proc/process_machine_mapping.php?action=get_mapping');
                const result   = await response.json();

                if (result.success) {
                    mappingData = result.data;
                    currentAssignments = {};
                    mappingData.machines.forEach(machine => {
                        currentAssignments[machine.idx] = machine.design_process_idx || 0;
                    });
                    renderLineFilterBar();
                    renderMappingUI();
                } else {
                    alert('Failed to load data: ' + result.message);
                }
            } catch (error) {
                console.error('Error loading mapping data:', error);
                alert('An error occurred while loading data.');
            }
        }

        function renderLineFilterBar() {
            if (!mappingData) return;

            const lineMap = new Map();
            mappingData.machines.forEach(machine => {
                const lineKey     = `${machine.factory_name || 'N/A'}-${machine.line_name || 'N/A'}`;
                const lineDisplay = `${machine.factory_name || 'N/A'} / ${machine.line_name || 'N/A'}`;
                if (!lineMap.has(lineKey)) {
                    lineMap.set(lineKey, { key: lineKey, display: lineDisplay, count: 0 });
                }
                lineMap.get(lineKey).count++;
            });

            const lines = Array.from(lineMap.values()).sort((a, b) => a.display.localeCompare(b.display));

            const filterBarLabel = lineFilterBar.querySelector('.line-filter-label');
            while (filterBarLabel.nextSibling) lineFilterBar.removeChild(filterBarLabel.nextSibling);

            lines.forEach((lineInfo, index) => {
                const btn = document.createElement('button');
                btn.className        = 'line-filter-btn';
                btn.dataset.lineKey  = lineInfo.key;
                btn.innerHTML        = `${lineInfo.display} <span class="line-filter-count">${lineInfo.count}</span>`;
                btn.addEventListener('click', () => selectLine(lineInfo.key));
                lineFilterBar.appendChild(btn);

                if (index === 0 && selectedLine === null) {
                    selectedLine = lineInfo.key;
                    btn.classList.add('active');
                }
            });
        }

        function selectLine(lineKey) {
            selectedLine = lineKey;
            document.querySelectorAll('.line-filter-btn').forEach(btn => {
                btn.classList.toggle('active', btn.dataset.lineKey === lineKey);
            });
            renderMappingUI();
        }

        function getFilteredMachines() {
            if (!selectedLine) return [];
            return mappingData.machines.filter(m => `${m.factory_name || 'N/A'}-${m.line_name || 'N/A'}` === selectedLine);
        }

        function getFilteredProcesses() {
            if (!selectedLine) return [];
            return mappingData.processes.filter(p => `${p.factory_name || 'N/A'}-${p.line_name || 'N/A'}` === selectedLine);
        }

        function renderMappingUI() {
            if (!mappingData) return;
            container.innerHTML = '';
            getFilteredProcesses().forEach(process => {
                container.appendChild(createProcessRow(process, getFilteredMachines()));
            });
            container.appendChild(createEmptyRow(getFilteredMachines()));
        }

        function createProcessRow(process, filteredMachines) {
            const row = document.createElement('div');
            row.className         = 'process-row';
            row.dataset.processIdx = process.idx;

            const processBox = document.createElement('div');
            processBox.className         = 'process-box';
            processBox.dataset.processIdx = process.idx;
            processBox.dataset.stdMcNeeded = process.std_mc_needed;

            const currentMachineCount = filteredMachines.filter(m =>
                parseInt(currentAssignments[m.idx]) === parseInt(process.idx)
            ).length;
            const availableSlots = process.std_mc_needed - currentMachineCount;
            const isFull         = currentMachineCount >= process.std_mc_needed;

            const viewDiv = document.createElement('div');
            viewDiv.className = 'process-box-view';
            viewDiv.innerHTML = `
                <div class="process-box-header">
                    <div class="process-name">${escapeHtml(process.design_process)}</div>
                    <div class="process-capacity">Capacity: <span class="process-capacity-badge ${isFull ? 'full' : ''}">${process.std_mc_needed}</span></div>
                    <div class="machine-count">Assigned: <span class="machine-count-current">${currentMachineCount}</span> / Available: <span class="machine-count-available ${isFull ? 'full' : ''}">${availableSlots}</span></div>
                </div>`;

            const editDiv = document.createElement('div');
            editDiv.className = 'process-box-edit';
            editDiv.innerHTML = `
                <input type="text" class="fiori-input" value="${escapeHtml(process.design_process)}" placeholder="Process Name">
                <input type="number" class="fiori-input" value="${process.std_mc_needed}" min="1" placeholder="Capacity">
                <div class="edit-actions">
                    <button class="fiori-btn fiori-btn--primary fiori-btn--sm save-process">Save</button>
                    <button class="fiori-btn fiori-btn--tertiary fiori-btn--sm cancel-edit">Cancel</button>
                </div>`;

            processBox.appendChild(viewDiv);
            processBox.appendChild(editDiv);

            processBox.addEventListener('click', function(e) {
                if (!processBox.classList.contains('editing') && (e.target === processBox || e.target.closest('.process-box-view'))) {
                    processBox.classList.add('editing');
                }
            });

            editDiv.querySelector('.save-process').addEventListener('click', async function(e) {
                e.stopPropagation();
                await saveProcessInfo(processBox, process.idx);
            });

            editDiv.querySelector('.cancel-edit').addEventListener('click', function(e) {
                e.stopPropagation();
                processBox.classList.remove('editing');
                editDiv.querySelectorAll('input')[0].value = process.design_process;
                editDiv.querySelectorAll('input')[1].value = process.std_mc_needed;
            });

            row.appendChild(processBox);

            const machinesContainer = document.createElement('div');
            machinesContainer.className          = 'machines-container drop-zone';
            machinesContainer.dataset.processIdx  = process.idx;
            machinesContainer.dataset.stdMcNeeded = process.std_mc_needed;

            filteredMachines.filter(m =>
                parseInt(currentAssignments[m.idx]) === parseInt(process.idx)
            ).forEach(machine => machinesContainer.appendChild(createMachineBox(machine)));

            setupDropZone(machinesContainer);
            row.appendChild(machinesContainer);
            return row;
        }

        function createEmptyRow(filteredMachines) {
            const row = document.createElement('div');
            row.className = 'process-row';

            const emptyBox = document.createElement('div');
            emptyBox.className   = 'process-box empty';
            emptyBox.textContent = 'Unassigned';
            row.appendChild(emptyBox);

            const machinesContainer = document.createElement('div');
            machinesContainer.className         = 'machines-container drop-zone';
            machinesContainer.dataset.processIdx = '0';

            filteredMachines.filter(m =>
                !currentAssignments[m.idx] || parseInt(currentAssignments[m.idx]) === 0
            ).forEach(machine => machinesContainer.appendChild(createMachineBox(machine)));

            setupDropZone(machinesContainer);
            row.appendChild(machinesContainer);
            return row;
        }

        function createMachineBox(machine) {
            const box = document.createElement('div');
            box.className          = 'machine-box';
            box.draggable          = true;
            box.dataset.machineIdx = machine.idx;
            box.dataset.machineNo  = machine.machine_no;
            box.innerHTML = `
                <div class="machine-box-info">
                    <div class="machine-box-location">
                        <div class="machine-box-factory">${escapeHtml(machine.factory_name || 'N/A')}</div>
                        <div class="machine-box-line">${escapeHtml(machine.line_name || 'N/A')}</div>
                    </div>
                    <div class="machine-box-number">${escapeHtml(machine.machine_no)}</div>
                </div>`;
            box.addEventListener('dragstart', handleDragStart);
            box.addEventListener('dragend', handleDragEnd);
            return box;
        }

        function startAutoScroll(direction) {
            if (autoScrollInterval) return;
            autoScrollInterval = setInterval(() => {
                container.scrollTop += direction === 'up' ? -SCROLL_SPEED : SCROLL_SPEED;
            }, 16);
        }

        function stopAutoScroll() {
            if (autoScrollInterval) { clearInterval(autoScrollInterval); autoScrollInterval = null; }
        }

        function handleGlobalDragOver(e) {
            if (!container) return;
            const rect        = container.getBoundingClientRect();
            const visibleTop  = Math.max(rect.top, 0);
            const visibleBot  = Math.min(rect.bottom, window.innerHeight);
            const mouseY      = e.clientY;
            if (mouseY >= visibleTop && mouseY <= visibleBot) {
                if (mouseY - visibleTop < SCROLL_ZONE_SIZE)      startAutoScroll('up');
                else if (visibleBot - mouseY < SCROLL_ZONE_SIZE) startAutoScroll('down');
                else stopAutoScroll();
            } else {
                stopAutoScroll();
            }
        }

        function setupDropZone(dropZone) {
            dropZone.addEventListener('dragover',  handleDragOver);
            dropZone.addEventListener('dragleave', handleDragLeave);
            dropZone.addEventListener('drop',      handleDrop);
        }

        function handleDragStart(e) {
            e.currentTarget.classList.add('dragging');
            e.dataTransfer.effectAllowed = 'move';
            e.dataTransfer.setData('machine_idx', e.currentTarget.dataset.machineIdx);
            document.addEventListener('dragover', handleGlobalDragOver);
        }

        function handleDragEnd(e) {
            e.currentTarget.classList.remove('dragging');
            stopAutoScroll();
            document.removeEventListener('dragover', handleGlobalDragOver);
        }

        function handleDragOver(e) {
            e.preventDefault();
            e.dataTransfer.dropEffect = 'move';
            e.currentTarget.classList.add('drag-over');
            return false;
        }

        function handleDragLeave(e) {
            e.currentTarget.classList.remove('drag-over');
        }

        function handleDrop(e) {
            e.stopPropagation();
            e.preventDefault();
            stopAutoScroll();
            document.removeEventListener('dragover', handleGlobalDragOver);

            const dropZone       = e.currentTarget;
            dropZone.classList.remove('drag-over');

            const machineIdx        = e.dataTransfer.getData('machine_idx');
            const targetProcessIdx  = parseInt(dropZone.dataset.processIdx || 0);
            const stdMcNeeded       = parseInt(dropZone.dataset.stdMcNeeded || 999);
            const draggedElement    = document.querySelector(`.machine-box[data-machine-idx="${machineIdx}"]`);

            if (!draggedElement) return;

            if (targetProcessIdx !== 0) {
                const filteredMachines = getFilteredMachines();
                const currentCountInLine = filteredMachines.filter(m =>
                    parseInt(currentAssignments[m.idx]) === targetProcessIdx
                ).length;
                const isAlreadyAssigned = parseInt(currentAssignments[machineIdx]) === targetProcessIdx;
                if (!isAlreadyAssigned && currentCountInLine >= stdMcNeeded) {
                    alert(`The maximum number of Machine allocations for this Process in the current line is ${stdMcNeeded}.`);
                    return;
                }
            }

            dropZone.appendChild(draggedElement);
            currentAssignments[machineIdx] = targetProcessIdx;
            updateAllMachineCounts();
            return false;
        }

        function updateAllMachineCounts() {
            document.querySelectorAll('.process-row').forEach(row => {
                const processIdx = parseInt(row.dataset.processIdx);
                const processBox = row.querySelector('.process-box:not(.empty)');
                if (!processBox) return;

                const stdMcNeeded         = parseInt(processBox.dataset.stdMcNeeded);
                const dropZone            = row.querySelector('.drop-zone');
                const currentMachineCount = dropZone.querySelectorAll('.machine-box').length;
                const availableSlots      = stdMcNeeded - currentMachineCount;
                const isFull              = currentMachineCount >= stdMcNeeded;

                const currentCountSpan  = processBox.querySelector('.machine-count-current');
                const availableCountSpan = processBox.querySelector('.machine-count-available');
                const capacityBadge     = processBox.querySelector('.process-capacity-badge');

                if (currentCountSpan)   currentCountSpan.textContent  = currentMachineCount;
                if (availableCountSpan) { availableCountSpan.textContent = availableSlots; availableCountSpan.classList.toggle('full', isFull); }
                if (capacityBadge)      capacityBadge.classList.toggle('full', isFull);
            });
        }

        async function saveProcessInfo(processBox, processIdx) {
            const inputs      = processBox.querySelectorAll('input');
            const newName     = inputs[0].value.trim();
            const newCapacity = parseInt(inputs[1].value);

            if (!newName)       { alert('Please enter a process name.'); return; }
            if (newCapacity < 1) { alert('Capacity must be at least 1.'); return; }

            try {
                const formData = new FormData();
                formData.append('action',       'update_process');
                formData.append('idx',          processIdx);
                formData.append('design_process', newName);
                formData.append('std_mc_needed', newCapacity);

                const response = await fetch('proc/process_machine_mapping.php', { method: 'POST', body: formData });
                const result   = await response.json();

                if (result.success) {
                    processBox.classList.remove('editing');
                    processBox.querySelector('.process-name').textContent         = newName;
                    processBox.querySelector('.process-capacity-badge').textContent = newCapacity;
                    processBox.dataset.stdMcNeeded = newCapacity;
                    const dropZone = processBox.parentElement.querySelector('.drop-zone');
                    if (dropZone) dropZone.dataset.stdMcNeeded = newCapacity;
                    const process = mappingData.processes.find(p => p.idx === processIdx);
                    if (process) { process.design_process = newName; process.std_mc_needed = newCapacity; }
                    updateAllMachineCounts();
                } else {
                    alert('Save failed: ' + result.message);
                }
            } catch (error) {
                alert('An error occurred while saving.');
            }
        }

        async function saveMachineAssignments() {
            try {
                const assignments = Object.keys(currentAssignments).map(machineIdx => ({
                    machine_idx:        parseInt(machineIdx),
                    design_process_idx: parseInt(currentAssignments[machineIdx])
                }));

                const formData = new FormData();
                formData.append('action',      'update_machine_assignments');
                formData.append('assignments', JSON.stringify(assignments));

                const response = await fetch('proc/process_machine_mapping.php', { method: 'POST', body: formData });
                const result   = await response.json();

                if (result.success) {
                    alert(`Machine assignments saved. (${result.updated_count} items)`);
                    if (resourceManager && resourceManager.loadData) await resourceManager.loadData();
                } else {
                    alert('Save failed: ' + result.message);
                }
            } catch (error) {
                alert('An error occurred while saving.');
            }
        }

        function escapeHtml(text) {
            const div = document.createElement('div');
            div.textContent = text;
            return div.innerHTML;
        }
    }
</script>


</body>
</html>
